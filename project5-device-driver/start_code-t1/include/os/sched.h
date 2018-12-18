/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *        Process scheduling related content, such as: scheduler, process blocking, 
 *                 process wakeup, process creation, process kill, etc.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE. 
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#ifndef INCLUDE_SCHEDULER_H_
#define INCLUDE_SCHEDULER_H_

#include "type.h"
#include "queue.h"
#include "lock.h"
#include "mm.h"

#define NUM_MAX_TASK 16
#define STACK_MAX 0xa0f00000
#define STACK_MIN 0xa0d00000
#define PCB_STACK_SIZE 0x10000
#define NUM_LOCK 4
/* used to save register infomation */
typedef struct regs_context
{
    /* Saved main processor registers.*/
    /* 32 * 4B = 128B */
    uint32_t regs[32];

    /* Saved special registers. */
    /* 7 * 4B = 28B */
    uint32_t cp0_status;
    uint32_t hi;
    uint32_t lo;
    uint32_t cp0_badvaddr;
    uint32_t cp0_cause;
    uint32_t cp0_epc;
    uint32_t pc;

} regs_context_t; /* 128 + 28 = 156B */

typedef enum {
    TASK_BLOCKED,
    TASK_RUNNING,
    TASK_READY,
    TASK_EXITED,
} task_status_t;

typedef enum {
    KERNEL_PROCESS,
    KERNEL_THREAD,
    USER_PROCESS,
    USER_THREAD,
} task_type_t;

/* Process Control Block */
typedef struct pcb
{
    /* register context */
    regs_context_t kernel_context;
    regs_context_t user_context;
    
    uint32_t kernel_stack_top;
    uint32_t user_stack_top;

    /* lock */
    mutex_lock_t *lock[NUM_LOCK];
    int lock_top;

    /* User stack page*/
    pte_t pgtab[2];

    /* wait queue */
    queue_t wait_queue;

    /* sleep time*/
    uint32_t begin_time;
    uint32_t sleep_time;

    /* cursor position */
    int cursor_x;
    int cursor_y;
    
    /* previous, next pointer */
    void *prev;
    void *next;

    /* process id */
    pid_t pid;

    /* the queue which the task is in */
    queue_t *my_queue;

    /* kernel/user thread/process */
    task_type_t type;

    /* BLOCK | READY | RUNNING */
    task_status_t status;

    /* Priority */
    int priority;
    int task_priority;
} pcb_t;

/* task information, used to init PCB */
typedef struct task_info
{
    char *name;
    uint32_t entry_point;
    task_type_t type;
    //int priority;
} task_info_t;

/* ready queue to run */
extern queue_t ready_queue;

/* sleep queue to wait */
extern queue_t sleep_queue;

/* current running task PCB */
extern pcb_t *current_running;
extern pid_t process_id;

extern pcb_t pcb[NUM_MAX_TASK];
extern uint32_t initial_cp0_status;
extern uint32_t exception_handler[32];

void do_scheduler(void);
void do_sleep(uint32_t);

void do_block(queue_t *);
void do_unblock_one(queue_t *);
void do_unblock_all(queue_t *);

uint32_t new_stack();
extern uint32_t stack_top;

/* 当前应分配的pcb表的下表, 退出的任务回收的pcb表*/
extern uint32_t cur_pcb_id;
extern queue_t exit_pcb_queue;

extern uint32_t exit_stack[40];
extern int exit_stack_top;

pid_t do_getpid(void);
void test_shell();

void do_ps(void);
void do_clear(void);
void do_clear_all(void);

void do_spawn(task_info_t *);
void do_kill(pid_t);
void do_exit(void);
void do_wait(pid_t);

#endif