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
#include "utility.h"
#include <string.h>
#include "server_threadhold.h"
#include "conf/dictionary_reader.h"
#include "message/base_message.h"
#include "datastruct/vector.h"
#include "datastruct/hashmap.h"
#include "datastruct/hashmapi.h"
#include "message/base_receive.h"
#include "packet_process.h"

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
volatile int queue_suspended;
volatile int is_on_reading = 1;

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

    read_config();
/*
    printf("flag value :%d\n", gen_flag_value(1, 1, 0, 1));
    msg_packet* a = create_new_msg_packet(512);
    put_int32(a, -12345);
    put_str(a, "haha");
    //put_flex_int(a, 123, 3);
    printf("data: %d %d %d %d %d %d %d %d\n", a->data[0], a->data[1], a->data[2], a->data[3], a->data[4], a->data[5],
    a->data[6], a->data[7]);
    int* type;
    hashmapi_get(data_type_def, Enumerated, (void**)(&type));
    printf("type: %d\n", *type);

    base_send* send = create_new_send_msg(1, CREDIT_CONTROL, 1, 0, 0, 0);
    printf("cmd name: %s\n", send->name);
    generate_avp_msg(send);
    auto_set_all_avp(send);
    int i;
    avp_message* avp_msg;
    int* parent_type;

    for(i = 0; i < send->all_avp_msg->count; i++){
        avp_msg = (avp_message*)vector_get(send->all_avp_msg, i);
        avp_represent* aaa = avp_msg->avp_rep;
        hashmapi_get(data_type_def, avp_msg->avp_rep->type, (void **)(&parent_type));
        if(*parent_type == OctetString || *parent_type == UTF8String){
            printf("avp name: %s code:%d value:%s len: %d\n", avp_msg->avp_rep->name, avp_msg->avp_rep->code, (char*)avp_msg->value, 0);
        }else if(*parent_type == Integer32 || *parent_type == Unsigned32){
            printf("avp name: %s code:%d value:%d len: %d\n", avp_msg->avp_rep->name, avp_msg->avp_rep->code, *(int*)avp_msg->value, 0);
        }else if(*parent_type== Integer64 || *parent_type == Unsigned64){
            printf("avp name: %s code:%d value:%d len: %d\n", avp_msg->avp_rep->name, avp_msg->avp_rep->code, *(long*)avp_msg->value, 0);
        }else if(*parent_type == Enumerated){
            printf("avp name: %s code:%d value:%d len: %d\n", avp_msg->avp_rep->name, avp_msg->avp_rep->code, *(int*)avp_msg->value, 0);
        }else{
            printf("avp name: %s code:%d type:%d len: %d\n", avp_msg->avp_rep->name, avp_msg->avp_rep->code, avp_msg->avp_rep->type, 0);
        }
    }

    create_send_data(send);
    for(i = 0; i < send->packet->position; i++){
        //printf("%d", send->packet->data[i]);
    }
    void* aa = malloc(sizeof(int));
    *((int*)aa) = 4;
    printf("as: %d\n", *(int*)aa);
    base_receive* rev = create_new_receive_packet();
    send->packet->max_size = send->packet->position;
    send->packet->position = 0;
    int asa = send->packet->max_size;
    rev->packet = send->packet;
    unpack_message_header(rev);
    unpack_message_data(rev);
    printf("code: %d , appid: %d, eid:%d\n", rev->command_code, rev->app_id, rev->end_end_id);
    avp_receive_msg* avp_rev, *avp1;
    int j;
    for(i = 0; i < rev->all_avp_msg->count; i++){
        avp_rev = (avp_receive_msg*)vector_get(rev->all_avp_msg, i);
        hashmapi_get(data_type_def, avp_rev->avp_rep->type, (void **)(&parent_type));
        if(*parent_type == OctetString || *parent_type == UTF8String){
            printf("avp name: %s code:%d value:%s\n", avp_rev->avp_rep->name, avp_rev->avp_rep->code, (char*)avp_rev->value);
        }else if(*parent_type == Integer32 || *parent_type == Unsigned32){
            printf("avp name: %s code:%d value:%d\n", avp_rev->avp_rep->name, avp_rev->avp_rep->code, *(int*)avp_rev->value);
        }else if(*parent_type== Integer64 || *parent_type == Unsigned64){
            printf("avp name: %s code:%d value:%d\n", avp_rev->avp_rep->name, avp_rev->avp_rep->code, *(long*)avp_rev->value);
        }else if(*parent_type == Enumerated){
            printf("avp name: %s code:%d value:%d\n", avp_rev->avp_rep->name, avp_rev->avp_rep->code, *(int*)avp_rev->value);
        }else if(*parent_type == Grouped){
            printf("avp name: %s code: %d\n", avp_rev->avp_rep->name, avp_rev->avp_rep->code);
            print_avp_child(avp_rev);
        }else{
            printf("avp name: %s code:%d type:%d\n", avp_rev->avp_rep->name, avp_rev->avp_rep->code, avp_rev->avp_rep->type);
        }
    }

    free_base_send(send);
    free_base_receive(rev);
*/


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

struct epoll_event event;
struct epoll_event *events;
struct sockaddr_in server_addr;
struct sockaddr_in client_address;
void* listening_client(void* arg){
    int sfd, efd, s;
    int server_len;
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(3868);
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
    efd = epoll_create1(0);//create new epoll instance
    if(efd == -1){
        perror("epoll create");
        pthread_exit(NULL);
    }
    event.data.fd = sfd;//add file descriptor
    event.events = EPOLLIN | EPOLLET;
    s = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event);//add fd to watch on epoll instance
    if(s == -1){
        perror("epoll_ctl");
        pthread_exit(NULL);
    }
    events = calloc(MAX_EVENTS, sizeof event);

    base_receive* rev;
    base_send* res;
    while(1){//while to read all event occur
        int n, i;
        n = epoll_wait(efd, events, MAX_EVENTS, -1);//wait event on file descriptor, that will block until events are available
        //n: number of connection
        for(i = 0; i < n; i++){
            if((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN))){
                fprintf(stderr, "epoll error\n");
                close(events[i].data.fd);
                continue;
            }
            else if(sfd == events[i].data.fd){
            /* We have a notification on the listening socket, which
                 means one or more incoming connections. */
                while(1){//while for all incoming connect
                    int infd;
                    int in_len;
                    in_len = sizeof(client_address);
                    infd = accept(sfd, (struct sockaddr*) &client_address, &in_len);
                    if(infd == -1){
                        if(errno == EAGAIN || errno == EWOULDBLOCK){
                            //if don't have any connection then break from while
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
                    //processed each connections
                    /* add incomming socket to the
                     list of fds to monitor. */
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
                    char buf[5];
                    memset(buf, 0, sizeof(buf));
                    //count = read(events[i].data.fd, buf, sizeof(buf));
                    rev = process_read(events[i].data.fd);
                    if(rev == READ_ERROR){
                        perror("read");
                        done = 1;
                        break;
                    }
                    else if(rev == READ_DONE){
                        break;
                    }else{
                        if(rev->command_code == DEVICE_WATCHDOG){
                            res = process_gen_keep_con(rev);
                            write(events[i].data.fd, res->packet->data, res->length);
                            free_base_receive(rev);
                            free_base_send(res);
                        }else if(rev->command_code == CAPABILITIES_EXCHANGE){
                            res = process_gen_init_con(rev);
                            write(events[i].data.fd, res->packet->data, res->length);
                            free_base_receive(rev);
                            free_base_send(res);
                        }else{
                            pthread_mutex_lock(&monitor_mutex);
                            over_msg = incoming_msg();
                            pthread_mutex_unlock(&monitor_mutex);
                            if(over_msg){
                                //push message to queue
                                pthread_mutex_lock(&queue_mutex);
                                check_and_push(events[i].data.fd, rev, rev->length, &queue);
                                pthread_mutex_unlock(&queue_mutex);
                                pthread_mutex_lock(&status_read_mutex);
                                if(!is_on_reading){
                                    is_on_reading = 1;
                                    pthread_mutex_unlock(&status_read_mutex);
                                    resumeQueueThread();
                                }else{
                                    pthread_mutex_unlock(&status_read_mutex);
                                }
                            }else{
                                res = process_gen_response_error(rev);
                                write(events[i].data.fd, res->packet->data, res->length);
                                free_base_receive(rev);
                                free_base_send(res);
                            }
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
    base_send* res;
    base_receive* rev;
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
            res = process_gen_response(&data);
            write(data.efd, res->packet->data, res->length);
            free_base_send(res);
            free_base_receive(data.rev);
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

void print_avp_child(avp_receive_msg* avp_rev){
    int i, j;
    avp_receive_msg* avp1;
    int* parent_type;
    printf("child num %d:\n", avp_rev->child_avp->count);
    avp_receive_msg* child_rev;
    for(i = 0; i < avp_rev->child_avp->count; i++){
        child_rev = (avp_receive_msg*)vector_get(avp_rev->child_avp, i);
        hashmapi_get(data_type_def, child_rev->avp_rep->type, (void **)(&parent_type));
        if(*parent_type == OctetString || *parent_type == UTF8String){
            printf("avp name: %s code:%d value:%s\n", child_rev->avp_rep->name, child_rev->avp_rep->code, (char*)child_rev->value);
        }else if(*parent_type == Integer32 || *parent_type == Unsigned32){
            printf("avp name: %s code:%d value:%d\n", child_rev->avp_rep->name, child_rev->avp_rep->code, *(int*)child_rev->value);
        }else if(*parent_type== Integer64 || *parent_type == Unsigned64){
            printf("avp name: %s code:%d value:%d\n", child_rev->avp_rep->name, child_rev->avp_rep->code, *(long*)child_rev->value);
        }else if(*parent_type == Enumerated){
            printf("avp name: %s code:%d value:%d\n", child_rev->avp_rep->name, child_rev->avp_rep->code, *(int*)child_rev->value);
        }else if(*parent_type == Grouped){
            print_avp_child(child_rev);
        }else{
            printf("avp name: %s code:%d type:%d\n", avp_rev->avp_rep->name, avp_rev->avp_rep->code, avp_rev->avp_rep->type);
        }
    }
}
