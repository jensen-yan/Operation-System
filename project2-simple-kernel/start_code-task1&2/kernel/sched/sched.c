#include "lock.h"
#include "time.h"
#include "stdio.h"
#include "sched.h"
#include "queue.h"
#include "screen.h"

pcb_t pcb[NUM_MAX_TASK];

/* current running task PCB */
pcb_t *current_running;

/* global process id */
pid_t process_id = 1;

static void check_sleeping()
{
    pcb_t * p = sleep_queue.head;
    if(p != NULL)
    {
        if(get_timer() - p->begin_time >= p->sleep_time)
        {
            p = (pcb_t *)queue_dequeue(&sleep_queue);
            queue_push(&ready_queue, p);
        }
    }
}

void scheduler(void)
{
    // TODO schedule
    // Modify the current_running pointer.
    check_sleeping();
    pcb_t *next_running;
    //if(queue_is_empty(&ready_queue))
    //    next_running = current_running;
    //else
        next_running = (pcb_t *)queue_dequeue(&ready_queue);

    if(current_running->status != TASK_BLOCKED)
    {
        current_running->status = TASK_READY;
        if(current_running->pid != 1)
        {
            queue_push(&ready_queue, current_running);
        }
    }
    current_running = next_running;
    current_running->status = TASK_RUNNING;
    return ;
}

void do_sleep(uint32_t sleep_time)
{
    // TODO sleep(seconds)
    current_running->status = TASK_BLOCKED;
    current_running->begin_time = get_ticks();
    current_running->sleep_time = sleep_time;
    queue_push(&sleep_queue, (void *)current_running);
    do_scheduler();
}

void do_block(queue_t *queue)
{
    // block the current_running task into the queue
    current_running->status = TASK_BLOCKED;
    queue_push(queue, (void *)current_running);
    do_scheduler();
}

void do_unblock_one(queue_t *queue)
{
    // unblock the head task from the queue
    pcb_t *item = (pcb_t *)queue_dequeue(queue);
    item->status = TASK_READY;
    queue_push(&ready_queue, item);
}

void do_unblock_all(queue_t *queue)
{
    // unblock all task in the queue
    pcb_t *item;
    while(!queue_is_empty(queue))
    {
        item = (pcb_t *)queue_dequeue(queue);
        item->status = TASK_READY;
        queue_push(&ready_queue, item);
    }
}
