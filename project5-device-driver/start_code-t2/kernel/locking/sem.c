#include "sem.h"
#include "sched.h"
#include "stdio.h"

void do_semaphore_init(semaphore_t *s, int val)
{
    s->semph = val;
    queue_init(&s->semph_queue);
}

void do_semaphore_up(semaphore_t *s)
{
    if(!queue_is_empty(&s->semph_queue))
    {
        pcb_t *item = (pcb_t *)queue_dequeue(&s->semph_queue);
        item->status = TASK_READY;
        item->my_queue = &ready_queue;
        priority_queue_push(&ready_queue, (void *)item);
    }
    else
        ++s->semph;
}

void do_semaphore_down(semaphore_t *s)
{
    if(s->semph > 0)
        --s->semph;
    else
        do_block(&s->semph_queue);
}