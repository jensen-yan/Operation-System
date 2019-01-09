/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *         The kernel's entry, where most of the initialization work is done.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this 
 * software and associated documentation files (the "Software"), to deal in the Software 
 * without restriction, including without limitation the rights to use, copy, modify, 
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit 
 * persons to whom the Software is furnished to do so, subject to the following conditions:
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

#include "fs.h"
//#include "mm.h"
#include "irq.h"
#include "mac.h"
#include "test.h"
#include "stdio.h"
#include "sched.h"
#include "screen.h"
#include "common.h"
#include "syscall.h"

uint32_t stack_top = STACK_MAX;
uint32_t initial_cp0_status;
uint32_t exception_handler[32];
queue_t ready_queue, sleep_queue;

/* 当前应分配的pcb表的下表, 退出的任务回收的pcb表*/
uint32_t cur_pcb_id;
queue_t exit_pcb_queue;
/* 回收的栈表*/
uint32_t exit_stack[40];
int exit_stack_top = -1;

/*static void init_page_table()
{
	//free page frame from 16M-32M
	//freerange(0x1000000, 0x2000000);
	freerange(0x1000000, 0x1002000);
	//int i = 0;
	//while(pmem.top >= 0)
	//	pgtab[i++] = pmem.freestack[pmem.top--] & 0xfffff000 | PTE_C | PTE_D | PTE_V | PTE_G;

	//for(; i < NPTENTRIES; i++)
	//	pgtab[i] = PTE_C | PTE_G;
	
	//user stack, pinning pages
	bzero(pgtab, sizeof(pgtab));
	int i;
	uint32_t user_stack_top = (MAXPFRAME << PTXSHIFT) | PTE_C | PTE_D | PTE_V;
	for(i = 2; i < NUM_MAX_TASK; i++)
	{
		pcb[i].pgtab[1].ptentry = user_stack_top;
		pcb[i].pgtab[0].ptentry = user_stack_top - (1 << 12);
		user_stack_top -= 1 << 13;
	}
}

static void init_memory()
{
	freelist.head = freelist.tail = NULL;
	busylist.head = busylist.tail = NULL; //initial

	TLB_flush();
	init_page_table();
	//In task1&2, page table is initialized completely with address mapping, but only virtual pages in task3.
	//init_TLB();		//only used in P4 task1
	//init_swap();	//only used in P4 bonus: Page swap mechanism
}*/

static void init_pcb()
{
	int i;

	queue_init(&ready_queue);
	queue_init(&sleep_queue);
	queue_init(&exit_pcb_queue);
	
	/* info about shell kernel */
	pcb[0].pid = 0;
	pcb[0].status = TASK_RUNNING;
	pcb[0].lock_top = -1;
	queue_init(&pcb[0].wait_queue);
	pcb[0].my_queue = NULL;
	pcb[0].type = KERNEL_PROCESS;

	pcb_t *new_pcb = &pcb[1];
    //priority: '->' > '&' 
    bzero(&new_pcb->kernel_context, sizeof(new_pcb->kernel_context));
	bzero(&new_pcb->user_context, sizeof(new_pcb->user_context));

	// stack 
	new_pcb->kernel_stack_top = new_pcb->kernel_context.regs[29] = new_stack();
    new_pcb->user_stack_top = new_pcb->user_context.regs[29] = new_stack();

	new_pcb->kernel_context.regs[31] = (uint32_t)&first_run_handle;
	new_pcb->kernel_context.cp0_status = 0x10008003;
	new_pcb->kernel_context.cp0_epc = (uint32_t)&test_shell;

	new_pcb->user_context.regs[31] = (uint32_t)&test_shell;
	new_pcb->user_context.cp0_status = 0x10008003;
    new_pcb->user_context.cp0_epc = (uint32_t)&test_shell;

    new_pcb->lock_top = -1;
    queue_init(&new_pcb->wait_queue);
    new_pcb->pid = ((uint32_t)new_pcb - (uint32_t)pcb)/sizeof(pcb_t);
    new_pcb->my_queue = &ready_queue;

    new_pcb->type = USER_PROCESS;
	new_pcb->status = TASK_READY;
	//new_pcb->task_priority = task->priority;
	//new_pcb->priority = task->priority;
	priority_queue_push(&ready_queue, (void *)new_pcb);

	cur_pcb_id = 2;
	/* current running */
	current_running = &pcb[0];
}

static void init_exception_handler()
{
	int i;
	exception_handler[0] = (uint32_t)&handle_int;
	for(i = 1; i < 32; i++)
		exception_handler[i] = (uint32_t)&handle_other;
	//exception_handler[2] = (uint32_t)&handle_TLB;
	//exception_handler[3] = (uint32_t)&handle_TLB;
	exception_handler[8] = (uint32_t)&handle_syscall;
}

static void init_exception()
{
	init_exception_handler();
	// 1. Get CP0_STATUS
	initial_cp0_status = get_cp0_status();

	// 2. Disable all interrupt
	initial_cp0_status |= 0x1000ff01;
	initial_cp0_status ^= 0x1;
	set_cp0_status(initial_cp0_status);
	initial_cp0_status |= 0x1;

	// 3. Copy the level 2 exception handling code to 0x80000180
	bzero((void *)BEV0_EBASE, BEV0_OFFSET);
	memcpy((void *)BEV0_EBASE, TLBexception_handler_begin, TLBexception_handler_end-TLBexception_handler_begin);
	memcpy((void *)0x80000180, exception_handler_entry, exception_handler_end-exception_handler_begin);
	bzero((void *)BEV1_EBASE, BEV1_OFFSET);
	memcpy((void *)BEV1_EBASE, TLBexception_handler_begin, TLBexception_handler_end-TLBexception_handler_begin);
	memcpy((void *)0xbfc00380, exception_handler_entry, exception_handler_end-exception_handler_begin);
	
	// 4. reset CP0_COMPARE & CP0_COUNT register
	set_cp0_COUNT(0);
	set_cp0_COMPARE(0x30000);
}

static void init_syscall(void)
{
	// init system call table.
	int i;
	for(i = 0; i < NUM_SYSCALLS; i++)
		syscall[i] = (int (*)())&sys_other;
	syscall[SYSCALL_SLEEP              ] = (int (*)()) & do_sleep;
	syscall[SYSCALL_GETPID             ] = (int (*)()) & do_getpid;
	syscall[SYSCALL_BLOCK              ] = (int (*)()) & do_block;
	syscall[SYSCALL_UNBLOCK_ONE        ] = (int (*)()) & do_unblock_one;
	syscall[SYSCALL_UNBLOCK_ALL        ] = (int (*)()) & do_unblock_all;
	syscall[SYSCALL_WRITE              ] = (int (*)()) & screen_write;
	syscall[SYSCALL_CURSOR             ] = (int (*)()) & screen_move_cursor;
	syscall[SYSCALL_REFLUSH            ] = (int (*)()) & screen_reflush;
	syscall[SYSCALL_PS                 ] = (int (*)()) & do_ps;
	syscall[SYSCALL_CLEAR              ] = (int (*)()) & do_clear;
	syscall[SYSCALL_SPAWN              ] = (int (*)()) & do_spawn;
	syscall[SYSCALL_KILL               ] = (int (*)()) & do_kill;
	syscall[SYSCALL_EXIT               ] = (int (*)()) & do_exit;
	syscall[SYSCALL_WAIT               ] = (int (*)()) & do_wait;
	syscall[SYSCALL_MUTEX_LOCK_INIT    ] = (int (*)()) & do_mutex_lock_init;
	syscall[SYSCALL_MUTEX_LOCK_ACQUIRE ] = (int (*)()) & do_mutex_lock_acquire;
	syscall[SYSCALL_MUTEX_LOCK_RELEASE ] = (int (*)()) & do_mutex_lock_release;
	syscall[SYSCALL_CLEAR_ALL          ] = (int (*)()) & do_clear_all;
	syscall[SYSCALL_BARRIER_INIT       ] = (int (*)()) & do_barrier_init;
	syscall[SYSCALL_BARRIER_WAIT       ] = (int (*)()) & do_barrier_wait; 
	syscall[SYSCALL_SEMAPHORE_INIT     ] = (int (*)()) & do_semaphore_init;
	syscall[SYSCALL_SEMAPHORE_UP       ] = (int (*)()) & do_semaphore_up;
	syscall[SYSCALL_SEMAPHORE_DOWN     ] = (int (*)()) & do_semaphore_down;
	syscall[SYSCALL_CONDITION_INIT     ] = (int (*)()) & do_condition_init;
	syscall[SYSCALL_CONDITION_WAIT     ] = (int (*)()) & do_condition_wait;
	syscall[SYSCALL_CONDITION_SIGNAL   ] = (int (*)()) & do_condition_signal;
	syscall[SYSCALL_CONDITION_BROADCAST] = (int (*)()) & do_condition_broadcast;
	syscall[SYSCALL_INIT_MAC           ] = (int (*)()) & do_init_mac;
	syscall[SYSCALL_NET_SEND           ] = (int (*)()) & do_net_send;
	syscall[SYSCALL_NET_RECV           ] = (int (*)()) & do_net_recv;
	syscall[SYSCALL_WAIT_RECV          ] = (int (*)()) & do_wait_recv_package;
	syscall[SYSCALL_MKFS               ] = (int (*)()) & do_mkfs;
	syscall[SYSCALL_STATFS             ] = (int (*)()) & do_statfs;
	syscall[SYSCALL_MKDIR              ] = (int (*)()) & do_mkdir;
	syscall[SYSCALL_RMDIR              ] = (int (*)()) & do_rmdir;
	syscall[SYSCALL_CD                 ] = (int (*)()) & do_cd;
	syscall[SYSCALL_LS                 ] = (int (*)()) & do_ls;
	syscall[SYSCALL_TOUCH              ] = (int (*)()) & do_touch;
	syscall[SYSCALL_CAT                ] = (int (*)()) & do_cat;
	syscall[SYSCALL_FOPEN              ] = (int (*)()) & do_fopen;
	syscall[SYSCALL_FREAD              ] = (int (*)()) & do_fread;
	syscall[SYSCALL_FWRITE             ] = (int (*)()) & do_fwrite;
	syscall[SYSCALL_CLOSE              ] = (int (*)()) & do_close;
	syscall[SYSCALL_CFS                ] = (int (*)()) & do_cfs;
}

// jump from bootloader.
// The beginning of everything >_< ~~~~~~~~~~~~~~
void __attribute__((section(".entry_function"))) _start(void)
{
	// Close the cache, no longer refresh the cache 
	// when making the exception vector entry copy
	asm_start();

	// init interrupt (^_^)
	init_exception();
	printk("> [INIT] Interrupt processing initialization succeeded.\n");

	// init virtual memory
	//init_memory();
	//printk("> [INIT] Virtual memory initialization succeeded.\n");

	// init system call table (0_0)
	init_syscall();
	printk("> [INIT] System call initialized successfully.\n");

	// init Process Control Block (-_-!)
	init_pcb();
	printk("> [INIT] PCB initialization succeeded.\n");

	//init file system
	if(init_fs() == 0)
		printk("> [INIT] No File System.\n");	
	else
		printk("> [INIT] File System initialization succeeded.\n");

	// init screen (QAQ)
	init_screen();
	my_printk("> [INIT] SCREEN initialization succeeded.\n");

	// TODO Enable interrupt
	set_cp0_status(initial_cp0_status);
	while (1)
	{
		// (QAQQQQQQQQQQQ)
		// If you do non-preemptive scheduling, you need to use it to surrender control
		//do_scheduler();
	};
	return;
}
