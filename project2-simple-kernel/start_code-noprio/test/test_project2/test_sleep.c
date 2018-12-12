#include "test2.h"
#include "sched.h"
#include "stdio.h"
#include "syscall.h"

static char blank[] = {"                                                "};

void sleep_task(void)
{
    int i, j = 0;
    int print_location = 5;
    int sleep_time = 5;

    while (1)
    {
        j++;
        for (i = 0; i < 20; i++)
        {
            sys_move_cursor(1, print_location);
            printf("> [TASK] This task is to test sleep(). (%d)\n", i);
        }

        sys_move_cursor(1, print_location);
        printf("> [TASK] This task is sleeping, sleep time is %d.(%d)\n", sleep_time, j);

        /* call syscall sleep() */
        sys_sleep(sleep_time);

        sys_move_cursor(1, print_location);
        printf("%s", blank);
    }
}