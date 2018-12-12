#include "irq.h"
#include "time.h"
#include "sched.h"
#include "string.h"
#include "screen.h"

int location = -2;
static void irq_timer()
{
    // TODO clock interrupt handler.
    // scheduler, time counter in here to do, emmmmmm maybe.
    //screen_reflush();
    time_elapsed += 10000;//TIMER_INTERVAL;
    current_running->cursor_x = screen_cursor_x;
    current_running->cursor_y = screen_cursor_y;
    //int x1 = GET_CP0_COMPARE();
    //int x2 = GET_CP0_COUNT();
    //printk("pid: %d %d %d\n", current_running->pid, x1, x2);
    do_scheduler();
    screen_cursor_x = current_running->cursor_x;
    screen_cursor_y = current_running->cursor_y;
    SET_CP0_COUNT(0);
    SET_CP0_COMPARE(500000);
    //x1 = GET_CP0_COMPARE();
    //x2 = GET_CP0_COUNT();
    //printk("pid: %d %d %d\n", current_running->pid, x1, x2);
    //vt100_move_cursor(1, 3);
    //printk("%d %d", );
    return ;
}

void other_exception_handler()
{
    // TODO other exception handler
    time_elapsed += 10000;//TIMER_INTERVAL;
    return ;
}

void interrupt_helper(uint32_t status, uint32_t cause)
{
    // TODO interrupt handler.
    // Leve3 exception Handler.
    // read CP0 register to analyze the type of interrupt.
    uint32_t interrupt = status & cause & 0x0000ff00;
    if(interrupt & 0x00008000)
        irq_timer();
    else
        other_exception_handler();
    return ;
}
