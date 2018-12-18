#include "sched.h"
#include "stdio.h"
#include "syscall.h"
#include "time.h"
#include "screen.h"
#include "test4.h"
#define RW_TIMES 2
int rand()
{	
	int current_time = get_timer();
	return current_time % 100000;
}

static void disable_interrupt()
{
    uint32_t cp0_status = get_cp0_status();
    cp0_status &= 0xfffffffe;
    set_cp0_status(cp0_status);
}

static void Enable_interrupt()
{
    uint32_t cp0_status = get_cp0_status();
    cp0_status |= 0x01;
    set_cp0_status(cp0_status);
}

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

static void scanf(int *mem)
{
	//TODO:Use read_uart_ch() to complete scanf(), read input as a hex number.
	//Extending function parameters to (const char *fmt, ...) as printf is recommended but not required.
	char num[10];
	int i = 0;
	sys_move_cursor(0, 0);
	printf("                                          ");
	sys_move_cursor(0, 0);
	printf("Input: ");
	disable_interrupt();
	screen_reflush();
	while(1)
	{
        char ch = read_uart_ch();    

        // TODO solve command
        if(ch == 0 || (i == 0 && (ch == 0x7f || ch == 8)))
            continue;
		my_printk("%c", ch);
		screen_reflush();
		if(ch != '\r')
		{
			if(ch == 8 || ch == 0x7f) // backspace
                i--;
            else 
                num[i++] = ch;
            continue;
		}
		else
		{
			num[i] = '\0';
			break;
		}
	}
	Enable_interrupt();
	*mem = itoa((char *)num, 16);
}

void rw_task1(void)
{
	int mem1, mem2 = 0;
	int curs = 1;
	int memory[RW_TIMES];
	int i = 0;
	for(i = 0; i < RW_TIMES; i++)
	{
		//vt100_move_cursor(1, curs+i);
		scanf(&mem1);
		memory[i] = mem2 = rand();
		*(int *)mem1 = mem2;
		sys_move_cursor(1, curs+i);
		printf("Write: 0x%x, %d", mem1, mem2);
	}
	curs = RW_TIMES + 1;
	for(i = 0; i < RW_TIMES; i++)
	{
		//vt100_move_cursor(1, curs+i);
		scanf(&mem1);
		memory[i+RW_TIMES] = *(int *)mem1;
		sys_move_cursor(1, curs+i);
		if(memory[i+RW_TIMES] == memory[i])
			printf("Read succeed: %d", memory[i+RW_TIMES]);
		else
			printf("Read error: %d", memory[i+RW_TIMES]);
	}
	while(1);
}
