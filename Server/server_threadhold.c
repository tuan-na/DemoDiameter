#include "server_threadhold.h"

int num_msg_in_sec = 0;
long time_start_count = 0;
int accept_msg = 0;
int TPS;
int acceptTPS;

int incoming_msg(){
    if(time_start_count == 0){
        time_start_count = get_curr_time();
        num_msg_in_sec = 0;
        accept_msg = 0;
    }
    if(get_curr_time() - time_start_count  < 1000){
        num_msg_in_sec++;
        if(num_msg_in_sec > MAX_TPS) return 0;
        accept_msg++;
        return 1;
    }else{
        //reset
        TPS = num_msg_in_sec;//record TPS
        acceptTPS = accept_msg;//real TPS
        num_msg_in_sec = 1;
        accept_msg = 1;
        time_start_count = get_curr_time();
        if(num_msg_in_sec > MAX_TPS) return 0;
        return 1;
    }
}
