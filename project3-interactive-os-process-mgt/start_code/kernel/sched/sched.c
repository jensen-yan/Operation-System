#include "lock.h"
#include "time.h"
#include "stdio.h"
#include "sched.h"
#include "queue.h"
#include "irq.h"
#include "screen.h"
#include "test3.h"

pcb_t pcb[NUM_MAX_TASK];

/* current running task PCB */
pcb_t *current_running;

/* global process id */
pid_t process_id = 1;

static void check_sleeping()
{
    pcb_t *p = sleep_queue.head;
    uint32_t time = get_timer();
    while(p != NULL)
    {
        if(time - p->begin_time >= p->sleep_time)
        {
            pcb_t *q = (pcb_t *)queue_remove(&sleep_queue, (void *)p);
            p->sleep_time = 0;
            p->status = TASK_READY;
            p->my_queue = &ready_queue;
            priority_queue_push(&ready_queue, (void *)p);
            p = q;
        }
        else
            p = p->next;
    }
}

void scheduler(void)
{
    // TODO schedule
    // Modify the current_running pointer.
    check_sleeping();
    pcb_t *next_running, *p;
    if(queue_is_empty(&ready_queue))
        next_running = current_running;
    else
        next_running = (pcb_t *)queue_dequeue(&ready_queue);

    if(current_running->status != TASK_BLOCKED && next_running != current_running \
        && current_running->status != TASK_EXITED)
    {
        current_running->status = TASK_READY;
        current_running->my_queue = &ready_queue;
        priority_queue_push(&ready_queue, current_running);
    }
    current_running = next_running;
    current_running->priority = current_running->task_priority;
    current_running->status = TASK_RUNNING;
    current_running->my_queue = NULL;

    p = (pcb_t *)ready_queue.head;
    while(p != NULL)
    {
        p->priority += 1;
        p = p->next;
    }
}

void do_sleep(uint32_t sleep_time)
{
    // TODO sleep(seconds)
    current_running->status = TASK_BLOCKED;
    current_running->my_queue = &sleep_queue;
    current_running->begin_time = get_timer();
    current_running->sleep_time = sleep_time;
    queue_push(&sleep_queue, (void *)current_running);
    do_scheduler();
}

void do_block(queue_t *queue)
{
    // block the current_running task into the queue
    current_running->status = TASK_BLOCKED;
    current_running->my_queue = queue;
    priority_queue_push(queue, (void *)current_running);
    do_scheduler();
}

void do_unblock_one(queue_t *queue)
{
    // unblock the head task from the queue
    pcb_t *p = (pcb_t *)(queue->head);
    while(p != NULL)
    {
        p->priority += 1;
        p = p->next;
    }

    pcb_t *item = (pcb_t *)queue_dequeue(queue);
    item->status = TASK_READY;
    item->my_queue = &ready_queue;
    priority_queue_push(&ready_queue, item);
}

void do_unblock_all(queue_t *queue)
{
    // unblock all task in the queue
    pcb_t *item;
    while(!queue_is_empty(queue))
    {
        item = (pcb_t *)queue_dequeue(queue);
        item->status = TASK_READY;
        item->my_queue = &ready_queue;
        priority_queue_push(&ready_queue, item);
    }
}

pid_t do_getpid()
{
    return current_running->pid;
}

void do_ps()
{
    int i = 1;
    pcb_t *tmp = (pcb_t *)ready_queue.head;
    my_printk("[PROCESS TABLE]\n");
    my_printk("[0] PID : %d STATUS : RUNNING\n", current_running->pid);
    while(tmp != NULL)
    {
        my_printk("[%d] PID : %d STATUS : RUNNING\n", i++, tmp->pid);
        tmp = tmp->next;
    }
    tmp = (pcb_t *)sleep_queue.head;
    while(tmp != NULL)
    {
        my_printk("[%d] PID : %d STATUS : SLEEP\n", i++, tmp->pid);
        tmp  = tmp->next;
    }
}

void do_clear()
{
    screen_clear(SCREEN_HEIGHT / 2 + 1, SCREEN_HEIGHT);
    screen_move_cursor(0, SCREEN_HEIGHT / 2 + 1);
}

void do_clear_all()
{
    screen_clear(0, SCREEN_HEIGHT);
    screen_move_cursor(0, SCREEN_HEIGHT / 2);
    my_printk("-------------------- COMMAND --------------------\n");
}

uint32_t new_stack()
{
    uint32_t new_stack_top = stack_top;
    if(exit_stack_top < 0)
        stack_top -= PCB_STACK_SIZE;
    else
        new_stack_top = exit_stack[exit_stack_top--];
    return new_stack_top;
}

void do_spawn(task_info_t *task)
{
    pcb_t *new_pcb;
    if(queue_is_empty(&exit_pcb_queue))
        new_pcb = &pcb[cur_pcb_id++];
    else
        new_pcb = queue_dequeue(&exit_pcb_queue);
    //priority: '->' > '&' 
    bzero(&new_pcb->kernel_context, sizeof(new_pcb->kernel_context));
	bzero(&new_pcb->user_context, sizeof(new_pcb->user_context));

	// stack 
	new_pcb->kernel_stack_top = new_pcb->kernel_context.regs[29] = new_stack();
    new_pcb->user_stack_top = new_pcb->user_context.regs[29] = new_stack();

	new_pcb->kernel_context.regs[31] = (uint32_t)&first_run_handle;
	new_pcb->kernel_context.cp0_status = 0x10008003;
	new_pcb->kernel_context.cp0_epc = task->entry_point;

	new_pcb->user_context.regs[31] = task->entry_point;
	new_pcb->user_context.cp0_status = 0x10008003;
    new_pcb->user_context.cp0_epc = task->entry_point;

    new_pcb->lock_top = -1;
    queue_init(&new_pcb->wait_queue);
    new_pcb->pid = ((uint32_t)new_pcb - (uint32_t)pcb)/sizeof(pcb_t);
    new_pcb->my_queue = &ready_queue;

    new_pcb->type = task->type;
	new_pcb->status = TASK_READY;
	//new_pcb->task_priority = task->priority;
	//new_pcb->priority = task->priority;
	priority_queue_push(&ready_queue, (void *)new_pcb);
}

void do_kill(pid_t pid)
{
    int i;
    pcb_t *victim = &pcb[pid];
    victim->status = TASK_EXITED;
    /* remove victim from task queue */
    if(victim->my_queue != NULL)
    {
        queue_remove(victim->my_queue, (void *)victim);
        victim->my_queue = NULL;
    }
    /* release lock */
    for(i = 0; i <= victim->lock_top; i++)
        do_mutex_lock_release(victim->lock[i]);
    /* release wait task */
    while(!queue_is_empty(&victim->wait_queue))
    {
        pcb_t *wait_task = queue_dequeue(&victim->wait_queue);
        wait_task->status = TASK_READY;
        wait_task->my_queue = &ready_queue;
        priority_queue_push(&ready_queue, (void *)wait_task);
    }
    /* stack space */
    exit_stack[++exit_stack_top] = victim->kernel_stack_top;
    exit_stack[++exit_stack_top] = victim->user_stack_top;
    /* pcb space */
    queue_push(&exit_pcb_queue, (void *)victim);
}

void do_exit(void)
{
    int i;
    pcb_t *victim = current_running;
    victim->status = TASK_EXITED;
    victim->my_queue = NULL;
    /* release lock */
    for(i = 0; i <= victim->lock_top; i++)
        do_mutex_lock_release(victim->lock[i]);
    /* release wait task */
    while(!queue_is_empty(&victim->wait_queue))
    {
        pcb_t *wait_task = queue_dequeue(&victim->wait_queue);
        wait_task->status = TASK_READY;
        wait_task->my_queue = &ready_queue;
        priority_queue_push(&ready_queue, (void *)wait_task);
    }
    /* stack space */
    exit_stack[++exit_stack_top] = victim->kernel_stack_top;
    exit_stack[++exit_stack_top] = victim->user_stack_top;
    /* pcb space */
    queue_push(&exit_pcb_queue, (void *)victim);
    /* scheduler to next task */
    do_scheduler();
}

void do_wait(pid_t pid)
{
    if(pcb[pid].my_queue == NULL)
    {
        screen_move_cursor(0, screen_cursor_y);
        my_printk("\nWaiting task is not exist!                 \n");
    }
    else
    {
        current_running->status = TASK_BLOCKED;
        current_running->my_queue = &pcb[pid].wait_queue;
        queue_push(&pcb[pid].wait_queue, (void *)current_running);
        do_scheduler();
    }
    
}