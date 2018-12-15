/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
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

#ifndef INCLUDE_TEST_H_
#define INCLUDE_TEST_H_

#include "test2.h"
#include "test3.h"
#include "test4.h"
#include "sched.h"
#define PSIZE (256)
#define PNUM (64)
#define SEND_BUFFER (0xa1200000)
#define RECV_BUFFER ((SEND_BUFFER) + (PNUM) *( PSIZE))
#define SEND_DESC ((RECV_BUFFER) + (PSIZE + 16) * PNUM)
#define RECV_DESC (SEND_DESC + (16) * PNUM)
#define LAST_RECV_DESC (RECV_DESC+(PNUM-1)*16)

void test_shell_task1(void);
void test_shell_task2(void);
void test_shell_task3(void);
void test_shell(void);

//extern void do_wait_recv_package(void);
extern uint32_t *Recv_desc;

extern struct task_info *sched1_tasks[16];
extern int num_sched1_tasks;

extern struct task_info *lock_tasks[16];
extern int num_lock_tasks;

extern struct task_info *timer_tasks[16];
extern int num_timer_tasks;

extern struct task_info *sched2_tasks[16];
extern int num_sched2_tasks;

extern struct task_info *kill_tasks[16];
extern int num_kill_tasks;

extern struct task_info *wait_tasks[16];
extern int num_wait_tasks;

extern struct task_info *cond_tasks[16];
extern int num_cond_tasks;

extern struct task_info *sem_tasks[16];
extern int num_sem_tasks;

extern struct task_info *barrier_tasks[16];
extern int num_barrier_tasks;

extern struct task_info *mail_tasks[16];
extern int num_mail_tasks;

extern struct task_info *shell_tasks[16];
extern int num_shell_tasks;

extern struct task_info *net_tasks[16];
extern int num_net_tasks;
#endif