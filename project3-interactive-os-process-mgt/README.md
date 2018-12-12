# Shell
## command  
1. `ps`  
展示进程，包括当前正在运行，就绪队列和睡眠队列中的任务
2. `exec` task_num  
执行任务号为task_num的任务，task_num从1开始
3. `kill` pid  
杀死进程号为pid对应的任务
4. clear  
清除命令行
5. clear all  
清除全屏幕

## 时间片
当前时间片设置为100000，修改可在entry.S中修改