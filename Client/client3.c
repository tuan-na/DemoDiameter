#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#define MSG_PER_SEC 3

long timeSentBefore = 0;

int main(){
    int sockfd;
    int len;
    struct sockaddr_in addr;
    int result;
    char msg[100];
    char recv[100];
    strcpy(msg, "test\n");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(8500);
    len = sizeof(addr);
    result = connect(sockfd, (struct sockaddr*)&addr, len);
    if(result == -1){
    	perror("opps: client3");
    	if(errno != EALREADY){
            close(sockfd);
    	}
    	exit(1);
    }
    while(1){
        if(check_and_send()){
            write(sockfd, &msg, strlen(msg));
            read(sockfd, &recv, strlen(msg));
            write(1, &recv, strlen(recv));
        }
    }
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
