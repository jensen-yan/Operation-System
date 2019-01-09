#include "mm.h"
#include "sched.h"
#include "irq.h"
#include "sem.h"

uint32_t sd_paddr = 0x10000000;

//TODO:Finish memory management functions here refer to mm.h and add any functions you need.
pgframe_t *palloc()
{
    pgframe_t *item = (pgframe_t *)freelist.head;
    
    if(freelist.head == freelist.tail)
        freelist.head = freelist.tail = NULL;
    else
    {
        freelist.head = (void *)item->next;
        item->next->prev = NULL;
    }

    if(busylist.head == NULL)
    {
        busylist.head = (void *)item;
        busylist.tail = (void *)item;
        item->next = item->prev = NULL;
    }
    else
    {
        ((pgframe_t *)busylist.tail)->next = item;
        item->next = NULL;
        item->prev = (pgframe_t *)busylist.tail;
        busylist.tail = (void *)item;
    }

    return item;
}

void pfree(pgframe_t *item)
{
    if(item->prev != NULL && item->next != NULL)
    {
        void *_item = (void *)item;
        if (_item == busylist.head && _item == busylist.tail)
        {
            busylist.head = NULL;
            busylist.tail = NULL;
        }
        else if (_item == busylist.head)
        {
            busylist.head = (void *)item->next;
            ((pgframe_t *)(busylist.head))->prev = NULL;
        }
        else if (_item == busylist.tail)
        {
            busylist.tail = (void *)item->prev;
            ((pgframe_t *)(busylist.tail))->next = NULL;
        }
        else
        {
            (item->prev)->next = item->next;
            (item->next)->prev = item->prev;
        }
    }

    if(freelist.head == NULL)
    {
        freelist.head = (void *)item;
        freelist.tail = (void *)item;
        item->next = item->prev = NULL;
    }
    else
    {
        ((pgframe_t *)freelist.tail)->next = item;
        item->next = NULL;
        item->prev = (pgframe_t *)freelist.tail;
        freelist.tail = (void *)item;
    }
}
semaphore_t sem;
void freerange(uint32_t pstart, uint32_t pend)
{
    uint32_t p, i;
    p = PGROUNDUP(pstart);
    for(i = 0; p + PGSIZE <= pend; p += PGSIZE, i++)
    {
        pmem[i].paddr = p;
        pmem[i].vpte = NULL;
        pmem[i].prev = pmem[i].next  = NULL;
        pfree(&pmem[i]);
    }
    do_semaphore_init(&sem, 0);
}
int to = 0;

void spawn_swap(pcb_t *curpcb)
{
    pcb_t *new_pcb;
    if(queue_is_empty(&exit_pcb_queue))
        new_pcb = &pcb[cur_pcb_id++];
    else
        new_pcb = queue_dequeue(&exit_pcb_queue);
    //priority: '->' > '&' 
    bzero(&new_pcb->kernel_context, sizeof(new_pcb->kernel_context));
	bzero(&new_pcb->user_context, sizeof(new_pcb->user_context));

    new_pcb->kernel_context.regs[4] = new_pcb->user_context.regs[4] = (uint32_t)curpcb;
    new_pcb->kernel_context.regs[5] = new_pcb->user_context.regs[5] = (uint32_t)(get_cp0_context() << 9);

	// stack 
	new_pcb->kernel_stack_top = new_pcb->kernel_context.regs[29] = new_stack();
    new_pcb->user_stack_top = new_pcb->user_context.regs[29] = new_stack();

	new_pcb->kernel_context.regs[31] = (uint32_t)&first_run_handle;
	new_pcb->kernel_context.cp0_status = 0x10008003;
	new_pcb->kernel_context.cp0_epc = (uint32_t)&do_page_swap;

	new_pcb->user_context.regs[31] = (uint32_t)&do_page_swap;
	new_pcb->user_context.cp0_status = 0x10008003;
    new_pcb->user_context.cp0_epc = (uint32_t)&do_page_swap;

    new_pcb->lock_top = -1;
    queue_init(&new_pcb->wait_queue);
    new_pcb->pid = ((uint32_t)new_pcb - (uint32_t)pcb)/sizeof(pcb_t);
    new_pcb->kernel_context.hi = new_pcb->user_context.hi = curpcb->pid;
    new_pcb->my_queue = &ready_queue;

    new_pcb->type = KERNEL_PROCESS;
	new_pcb->status = TASK_READY;
	//new_pcb->task_priority = task->priority;
	//new_pcb->priority = task->priority;
	priority_queue_push(&ready_queue, (void *)new_pcb);
    do_wait(new_pcb->pid);
    //vt100_move_cursor(0, 12);
    //printk("RWATIE(%d)", to++);
}

int pos = 7;
void do_TLB_Refill()
{
    uint32_t entryhi = get_cp0_entryhi();
    uint32_t context = get_cp0_context() << 9;
    if((pgtab[context >> 12].ptentry & 0x1f) && (entryhi & 0xff != pgtab[context >> 12].asid))
    {
        my_printk("This Address is PROTECEED!\n");
        do_exit();
    }
    set_cp0_entryhi(context | current_running->pid);
    uint32_t entrylo0 = (pgtab[context >> 12].ptentry & ~PTE_C | PTE_C) >> 6;
    uint32_t entrylo1 = (pgtab[context >> 12 | 1].ptentry & ~PTE_C | PTE_C) >> 6;
    set_cp0_entrylo0(entrylo0);
    set_cp0_entrylo1(entrylo1);
    asm volatile("tlbwr");
}

//Refill user stack (initial)
void stack_Refill(uint32_t vpn2, int asid, pcb_t *curpcb)
{
    uint32_t entryhi = vpn2 << 13 | asid;
    uint32_t context = vpn2 << 13;
    set_cp0_entryhi(context | asid);
    uint32_t entrylo0 = curpcb->pgtab[context >> 12].ptentry >> 6;
    uint32_t entrylo1 = curpcb->pgtab[context >> 12 | 1].ptentry >> 6;
    set_cp0_entrylo0(entrylo0);
    set_cp0_entrylo1(entrylo1);
    asm volatile("tlbwr");
}

void do_TLB_Invalid()
{
    uint32_t context = get_cp0_context() << 9;
    if(!(current_running->pgtab[context >> 12].ptentry & PTE_V))
    {
        if(freelist.head != NULL)
        {
            pgframe_t *frame = palloc();
            pgtab[context >> 12].ptentry = frame->paddr & 0xfffff000 | PTE_C | PTE_D | PTE_V;
            frame->vpte = &pgtab[context >> 12];
            frame->vaddr = context | current_running->pid;
            frame = palloc();
            pgtab[context >> 12 | 1].ptentry = frame->paddr & 0xfffff000 | PTE_C | PTE_D | PTE_V;
            frame->vpte = &pgtab[context >> 12 | 1];
            frame->vaddr = (context | 1 << 12) | current_running->pid;
            pgtab[context >> 12].asid = pgtab[context >> 12 | 1].asid = current_running->pid;
                
            if(pgtab[context >> 12].sd_sect != 0)
            {
                spawn_swap(current_running);
            }
        }
        else
        {
            spawn_swap(current_running);
        }
    }

    set_cp0_entryhi(context | current_running->pid);
    asm volatile("tlbp");
    uint32_t entrylo0 = (pgtab[context >> 12].ptentry & ~PTE_C | PTE_C) >> 6;
    uint32_t entrylo1 = (pgtab[context >> 12 | 1].ptentry & ~PTE_C | PTE_C) >> 6;
    set_cp0_entrylo0(entrylo0);
    set_cp0_entrylo1(entrylo1);
    asm volatile("tlbwi");
}

void debug()
{
    vt100_move_cursor(0, pos);
    pos += 5;  
    pgframe_t *tmp = (pgframe_t *)freelist.head;
    while(tmp != NULL)
    {
        printk("%x ", tmp->paddr);
        tmp = tmp->next;
    }
    printk("\n");
    tmp = (pgframe_t *)busylist.head;
    while(tmp != NULL)
    {
        printk("%x ", tmp->paddr);
        tmp = tmp->next;
    }
    printk("\n");
}

//swap the first two page frames
pgframe_t *fifo_swap()
{
    pgframe_t *frame = (pgframe_t *)busylist.head;

    busylist.head = (void *)frame->next;
    ((pgframe_t *)(busylist.head))->prev = NULL;

    ((pgframe_t *)busylist.tail)->next = frame;
    frame->next = NULL;
    frame->prev = (pgframe_t *)busylist.tail;
    busylist.tail = (void *)frame;
    
    return frame;
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
void do_page_swap(pcb_t *npcb, uint32_t context)
{
    pgframe_t *frame0, *frame1;

    if(!(pgtab[context >> 12].ptentry & PTE_V))
    {
        frame0 = fifo_swap();
        frame1 = fifo_swap();

        pte_t *vpentry0 = frame0->vpte;
        pte_t *vpentry1 = frame1->vpte;

        //write
        uint32_t entryhi = frame0->vaddr & 0xffffe0ff;
        uint32_t entrylo0 = (frame0->paddr & 0xfffff000 | PTE_C | PTE_D | PTE_V) >> 6;
        uint32_t entrylo1 = (frame1->paddr & 0xfffff000 | PTE_C | PTE_D | PTE_V) >> 6;
        set_cp0_entryhi(entryhi);
        set_cp0_entrylo0(entrylo0);
        set_cp0_entrylo1(entrylo1);
        asm volatile("tlbp");
        uint32_t index = get_cp0_index();
        if(index & 0x80000000)
        {
            index = 31;
            set_cp0_index(index);
            asm volatile("tlbwi");
        }
        else
            asm volatile("tlbwi");

        if(vpentry0->ptentry & PTE_D)
        {
            if(vpentry0->sd_sect == 0)
            {
                sdwrite((uint8_t *)(frame0->vaddr & 0xfffff000), sd_paddr, 2*PGSIZE);
                vpentry0->sd_sect = sd_paddr;
                sd_paddr += PGSIZE;
                vpentry1->sd_sect = sd_paddr;
                sd_paddr += PGSIZE;
            }
            else
                sdwrite((uint8_t *)(frame0->vaddr & 0xfffff000), vpentry0->sd_sect, PGSIZE);
        }
        vpentry0->ptentry = vpentry1->ptentry = 0;

        /*if(vpentry1->ptentry & PTE_D)
        {
            if(vpentry1->sd_sect == 0)
            {
                sdwrite((uint8_t *)(frame1->vaddr & 0xfffff000), sd_paddr, PGSIZE);
                vpentry1->sd_sect = sd_paddr;
                sd_paddr += PGSIZE;
            }
            else
                sdwrite((uint8_t *)(frame1->vaddr & 0xfffff000), vpentry1->sd_sect, PGSIZE);
        }
        vpentry1->ptentry = 0;*/

        pgtab[context >> 12].ptentry = frame0->paddr & 0xfffff000 | PTE_C | PTE_D | PTE_V;
        frame0->vpte = &pgtab[context >> 12];
        frame0->vaddr = context | npcb->pid;
        pgtab[context >> 12 | 1].ptentry = frame1->paddr & 0xfffff000 | PTE_C | PTE_D | PTE_V;
        frame1->vpte = &pgtab[context >> 12 | 1];
        frame1->vaddr = (context | 1 << 12) | npcb->pid;
        pgtab[context >> 12].asid = pgtab[context >> 12 | 1].asid = npcb->pid;
    }
    uint32_t entryhi = 0;
    uint32_t entrylo0 = 0;
    uint32_t entrylo1 = 0;
    //set_cp0_entryhi(entryhi);
    set_cp0_entrylo0(entrylo0);
    set_cp0_entrylo1(entrylo1);
    asm volatile("tlbwi");

    if(pgtab[context >> 12].sd_sect != 0)
    {
        //disable_interrupt();
        entryhi = context | npcb->pid;
        set_cp0_entryhi(entryhi);
        entrylo0 = pgtab[context >> 12].ptentry >> 6;
        entrylo1 = pgtab[context >> 12 | 1].ptentry >> 6;
        set_cp0_entrylo0(entrylo0);
        set_cp0_entrylo1(entrylo1);
        //vt100_move_cursor(0, ++pos);
        //debug
        //sys_move_cursor(0, 6);
        //printf("**%x %x %x**\n", npcb->pid, context, get_cp0_entryhi());
        //printf("%x %x %x %x ", get_cp0_entryhi(), get_cp0_entrylo0(), get_cp0_entrylo1(), get_cp0_index());
        
        
        asm volatile("tlbp");
        //printf("%x\n", get_cp0_index());

        if(get_cp0_index() & 0x80000000)
            asm volatile("tlbwr");
        else
            asm volatile("tlbwi");
        disable_interrupt();
        sdread((uint8_t *)context, pgtab[context >> 12].sd_sect, 2*PGSIZE);
        enable_interrupt();
        //printf("EM:%x %x %x", (context | 1 << 12), pgtab[context>>12|1].ptentry, pgtab[context>>12|1].sd_sect);
        
        //sdread((uint8_t *)(context | 1 << 12), npcb->pgtab[context>>12|1].sd_sect, PGSIZE);
        //printf("Read Finish\n");
        //enable_interrupt();
    }

    /*set_cp0_entryhi(0x2);
    asm volatile("tlbp");
    if(get_cp0_index() & 0x80000000)
    {
        set_cp0_entrylo0(pcb[2].pgtab[0].ptentry >> 6);
        set_cp0_entrylo1(pcb[2].pgtab[1].ptentry >> 6);
        asm volatile("tlbwr");
        printk("WARNING!!!");
    }*/

    sys_exit();
}