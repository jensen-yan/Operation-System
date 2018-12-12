# Project2-A Simple Kernel
Author    : 段江飞  
Student ID: 2016K8009929011
## task1 & task2
task1 和 task2 的源码在 startcode-task1&2  

任务不包括优先级测试

## task3 & task4 & bonus
task3 和 task4 和 bonus 的源码在 startcode-task1&2&bonus    

若要用于测试 task1 和 task2, 需要将initpcb中的31号寄存器初始化为函数入口地址, 关闭中断, 修改测试任务  

测试任务 test_lock2.c 是 bonus 的测试样例, 三个任务抢两把锁, 具体任务设计见设计文档  

单独测试只需要注释掉initpcb的完整for循环即可  

测试任务中包含几个带--或++的测试文件, scheduler的是用于测试中断, lock的是bonus的设计和正常的测试任务
