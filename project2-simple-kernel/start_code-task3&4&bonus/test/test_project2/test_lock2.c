#include "test2.h"
#include "lock.h"
#include "stdio.h"
#include "syscall.h"

int is_init = FALSE;
static char blank[] = {"                                             "};

/* if you want to use spin lock, you need define SPIN_LOCK */
//  #define SPIN_LOCK
spin_lock_t spin_lock;

/* if you want to use mutex lock, you need define MUTEX_LOCK */
#define MUTEX_LOCK
mutex_lock_t mutex_lock_o, mutex_lock_s;

void lock_task1(void)
{
        int print_location = 3, j = 1;
        while (1)
        {
                j++;
                int i;
                if (!is_init)
                {
                        is_init = TRUE;
#ifdef SPIN_LOCK
                        spin_lock_init(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                        mutex_lock_init(&mutex_lock_o);
                        mutex_lock_init(&mutex_lock_s);
#endif
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK1] Applying for a lock1.(%d)\n", j);

                //do_scheduler();

#ifdef SPIN_LOCK
                spin_lock_acquire(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                mutex_lock_acquire(&mutex_lock_o);
#endif

                for (i = 0; i < 30; i++)
                {
                        sys_move_cursor(1, print_location);
                        printf("> [TASK1] Has acquired lock1 and running.(%d)\n", i);
                        //do_scheduler();
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK1] Applying for a lock2.(%d)\n", j);

                //do_scheduler();

#ifdef SPIN_LOCK
                spin_lock_acquire(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                mutex_lock_acquire(&mutex_lock_s);
#endif

                for (i = 0; i < 100; i++)
                {
                        sys_move_cursor(1, print_location);
                        printf("> [TASK1] Has acquired lock2 and running.(%d)\n", i);
                        //do_scheduler();
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK1] Has acquired lock2 and exited.(%d)\n", j);

#ifdef SPIN_LOCK
                spin_lock_release(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                mutex_lock_release(&mutex_lock_s);
#endif
                //do_scheduler();                

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK1] Has acquired lock1 and exited.(%d)\n", j);

#ifdef SPIN_LOCK
                spin_lock_release(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                mutex_lock_release(&mutex_lock_o);
#endif
                //do_scheduler();
        }
}

void lock_task2(void)
{
        int print_location = 4, j = 1;
        while (1)
        {
                j++;
                int i;
                if (!is_init)
                {
                        is_init = TRUE;
#ifdef SPIN_LOCK
                        spin_lock_init(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                        mutex_lock_init(&mutex_lock_o);
                        mutex_lock_init(&mutex_lock_s);
#endif
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK2] Applying for a lock1.(%d)\n", j);

                //do_scheduler();

#ifdef SPIN_LOCK
                spin_lock_acquire(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                mutex_lock_acquire(&mutex_lock_o);
#endif

                for (i = 0; i < 20; i++)
                {
                        sys_move_cursor(1, print_location);
                        printf("> [TASK2] Has acquired lock1 and running.(%d)\n", i);
                        //do_scheduler();
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK2] Has acquired lock1 and exited.(%d)\n", j);

#ifdef SPIN_LOCK
                spin_lock_release(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                mutex_lock_release(&mutex_lock_o);
#endif
                //do_scheduler();

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK2] Applying for a lock2.(%d)\n", j);

                //do_scheduler();

#ifdef SPIN_LOCK
                spin_lock_acquire(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                mutex_lock_acquire(&mutex_lock_s);
#endif

                for (i = 0; i < 20; i++)
                {
                        sys_move_cursor(1, print_location);
                        printf("> [TASK2] Has acquired lock2 and running.(%d)\n", i);
                        //do_scheduler();
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK2] Has acquired lock2 and exited.(%d)\n", j);

#ifdef SPIN_LOCK
                spin_lock_release(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                mutex_lock_release(&mutex_lock_s);
#endif
                //do_scheduler();
        }
}

void lock_task3(void)
{
        int print_location = 5, j = 1;
        while (1)
        {
                j++;
                int i;
                if (!is_init)
                {
                        is_init = TRUE;
#ifdef SPIN_LOCK
                        spin_lock_init(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                        mutex_lock_init(&mutex_lock_o);
                        mutex_lock_init(&mutex_lock_s);
#endif
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK3] Applying for a lock1.(%d)\n", j);

                //do_scheduler();

#ifdef SPIN_LOCK
                spin_lock_acquire(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                mutex_lock_acquire(&mutex_lock_o);
#endif

                for (i = 0; i < 20; i++)
                {
                        sys_move_cursor(1, print_location);
                        printf("> [TASK3] Has acquired lock1 and running.(%d)\n", i);
                        //do_scheduler();
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK3] Has acquired lock1 and exited.(%d)\n", j);

#ifdef SPIN_LOCK
                spin_lock_release(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                mutex_lock_release(&mutex_lock_o);
#endif
                //do_scheduler();
        }
}