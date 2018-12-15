/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                  The shell acts as a task running in user mode. 
 *       The main function is to make system calls through the user's output.
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

#include "test.h"
#include "stdio.h"
#include "screen.h"
#include "syscall.h"

// pid = 2
// test exit
void test_shell_task1()
{
    int i;

    for (i = 0; i < 500; i++)
    {
        sys_move_cursor(0, 0);
        printf("I am Task A.(%d)           \n", i);
    }

    sys_exit();
}

// pid = 3
// test kill & waitpid
void test_shell_task2()
{
    int i;

    sys_move_cursor(0, 1);
    printf("I am waiting Task A to exit.\n");
    sys_waitpid(2);

    for (i = 0;; i++)
    {
        sys_move_cursor(0, 1);
        printf("I am Task B.(%d)           \n", i);
    }
}

// pid = 4
// test waitpid
void test_shell_task3()
{
    int i;

    sys_move_cursor(0, 2);
    printf("I am waiting Task B to exit.\n");
    sys_waitpid(3);

    for (i = 0;; i++)
    {
        sys_move_cursor(0, 2);
        printf("I am Task C.(%d)           \n", i);
    }
}

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

#define SHLLE_BUFF_SIZE 64

static int shell_tail = 0;
static char shell_buff[SHLLE_BUFF_SIZE];

static char read_uart_ch(void)
{
    char ch = 0;
    unsigned char *read_port = (unsigned char *)(0xbfe48000 + 0x00);
    unsigned char *stat_port = (unsigned char *)(0xbfe48000 + 0x05);

    while ((*stat_port & 0x01))
    {
        ch = *read_port;
    }
    return ch;
}

struct task_info task1 = {"task1", (uint32_t)&ready_to_exit_task, USER_PROCESS};
struct task_info task2 = {"task2", (uint32_t)&wait_lock_task, USER_PROCESS};
struct task_info task3 = {"task3", (uint32_t)&wait_exit_task, USER_PROCESS};

struct task_info task4 = {"task4", (uint32_t)&semaphore_add_task1, USER_PROCESS};
struct task_info task5 = {"task5", (uint32_t)&semaphore_add_task2, USER_PROCESS};
struct task_info task6 = {"task6", (uint32_t)&semaphore_add_task3, USER_PROCESS};

struct task_info task7 = {"task7", (uint32_t)&producer_task, USER_PROCESS};
struct task_info task8 = {"task8", (uint32_t)&consumer_task1, USER_PROCESS};
struct task_info task9 = {"task9", (uint32_t)&consumer_task2, USER_PROCESS};

struct task_info task10 = {"task10", (uint32_t)&barrier_task1, USER_PROCESS};
struct task_info task11 = {"task11", (uint32_t)&barrier_task2, USER_PROCESS};
struct task_info task12 = {"task12", (uint32_t)&barrier_task3, USER_PROCESS};

struct task_info task13 = {"SunQuan",(uint32_t)&SunQuan, USER_PROCESS};
struct task_info task14 = {"LiuBei", (uint32_t)&LiuBei, USER_PROCESS};
struct task_info task15 = {"CaoCao", (uint32_t)&CaoCao, USER_PROCESS};
#if 1
struct task_info task4_1 = {"send",(uint32_t)&phy_regs_task1, USER_PROCESS};
struct task_info task4_2 = {"recv",(uint32_t)&phy_regs_task2, USER_PROCESS};
struct task_info task4_3 = {"initmac",(uint32_t)&phy_regs_task3, USER_PROCESS};


static struct task_info *test_tasks[19] = {&task4_3,&task4_1,&task4_2};
static int num_test_tasks = 3;
#endif




void test_shell()
{
    char ch;
    char name[32];
    int i, j, l, k, pid, command;

    sys_move_cursor(0, SCREEN_HEIGHT / 2);
    printf("------------------- COMMAND -------------------\n");
    printf("> root@UCAS_OS: ");

    while (1)
    {
        command = 0;

        disable_interrupt();

        // read UART port
        ch = read_uart_ch();

        enable_interrupt();

        if (ch == 8) // Backspace
        {
            if (shell_tail > 0)
            {
                shell_tail--;
            }
            printf("%c", ch);
        }
        else if (ch == 13) // Enter
        {
            command = 1;
            printf("\n");
        }
        else if (ch != 0)
        {
            shell_buff[shell_tail++] = ch;
            printf("%c", ch);
        }

        if (command == 1)
        {
            shell_buff[shell_tail] = '\0';
            l = strlen(shell_buff);

            // process show
            if (memcmp(shell_buff, "ps", 2) == 0)
            {
                sys_process_show();
            }
            // spawn a new task
            else if (memcmp(shell_buff, "exec", 4) == 0)
            {
                pid = 0;
                l = strlen(shell_buff);

                for (i = 4; i < l; i++)
                {
                    if (shell_buff[i] != ' ')
                    {
                        break;
                    }
                }

                while ((i < l) && (shell_buff[i] != ' '))
                {
                    pid = 10 * pid + (shell_buff[i] - '0');
                    i++;
                }

                if (pid >= 0 && pid < num_test_tasks)
                {
                    printf("exec process[%d].\n", pid);
                    sys_spawn(test_tasks[pid]);
                }
                else
                {
                    printf("error process index!\n");
                } 
            }
            // kill process
            else if (memcmp(shell_buff, "kill", 4) == 0)
            {
                pid = 0;
                l = strlen(shell_buff);

                for (i = 4; i < l; i++)
                {
                    if (shell_buff[i] != ' ')
                    {
                        break;
                    }
                }

                while ((i < l) && (shell_buff[i] != ' '))
                {
                    pid = 10 * pid + (shell_buff[i] - '0');
                    i++;
                }

                if (pid != 0)
                {
                    printf("kill process pid=%d\n", pid);
                    sys_kill(pid);
                }
            }
            // clear the screen
            else if (memcmp(shell_buff, "clear", 5) == 0)
            {
                sys_screen_clear(0, SCREEN_HEIGHT - 1);
                sys_move_cursor(0, SCREEN_HEIGHT / 2);
                printf("------------------- COMMAND -------------------\n");
            }

            printf("> root@UCAS_OS: ");
            shell_tail = 0;
        }
    }
}