#include "time.h"
#include "test3.h"
#include "lock.h"
#include "sched.h"
#include "stdio.h"
#include "syscall.h"

static char blank[] = {"                                                "};

static int is_init = FALSE;

mutex_lock_t lock1;
mutex_lock_t lock2;

// pid = 2
void ready_to_exit_task()
{
    int i = 0, print_location = 0;
    
    if (!is_init)
    {
        mutex_lock_init(&lock1);
        mutex_lock_init(&lock2);
        is_init = TRUE;
    }

    mutex_lock_acquire(&lock1);
    mutex_lock_acquire(&lock2);

     //sys_spawn(&task1);
     //sys_spawn(&task2);

    for (i = 0; i < 500; i++)
    {
        sys_move_cursor(0, print_location);
        printf("> [TASK] I am task with pid %d, I have acquired two mutex lock. (%d)", current_running->pid, i++);
    }
    sys_exit(); // test exit
}

// pid = 3
void wait_lock_task()
{
    int i, print_location = 1;

    if (!is_init)
    {
        mutex_lock_init(&lock1);
        mutex_lock_init(&lock2);
        is_init = TRUE;
    }

    sys_move_cursor(0, print_location);
    printf("> [TASK] I want to acquire a mutex lock from task(pid=2).");

    mutex_lock_acquire(&lock1);

    sys_move_cursor(0, print_location);
    printf("> [TASK] I have acquired a mutex lock from task(pid=2).");

    sys_exit(); // test exit
}

// pid = 4
void wait_exit_task()
{
    int i, print_location = 2;

    if (!is_init)
    {
        mutex_lock_init(&lock1);
        mutex_lock_init(&lock2);
        is_init = TRUE;
    }

    sys_move_cursor(0, print_location);
    printf("> [TASK] I want to wait task (pid=2) to exit.");

    sys_waitpid(2); //test waitpid

    sys_move_cursor(0, print_location);
    printf("> [TASK] Task (pid=2) has exited.                ");

    sys_exit(); // test exit
}