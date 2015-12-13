#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include "diameter_queue_input.h"
#include <pthread.h>
#include "utility.h"
#include <string.h>
#include "server_threadhold.h"

#define MAX_EVENTS 64
#define TIME_QUEUE_WAIT 3000

struct d_queue queue;
pthread_t queue_in_thread;
pthread_t listen_thread;
pthread_t monitor_thread;

pthread_mutex_t queue_mutex;
//use for suspend
pthread_mutex_t suspend_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  resume_cond  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t status_read_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t monitor_mutex = PTHREAD_MUTEX_INITIALIZER;
int queue_suspended;
int is_on_reading = 1;

void* listening_client(void * arg);
void* checking_queue_in(void *arg);
void* monitoring_income(void *arg);
void suspendQueueThread();
void resumeQueueThread();
long time_start_free;

static int make_socket_non_blocking(int sfd){
    int flags, s;
    flags = fcntl(sfd, F_GETFL, 0);
    if(flags == -1){
        perror("fcntl");
        return -1;
    }

    s = fcntl(sfd, F_SETFL, flags | O_NONBLOCK);
    if(s == -1){
        perror("fcntl");
        return -1;
    }
    return 0;
}

int main()
{
    int res;
    void* thread_queue_result;
    void* thread_listen_result;
    void* thread_monitor_result;

    time_start_free = -1;
    init_queue(&queue);
    res = pthread_mutex_init(&queue_mutex, NULL);
    if(res != 0){
        perror("Mutex initialization failed");
        exit(EXIT_FAILURE);
    }
    res = pthread_create(&listen_thread, NULL, listening_client, NULL);
	if(res != 0){
		perror("Thread creation failed");
		exit(EXIT_FAILURE);
	}

    res = pthread_create(&queue_in_thread, NULL, checking_queue_in, NULL);
	if(res != 0){
		perror("Thread creation failed");
		exit(EXIT_FAILURE);
	}
	res = pthread_create(&monitor_thread, NULL, monitoring_income, NULL);
	if(res != 0){
		perror("Thread creation failed");
		exit(EXIT_FAILURE);
	}

	res = pthread_join(listen_thread, &thread_listen_result);
	if(res != 0){
        perror("Thread join failed");
        exit(EXIT_FAILURE);
	}
	res = pthread_join(queue_in_thread, &thread_queue_result);
	if(res != 0){
        perror("Thread join failed");
        exit(EXIT_FAILURE);
	}
	res = pthread_join(monitor_thread, &thread_monitor_result);
	if(res != 0){
        perror("Thread join failed");
        exit(EXIT_FAILURE);
	}
	pthread_mutex_destroy(&queue_mutex);
	return 0;
}

void* listening_client(void* arg){
    int sfd, efd, s;
    struct epoll_event event;
    struct epoll_event *events;
    int server_len;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_address;
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(8500);
    server_len = sizeof(server_addr);
    int over_msg = 0;
    bind(sfd, (struct sockaddr*)&server_addr, server_len);
    if(sfd == -1){
        perror("bind");
        pthread_exit(NULL);
    }
    s = make_socket_non_blocking(sfd);
    if(s == -1){
        perror("make_socket_non_blocking");
        pthread_exit(NULL);
    }
    s = listen(sfd, SOMAXCONN);
    if(s == -1){
        perror("listen");
        pthread_exit(NULL);
    }
    printf("server is listening on port: %d\n", ntohs(server_addr.sin_port));
    efd = epoll_create1(0);
    if(efd == -1){
        perror("epoll create");
        pthread_exit(NULL);
    }
    event.data.fd = sfd;
    event.events = EPOLLIN | EPOLLET;
    s = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event);
    if(s == -1){
        perror("epoll_ctl");
        pthread_exit(NULL);
    }
    events = calloc(MAX_EVENTS, sizeof event);
    while(1){
        int n, i;
        n = epoll_wait(efd, events, MAX_EVENTS, -1);
        for(i = 0; i < n; i++){
            if((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN))){
                fprintf(stderr, "epoll error\n");
                close(events[i].data.fd);
                continue;
            }
            else if(sfd == events[i].data.fd){
                while(1){
                    int infd;
                    int in_len;
                    in_len = sizeof(client_address);
                    infd = accept(sfd, (struct sockaddr*) &client_address, &in_len);
                    if(infd == -1){
                        if(errno == EAGAIN || errno == EWOULDBLOCK){
                            break;
                        }else{
                            perror("accept");
                            break;
                        }
                    }
                    s = make_socket_non_blocking(infd);
                    if(s == -1){
                        pthread_exit(NULL);
                    }
                    printf("have connect from: %d\n", ntohs(client_address.sin_port));
                    event.data.fd = infd;
                    event.events = EPOLLIN | EPOLLET;
                    s = epoll_ctl(efd, EPOLL_CTL_ADD, infd, &event);
                    if(s == -1){
                        perror("epoll_ctl");
                        pthread_exit(NULL);
                    }
                }
                continue;
            }
            else{
                int done = 0;
                while(1){
                    int count;
                    char buf[512];
                    memset(buf, 0, sizeof buf);
                    count = read(events[i].data.fd, buf, sizeof(buf));
                    if(count == -1){
                        if(errno != EAGAIN){
                            perror("read");
                            done = 1;
                        }
                        break;
                    }else if(count == 0){
                        done = 1;
                        break;
                    }
                    //printf("Msg from: %d\n", events[i].data.fd);
                    //s = write(1, buf, count);
                    pthread_mutex_lock(&monitor_mutex);
                    over_msg = incoming_msg();
                    pthread_mutex_unlock(&monitor_mutex);
                    if(over_msg){
                        pthread_mutex_lock(&queue_mutex);
                        check_and_push(events[i].data.fd, buf, count, &queue);
                        pthread_mutex_unlock(&queue_mutex);
                        pthread_mutex_lock(&status_read_mutex);
                        if(!is_on_reading){
                            is_on_reading = 1;
                            pthread_mutex_unlock(&status_read_mutex);
                            resumeQueueThread();
                        }else{
                            pthread_mutex_unlock(&status_read_mutex);
                        }
                        if(s == -1){
                            perror("write");
                            pthread_exit(NULL);
                        }
                    }
                }
                if(done){
                    printf("Closed connection on descriptor %d\n", events[i].data.fd);
                    close(events[i].data.fd);
                }
            }
        }
    }
    free(events);
    close(sfd);
    pthread_exit(NULL);
}

void* checking_queue_in(void *arg){
    while(1){
        //search in queue if have any data
        pthread_mutex_lock(&queue_mutex);
        if(is_dempty(&queue)){
            pthread_mutex_unlock(&queue_mutex);
            if(time_start_free < 0){
                time_start_free = get_curr_time();
            }

            if(get_curr_time() - time_start_free > TIME_QUEUE_WAIT){
                pthread_mutex_lock(&status_read_mutex);
                is_on_reading = 0;
                pthread_mutex_unlock(&status_read_mutex);
                time_start_free = -1;
                suspendQueueThread();
            }

        }else{
            pthread_mutex_unlock(&queue_mutex);
            pthread_mutex_lock(&queue_mutex);
            struct d_queue_data data = pop_from_queue(&queue);
            pthread_mutex_unlock(&queue_mutex);
            write(data.efd, data.buf, strlen(data.buf));
            //printf("value: %d\n", data.a);
        }
    }
}

void suspendQueueThread() {//must call in queue thread
    pthread_mutex_lock(&suspend_mutex);
    queue_suspended = 1;
    do {
        pthread_cond_wait(&resume_cond, &suspend_mutex);
    } while (queue_suspended);
    pthread_mutex_unlock(&suspend_mutex);
}

void resumeQueueThread(){
    pthread_mutex_lock(&suspend_mutex);
    queue_suspended = 0;
    pthread_cond_signal(&resume_cond);
    pthread_mutex_unlock(&suspend_mutex);
}

void* monitoring_income(void *arg){
    while(1){
        if(get_curr_time() - time_start_count > 1000){
            pthread_mutex_lock(&monitor_mutex);
            acceptTPS = 0;
            TPS = 0;
            pthread_mutex_unlock(&monitor_mutex);
        }
        printf("Current TPS: %d\n", TPS);
        sleep(1);
    }
}
