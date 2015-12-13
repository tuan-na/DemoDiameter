#include "diameter_queue_input.h"

void init_queue(struct d_queue *q){
    q->first = 0;
    q->last = -1;
    q->n = 0;
}

int is_dempty(struct d_queue *q){
    if(q->n == 0) return 1;
    return 0;
}

int is_full(struct d_queue *q){
    if(q->n == MAX_SIZE_QUEUE) return 1;
    return 0;
}

int push_in_queue(struct d_queue *q, struct d_queue_data data){
    if(is_full(q)){
        return 0;
    }
    else{
        q->last = (q->last + 1) % MAX_SIZE_QUEUE;//neu q.last dang o MAX_SIZE_QUEUE thi se ve 0
        q->data[q->last] = data;
        q->n++;
        return 1;
    }
}

struct d_queue_data pop_from_queue(struct d_queue *q){
    struct d_queue_data x;
    if(is_dempty(q)){
        return x;
    }
    x = q->data[q->first];
    q->first = (q->first+1) % MAX_SIZE_QUEUE;
    q->n--;
    return x;
}

int check_and_push(int efd, char buf[], int count, struct d_queue *q){
    if(count == 0) return -1;
    int value = atoi(buf);
    struct d_queue_data data;
    data.buf = (char*) calloc(1, strlen(buf));
    strcpy(data.buf, buf);
    data.efd = efd;
    push_in_queue(q, data);
    return 0;
}

