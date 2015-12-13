#ifndef DIAMETER_QUEUE_INPUT_H_INCLUDED
#define DIAMETER_QUEUE_INPUT_H_INCLUDED

#include <stddef.h>
#include <string.h>

#define MAX_SIZE_QUEUE 40000
int a;
struct d_queue_data{
    char *buf;
    int efd;
};

struct d_queue{
    struct d_queue_data data[MAX_SIZE_QUEUE];
    int n;
    int first, last;
};

void init_queue(struct d_queue *q);
int is_dempty(struct d_queue *q);
int is_full(struct d_queue *q);
int push_in_queue(struct d_queue *q, struct d_queue_data data);//them 1 phan tu vao cuoi queue
struct d_queue_data pop_from_queue(struct d_queue *q);//lay phan tu cuoi ra khoi queue
int check_and_push(int efd, char buf[], int count, struct d_queue *q);

#endif // DIAMETER_QUEUE_INPUT_H_INCLUDED
