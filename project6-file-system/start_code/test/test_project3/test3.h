#ifndef INCLUDE_TASK3_H_
#define INCLUDE_TASK3_H_

#include "type.h"
#include "cond.h"

// [1-4]
void ready_to_exit_task(void);
void wait_lock_task();
void wait_exit_task();

// [8-10]
void producer_task(void);
void consumer_task1(void);
void consumer_task2(void);

extern mutex_lock_t mutex;
extern condition_t condition;

// [8-10]
void semaphore_add_task1(void);
void semaphore_add_task2(void);
void semaphore_add_task3(void);

// [8 -10]
void barrier_task1(void);
void barrier_task2(void);
void barrier_task3(void);

// [11-14]
void mbox_task1(void);
void mbox_task2(void);
void mbox_task3(void);
void mbox_task4(void);

void SunQuan(void);
void LiuBei(void);
void CaoCao(void);

#endif
