#ifndef SERVER_THREADHOLD_H_INCLUDED
#define SERVER_THREADHOLD_H_INCLUDED
#include <pthread.h>
#include "utility.h"

#define MAX_TPS 10
//monitor for income message. if the number of message get over threadhold, ignore it.

extern int num_msg_in_sec;//monitor for number of message
extern long time_start_count;//time count for message, reset every second.
extern int accept_msg;
extern int TPS;// message per sec
extern int acceptTPS;

int incoming_msg();


#endif // SERVER_THREADHOLD_H_INCLUDED
