#ifndef INCLUDE_BARRIER_H_
#define INCLUDE_BARRIER_H_

#include "queue.h"

typedef struct barrier
{
    uint32_t barry_n;
    uint32_t waiting_num;
    queue_t barry_queue;
} barrier_t;

void do_barrier_init(barrier_t *, int);
void do_barrier_wait(barrier_t *);

#endif