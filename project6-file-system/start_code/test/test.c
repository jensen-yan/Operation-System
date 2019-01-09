#include "test.h"

/* task group to test do_scheduler() */
struct task_info task2_1 = {"task1", (uint32_t)&printk_task1, USER_PROCESS};
struct task_info task2_2 = {"task1", (uint32_t)&printk_task2, USER_PROCESS};
struct task_info task2_3 = {"task1", (uint32_t)&drawing_task1, USER_PROCESS};
struct task_info *sched1_tasks[16] = {&task2_1, &task2_2, &task2_2};
int num_sched1_tasks = 3;

/* task group to test lock */
struct task_info task2_4 = {"task4", (uint32_t)&lock_task1, USER_PROCESS};
struct task_info task2_5 = {"task5", (uint32_t)&lock_task2, USER_PROCESS};
struct task_info *lock_tasks[16] = {&task2_4, &task2_5};
int num_lock_tasks = 2;

/* task group to test clock scheduler */
struct task_info task2_6 = {"task6", (uint32_t)&sleep_task, USER_PROCESS};
struct task_info task2_7 = {"task7", (uint32_t)&timer_task, USER_PROCESS};
struct task_info *timer_tasks[16] = {&task2_6, &task2_7};
int num_timer_tasks = 2;

/* task group to test clock scheduler */
struct task_info task2_8 = {"task8", (uint32_t)&printf_task1, USER_PROCESS};
struct task_info task2_9 = {"task9", (uint32_t)&printf_task2, USER_PROCESS};
struct task_info task2_10 = {"task10", (uint32_t)&drawing_task2, USER_PROCESS};
struct task_info *sched2_tasks[16] = {&task2_8, &task2_9, &task2_10};
int num_sched2_tasks = 3;

// use shell to test kill a process
struct task_info shell_task1 = {"task1", (uint32_t)&test_shell, USER_PROCESS};
struct task_info *shell_tasks[16] = {&shell_task1};
int num_shell_tasks = 1;

