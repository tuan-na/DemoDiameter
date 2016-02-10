#include <stdio.h>
#include <stdlib.h>
#include "../Server/conf/dictionary_reader.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include "../Server/message/base_receive.h"
#include <sys/time.h>

#define MSG_PER_SEC 3
#define TIME_SEND_KEEP_CON 5

#define READ_DONE 1
#define READ_ERROR 0

struct epoll_event event;
struct epoll_event *events;
struct sockaddr_in client_addr;
unsigned char is_accept_from_srv = 0;
unsigned char is_waiting_srv = 0;
long timeSentBefore = 0;
long timeSentKeepCom = 0;

unsigned char check_accept_from_srv(base_receive* rev){
    avp_receive_msg* avp_msg;
    int* value;
    if(rev->command_code == CAPABILITIES_EXCHANGE){
        avp_msg = get_avp_receive_by_code(rev, 268);
        value = (int*)avp_msg->value;
        if(*value == 2001){
            is_accept_from_srv = 1;
        }else{
            return 0;
        }
        is_waiting_srv = 0;
    }
    return 1;
}

base_receive* processReadFromSrv(int fd){
    base_receive* rev_temp;
    rev_temp = create_new_receive_packet();
    rev_temp->packet = create_new_msg_packet(MESSAGE_HEADER_LEN);
    int count;
    unsigned char* buf = rev_temp->packet->data;
    //read header
    memset(buf, 0, MESSAGE_HEADER_LEN);
    count = read(fd, buf, MESSAGE_HEADER_LEN);
    if(count == -1){
        if(errno != EAGAIN){
            return READ_ERROR;
        }else{
            return READ_DONE;
        }
    }
    else if(count == 0){
        return READ_ERROR;
    }
    unpack_message_header(rev_temp);
    free(rev_temp->packet->data);
    free(rev_temp->packet);
    int data_len = rev_temp->data_len;
    rev_temp->packet = create_new_msg_packet(data_len);
    buf = rev_temp->packet->data;
    memset(buf, 0, data_len);
    count = read(fd, buf, data_len);
    if(count == -1 || count == 0){
        return READ_ERROR;
    }
    unpack_message_data(rev_temp);
    if(!is_accept_from_srv){
        if(!check_accept_from_srv(rev_temp)){
            return 0;
        }
    }

    return rev_temp;
}

unsigned char process_write(int fd, int code){
    base_send* res;
    unsigned char is_req = 1;//write request only to server
    if(code == DEVICE_WATCHDOG || code == CAPABILITIES_EXCHANGE || code == CREDIT_CONTROL){
        res = create_new_send_msg(1, code, is_req, 0, 0, 0);
        generate_avp_msg(res);
        auto_set_all_avp(res);
        create_send_data(res);
        //return res;
        write(fd, (void*)res->packet->data, res->packet->position);
        free_base_send(res);
        return 1;
    }
    return 0;
}

long get_curr_time(){
    struct timeval my_time;
    gettimeofday(&my_time, 0);
    return my_time.tv_sec * 1000 + my_time.tv_usec / 1000;
}

int check_and_send(){
    double time_send_msg = 1000 / MSG_PER_SEC;
    long curr_time = get_curr_time();
    if(timeSentBefore == 0 || curr_time - timeSentBefore >= time_send_msg){
        timeSentBefore = curr_time;
        return 1;
    }else{
        return 0;
    }
}

int check_send_keep_con(){
    int time_sent = TIME_SEND_KEEP_CON * 1000;
    long curr_time = get_curr_time();
    if(timeSentKeepCom == 0 || curr_time - timeSentKeepCom >= time_sent){
        timeSentKeepCom = curr_time;
        return 1;
    }else{
        return 0;
    }
}

pthread_t keep_con_thread;
pthread_t exchange_message_thread;
void* keep_connection(void* arg);
void* exchange_message(void* arg);
int sfd;

int main()
{
    void* thread_monitor_result;
    void* thread_exchange_result;
    read_config();

    int s;
    int len;
    is_accept_from_srv = 0;
    is_waiting_srv = 0;
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    client_addr.sin_port = htons(3868);
    len = sizeof(client_addr);
    s = connect(sfd, (struct sockaddr*)&client_addr, len);
    if(s == -1){
        perror("opps");
    	if(errno != EALREADY){
            close(sfd);
    	}
    	exit(1);
    }

    base_receive* rev;
    //init message
    process_write(sfd, CAPABILITIES_EXCHANGE);
    rev = processReadFromSrv(sfd);
    is_waiting_srv = 1;
    if(!is_accept_from_srv && rev == 0){
        return 1;
    }
    if(rev == 1) return 1;
    free_base_receive(rev);

    int res = pthread_create(&keep_con_thread, NULL, keep_connection, NULL);
	if(res != 0){
		perror("Thread creation failed");
		exit(EXIT_FAILURE);
	}

	pthread_create(&exchange_message_thread, NULL, exchange_message, NULL);
	if(res != 0){
		perror("Thread creation failed");
		exit(EXIT_FAILURE);
	}
	res = pthread_join(exchange_message_thread, &thread_exchange_result);
	if(res != 0){
        perror("Thread join failed");
        exit(EXIT_FAILURE);
	}
	res = pthread_join(keep_con_thread, &thread_monitor_result);
	if(res != 0){
        perror("Thread join failed");
        exit(EXIT_FAILURE);
	}

    return 0;
}

void* keep_connection(void* arg){
    while(1){
        if(check_send_keep_con()){
            process_write(sfd, DEVICE_WATCHDOG);
        }
    }
}

void* exchange_message(void* arg){
    base_receive* rev;
    while(1){
        //neu khong loi, response to server
        if(check_and_send() && is_accept_from_srv){
            process_write(sfd, CREDIT_CONTROL);
            rev = processReadFromSrv(sfd);
            printf("receive cmd: %d  len: %d\n", rev->command_code, rev->data_len);
            if(rev->command_code == DEVICE_WATCHDOG){
                free_base_receive(rev);
                rev = processReadFromSrv(sfd);
                printf("receive cmd: %d  len: %d\n", rev->command_code, rev->data_len);
            }
            if(rev == 0 || rev == 1){
                return 1;
            }
            free_base_receive(rev);
//            ws = write(sfd, &msg, strlen(msg));
//
//            read(sockfd, &recv, strlen(msg));
//            write(1, &recv, strlen(recv));
        }

    }
}
