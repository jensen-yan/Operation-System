#ifndef INCLUDE_SEM_H_
#define INCLUDE_SEM_H_

#include "queue.h"
#include "type.h"

typedef struct semaphore
{
    int semph;
    queue_t semph_queue;
} semaphore_t;

void do_semaphore_init(semaphore_t *, int);
void do_semaphore_up(semaphore_t *);
void do_semaphore_down(semaphore_t *);

#endif