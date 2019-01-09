#include "lock.h"
#include "sched.h"
#include "common.h"
#include "screen.h"
#include "syscall.h"

void system_call_helper(int fn, int arg1, int arg2, int arg3)
{
    // syscall[fn](arg1, arg2, arg3)
    current_running->user_context.cp0_epc = current_running->user_context.cp0_epc + 4;
    int ret = syscall[fn](arg1, arg2, arg3);
    current_running->user_context.regs[2] = ret;
}

void sys_sleep(uint32_t time)
{
    invoke_syscall(SYSCALL_SLEEP, time, IGNORE, IGNORE);
}

pid_t sys_getpid()
{
    invoke_syscall(SYSCALL_GETPID, IGNORE, IGNORE, IGNORE);
}

void sys_block(queue_t *queue)
{
    invoke_syscall(SYSCALL_BLOCK, (int)queue, IGNORE, IGNORE);
}

void sys_unblock_one(queue_t *queue)
{
    invoke_syscall(SYSCALL_UNBLOCK_ONE, (int)queue, IGNORE, IGNORE);
}

void sys_unblock_all(queue_t *queue)
{
    invoke_syscall(SYSCALL_UNBLOCK_ALL, (int)queue, IGNORE, IGNORE);
}

void sys_write(char *buff)
{
    invoke_syscall(SYSCALL_WRITE, (int)buff, IGNORE, IGNORE);
}

void sys_move_cursor(int x, int y)
{
    invoke_syscall(SYSCALL_CURSOR, x, y, IGNORE);
}

void sys_reflush()
{
    invoke_syscall(SYSCALL_REFLUSH, IGNORE, IGNORE, IGNORE);
}

void sys_ps()
{
    invoke_syscall(SYSCALL_PS, IGNORE, IGNORE, IGNORE);
}

void sys_clear()
{
    invoke_syscall(SYSCALL_CLEAR, IGNORE, IGNORE, IGNORE);
}

void sys_clear_all()
{
    invoke_syscall(SYSCALL_CLEAR_ALL, IGNORE, IGNORE, IGNORE);
}

void sys_spawn(task_info_t *task)
{
    invoke_syscall(SYSCALL_SPAWN, (int)task, IGNORE, IGNORE);
}

void sys_kill(pid_t pid)
{
    invoke_syscall(SYSCALL_KILL, (int)pid, IGNORE, IGNORE);
}

void sys_exit()
{
    invoke_syscall(SYSCALL_EXIT, IGNORE, IGNORE, IGNORE);
}

void sys_waitpid(pid_t pid)
{
    invoke_syscall(SYSCALL_WAIT, (int)pid, IGNORE, IGNORE);
}

void mutex_lock_init(mutex_lock_t *lock)
{
    invoke_syscall(SYSCALL_MUTEX_LOCK_INIT, (int)lock, IGNORE, IGNORE);
}

void mutex_lock_acquire(mutex_lock_t *lock)
{
    invoke_syscall(SYSCALL_MUTEX_LOCK_ACQUIRE, (int)lock, IGNORE, IGNORE);
}

void mutex_lock_release(mutex_lock_t *lock)
{
    invoke_syscall(SYSCALL_MUTEX_LOCK_RELEASE, (int)lock, IGNORE, IGNORE);
}

void barrier_init(barrier_t *barrier, int goal)
{
    invoke_syscall(SYSCALL_BARRIER_INIT, (int)barrier, goal, IGNORE);
}

void barrier_wait(barrier_t *barrier)
{
    invoke_syscall(SYSCALL_BARRIER_WAIT, (int)barrier, IGNORE, IGNORE);
}

void semaphore_init(semaphore_t *s, int val)
{
    invoke_syscall(SYSCALL_SEMAPHORE_INIT, (int)s, val, IGNORE);
}

void semaphore_up(semaphore_t *s)
{
    invoke_syscall(SYSCALL_SEMAPHORE_UP, (int)s, IGNORE, IGNORE);
}

void semaphore_down(semaphore_t *s)
{
    invoke_syscall(SYSCALL_SEMAPHORE_DOWN, (int)s, IGNORE, IGNORE);
}

void condition_init(condition_t *condition)
{
    invoke_syscall(SYSCALL_CONDITION_INIT, (int)condition, IGNORE, IGNORE);
}

void condition_wait(mutex_lock_t *lock, condition_t *condition)
{
    invoke_syscall(SYSCALL_CONDITION_WAIT, (int)lock, (int)condition, IGNORE);
}

void condition_signal(condition_t *condition)
{
    invoke_syscall(SYSCALL_CONDITION_SIGNAL, (int)condition, IGNORE, IGNORE);
}

void condition_broadcast(condition_t *condition)
{
    invoke_syscall(SYSCALL_CONDITION_BROADCAST, (int)condition, IGNORE, IGNORE);
}

void sys_init_mac()
{
    invoke_syscall(SYSCALL_INIT_MAC, IGNORE, IGNORE, IGNORE);
}

void sys_net_send(uint32_t td, uint32_t td_phy)
{
    invoke_syscall(SYSCALL_NET_SEND, td, td_phy, IGNORE);
}

uint32_t sys_net_recv(uint32_t rd,uint32_t rd_phy,uint32_t daddr)
{
    invoke_syscall(SYSCALL_NET_RECV, rd, rd_phy, daddr);
}

void sys_wait_recv_package()
{
    invoke_syscall(SYSCALL_WAIT_RECV, IGNORE, IGNORE, IGNORE);
}

int sys_mkfs()
{
    invoke_syscall(SYSCALL_MKFS, IGNORE, IGNORE, IGNORE);
}

void sys_statfs()
{
    invoke_syscall(SYSCALL_STATFS, IGNORE, IGNORE, IGNORE);
}

int sys_cd(char *sname)
{
    invoke_syscall(SYSCALL_CD, (int)sname, IGNORE, IGNORE);
}

int sys_mkdir(char *sname)
{
    invoke_syscall(SYSCALL_MKDIR, (int)sname, IGNORE, IGNORE);
}

int sys_rmdir(char *sname)
{
    invoke_syscall(SYSCALL_RMDIR, (int)sname, IGNORE, IGNORE);
}

int sys_ls(char *sname)
{
    invoke_syscall(SYSCALL_LS, (int)sname, IGNORE, IGNORE);
}

int sys_touch(char *sname)
{
    invoke_syscall(SYSCALL_TOUCH, (int)sname, IGNORE, IGNORE);
}

int sys_cat(char *sname)
{
    invoke_syscall(SYSCALL_CAT, (int)sname, IGNORE, IGNORE);
}

int sys_fopen(char *name, int acess)
{
    invoke_syscall(SYSCALL_FOPEN, (int)name, (int)acess, IGNORE);
}

int sys_fread(int fd, char *buff, int size)
{
    invoke_syscall(SYSCALL_FREAD, fd, (int)buff, size);
}

int sys_fwrite(int fd, char *buff, int size)
{
    invoke_syscall(SYSCALL_FWRITE, fd, (int)buff, size);
}

void sys_close(int fd)
{
    invoke_syscall(SYSCALL_CLOSE, fd, IGNORE, IGNORE);
}

void sys_cfs()
{
    invoke_syscall(SYSCALL_CFS, IGNORE, IGNORE, IGNORE);
}

void sys_other()
{
    return ;
}