#ifndef MONITOR_TRANSACTION_H_INCLUDED
#define MONITOR_TRANSACTION_H_INCLUDED
#include <pthread.h>
#include "utility.h"
#include "server_threadhold.h"

extern pthread_t monitor_thread;

void* monitoring_income(void * arg);

#endif // MONITOR_TRANSACTION_H_INCLUDED
