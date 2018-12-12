#include "lock.h"
#include "sched.h"
#include "syscall.h"
#include "screen.h"

int lock_queue_id = 0;

static void disable_interrupt()
{
    uint32_t cp0_status = get_cp0_status();
    cp0_status &= 0xfffffffe;
    set_cp0_status(cp0_status);
}

static void enable_interrupt()
{
    uint32_t cp0_status = get_cp0_status();
    cp0_status |= 0x01;
    set_cp0_status(cp0_status);
}

void spin_lock_init(spin_lock_t *lock)
{
    lock->status = UNLOCKED;
}

void spin_lock_acquire(spin_lock_t *lock)
{
    while (LOCKED == lock->status)
    {
    };
    lock->status = LOCKED;
}

void spin_lock_release(spin_lock_t *lock)
{
    lock->status = UNLOCKED;
}

void do_mutex_lock_init(mutex_lock_t *lock)
{
    lock->status = UNLOCKED;
    queue_init(&lock->block_queue);
}

void do_mutex_lock_acquire(mutex_lock_t *lock)
{
    while(lock->status == LOCKED)
    {
        do_block(&lock->block_queue);
    }
    lock->status = LOCKED;
    current_running->lock[++current_running->lock_top] = lock;
}

void do_mutex_lock_release(mutex_lock_t *lock)
{
    int i;
    for(i = 0; i <= current_running->lock_top; i++)
        if(current_running->lock[i] == lock)
        {
            int j = i+1;
            while(j <= current_running->lock_top)
            {
                current_running->lock[j-1] = current_running->lock[j];
                j++;
            }
            --current_running->lock_top;
            break;
        }
    
    if(!queue_is_empty(&lock->block_queue))
    {
        do_unblock_one(&lock->block_queue);
    }
    lock->status = UNLOCKED;
}
