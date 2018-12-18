CC = mipsel-linux-gcc -g

all: clean createimage image asm # floppy

SRC_BOOT 	= ./arch/mips/boot/bootblock.S

SRC_ARCH	= ./arch/mips/kernel/entry.S ./arch/mips/kernel/syscall.S ./arch/mips/pmon/common.c 
SRC_DRIVER	= ./drivers/screen.c ./drivers/mac.c
SRC_INIT 	= ./init/main.c
SRC_INT		= ./kernel/irq/irq.c # ./kernel/irq/shell.c
SRC_LOCK	= ./kernel/locking/barrier.c ./kernel/locking/cond.c ./kernel/locking/sem.c ./kernel/locking/lock.c
#SRC_MM      = ./kernel/mm/memory.c
SRC_SCHED	= ./kernel/sched/sched.c ./kernel/sched/queue.c ./kernel/sched/time.c
SRC_SYSCALL	= ./kernel/syscall/syscall.c
SRC_LIBS	= ./libs/mailbox.c ./libs/string.c ./libs/printk.c

SRC_TEST	= ./test/test.c ./test/test_shell.c
SRC_TEST2	= ./test/test_project2/test_scheduler1.c ./test/test_project2/test_scheduler2.c ./test/test_project2/test_lock2.c ./test/test_project2/test_sleep.c ./test/test_project2/test_timer.c 
SRC_TEST3	= ./test/test_project3/test_kill.c ./test/test_project3/test_barrier.c ./test/test_project3/test_condition.c ./test/test_project3/test_semaphore.c ./test/test_project3/test_sanguo.c
#SRC_TEST4	= ./test/test_project4/process2.c ./test/test_project4/process1.c 
SRC_TEST5	= ./test/test_net/test_regs1.c

SRC_IMAGE	= ./tools/createimage.c

bootblock: $(SRC_BOOT)
	${CC} -G 0 -O2 -fno-pic -mno-abicalls -fno-builtin -nostdinc -mips3 -Ttext=0xffffffffa0800000 -N -o bootblock $(SRC_BOOT) -nostdlib -e main -Wl,-m -Wl,elf32ltsmip -T ld.script

main : $(SRC_ARCH) $(SRC_DRIVER) $(SRC_INIT) $(SRC_INT) $(SRC_LOCK) $(SRC_SCHED) $(SRC_SYSCALL) $(SRC_LIBS) $(SRC_TEST) $(SRC_TEST2) $(SRC_TEST3) $(SRC_TEST5)
	${CC} -G 0 -O0 -Iinclude -Ilibs  -Idev -Idev/mac -Iarch/mips/include -Idrivers -Iinclude/os -Iinclude/sys -Itest -Itest/test_project2 -Itest/test_project3 -Itest/test_net \
	-fno-pic -mno-abicalls -fno-builtin -nostdinc -mips3 -Ttext=0xffffffffa0800200 -N -o main \
	$(SRC_ARCH) $(SRC_DRIVER) $(SRC_INIT) $(SRC_INT) $(SRC_LOCK) $(SRC_SCHED) $(SRC_SYSCALL) $(SRC_PROC) $(SRC_LIBS) $(SRC_TEST) $(SRC_TEST2) $(SRC_TEST3)  $(SRC_TEST5) -nostdlib -Wl,-m -Wl,elf32ltsmip -T ld.script		

createimage: $(SRC_IMAGE)
	gcc $(SRC_IMAGE) -o createimage

image: bootblock main
	./createimage --extended bootblock main 

clean:
	rm -rf bootblock image createimage main *.o

floppy:
	sudo fdisk -l /dev/sdb
	sudo dd if=image of=/dev/sdb conv=notrunc
#	sudo dd if=image of=disk conv=notrunc

asm:
	mipsel-linux-objdump -d main > kernel.txt
