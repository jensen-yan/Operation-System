#include "barrier.h"

void do_barrier_init(barrier_t *barrier, int goal)
{
    barrier->barry_n = goal;
    barrier->waiting_num = 0;
    queue_init(&barrier->barry_queue);
}

void do_barrier_wait(barrier_t *barrier)
{
    ++barrier->waiting_num;
    if(barrier->waiting_num < barrier->barry_n)
        do_block(&barrier->barry_queue);
    else
    {
        do_unblock_all(&barrier->barry_queue);
        do_scheduler();
    }
}