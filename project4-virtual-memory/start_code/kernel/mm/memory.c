#include "mm.h"
#include "sched.h"

uint32_t sd_paddr = 0x1000;//0x20000000;

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
}
int pos = 0;
void do_TLB_Refill()
{
    uint32_t entryhi = get_cp0_entryhi();
    uint32_t context = get_cp0_context();
    context = context << 9;
    set_cp0_entryhi(context | entryhi & 0xff);
    asm volatile("tlbp");
    uint32_t index = get_cp0_index();
    uint32_t entrylo0, entrylo1;
    //vt100_move_cursor(0, pos++);
    //printk("%x %x %x",get_cp0_cause(), index, context);
    if(index & 0x80000000)
    {
        //TLB refill
        set_cp0_entryhi(entryhi);
        entrylo0 = (current_running->pgtab[context >> 12].ptentry & ~PTE_C | PTE_C) >> 6;
        entrylo1 = (current_running->pgtab[context >> 12 | 1].ptentry & ~PTE_C | PTE_C) >> 6;
        //printk("##%x %x %x",entryhi, entrylo0, entrylo1);
        set_cp0_entrylo0(entrylo0);
        set_cp0_entrylo1(entrylo1);
        asm volatile("tlbwr");
    }
    else
    {
        //TLB invalid
        if(!(current_running->pgtab[context >> 12].ptentry & PTE_V))
        {
            if(freelist.head != NULL)
            {
                pgframe_t *frame = palloc();
                current_running->pgtab[context >> 12].ptentry = frame->paddr & 0xfffff000 | PTE_C | PTE_D | PTE_V;
                frame->vpte = &current_running->pgtab[context >> 12];
                frame->vaddr = context | current_running->pid;
                frame = palloc();
                current_running->pgtab[context >> 12 | 1].ptentry = frame->paddr & 0xfffff000 | PTE_C | PTE_D | PTE_V;
                frame->vpte = &current_running->pgtab[context >> 12 | 1];
                frame->vaddr = (context | 1 << 12) | current_running->pid;
                
                if(current_running->pgtab[context >> 12].sd_sect != 0)
                    do_page_swap();
            }
            else
                do_page_swap();
        }
        /*if(!(current_running->pgtab[context >> 12 | 1].ptentry & PTE_V))
        {
            if(freelist.head != NULL)
            {
                
            }
            else
                do_page_swap();
        }*/
        entrylo0 = current_running->pgtab[context >> 12].ptentry >> 6;
        entrylo1 = current_running->pgtab[context >> 12 | 1].ptentry >> 6;
        //printk("**%x %x %x",get_cp0_entryhi(), entrylo0, entrylo1);
        set_cp0_entrylo0(entrylo0);
        set_cp0_entrylo1(entrylo1);
        asm volatile("tlbwi");
    }
}

void stack_Refill(uint32_t vpn2, int asid, pcb_t *curpcb)
{
    uint32_t entryhi = vpn2 << 13 | asid;
    uint32_t context = vpn2 << 13;
    set_cp0_entryhi(context | entryhi & 0xff);
    asm volatile("tlbp");
    uint32_t index = get_cp0_index();
    uint32_t entrylo0, entrylo1;

    if(freelist.head != NULL)
    {
        pgframe_t *frame = palloc();
        curpcb->pgtab[context >> 12].ptentry = frame->paddr & 0xfffff000 | PTE_C | PTE_D | PTE_V;
        frame->vpte = &curpcb->pgtab[context >> 12];
        frame->vaddr = context | current_running->pid;
        frame = palloc();
        curpcb->pgtab[context >> 12 | 1].ptentry = frame->paddr & 0xfffff000 | PTE_C | PTE_D | PTE_V;
        frame->vpte = &curpcb->pgtab[context >> 12 | 1];
        frame->vaddr = (context | 1 << 12) | current_running->pid;
        
        if(curpcb->pgtab[context >> 12].sd_sect != 0)
        {
            do_page_swap();
        }
    }
    else
    {
        do_page_swap();
    }
    /*if(freelist.head != NULL)
    {
        pgframe_t *frame = palloc();
        curpcb->pgtab[context >> 12 | 1].ptentry = frame->paddr & 0xfffff000 | PTE_C | PTE_D | PTE_V;
        frame->vpte = &curpcb->pgtab[context >> 12 | 1];
    }
    else
        do_page_swap();*/

    //vt100_move_cursor(0, pos++);
    //printk("%x %x %x",get_cp0_cause(), index, context);
    if(index & 0x80000000)
    {
        //TLB refill
        set_cp0_entryhi(entryhi);
        entrylo0 = curpcb->pgtab[context >> 12].ptentry >> 6;
        entrylo1 = curpcb->pgtab[context >> 12 | 1].ptentry >> 6;
        //printk("##%x %x %x",entryhi, entrylo0, entrylo1);
        set_cp0_entrylo0(entrylo0);
        set_cp0_entrylo1(entrylo1);
        asm volatile("tlbwr");
    }
    else
    {
        //TLB invalid
        entrylo0 = curpcb->pgtab[context >> 12].ptentry >> 6;
        entrylo1 = curpcb->pgtab[context >> 12 | 1].ptentry >> 6;
        //printk("**%x %x %x",get_cp0_entryhi(), entrylo0, entrylo1);
        set_cp0_entrylo0(entrylo0);
        set_cp0_entrylo1(entrylo1);
        asm volatile("tlbwi");
    }
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
    pgframe_t * frame = (pgframe_t *)busylist.head;
    
    busylist.head = (void *)frame->next;
    ((pgframe_t *)(busylist.head))->prev = NULL;

    ((pgframe_t *)busylist.tail)->next = frame;
    frame->next = NULL;
    frame->prev = (pgframe_t *)busylist.tail;
    busylist.tail = (void *)frame;
    
    return frame;
}

void do_page_swap()
{
    //vt100_move_cursor(0, 0);
    //printk("##Page Swap!!!???\n");
    pgframe_t *frame0, *frame1;
    uint32_t context = get_cp0_context() << 9;

    if(!(current_running->pgtab[context >> 12].ptentry & PTE_V))
    {
        frame0 = fifo_swap();
        frame1 = fifo_swap();

        pte_t *vpentry0 = frame0->vpte;
        pte_t *vpentry1 = frame1->vpte;
        
        /*if((frame0->vaddr & 0xfff) != current_running->pid)
        {
            uint32_t entryhi = get_cp0_entryhi();
            entryhi = entryhi & 0xffffff00 | frame0->vaddr & 0xfff;
            set_cp0_entryhi(entryhi);
        }*/

        if(vpentry0->ptentry & PTE_D)
        {
            if(vpentry0->sd_sect == 0)
            {vt100_move_cursor(0, 8);
                //printk("Pre Write!");
                //printk(" %x %x\n", frame0->vaddr, current_running->pid);
                
                sdwrite((uint8_t *)(frame0->vaddr & 0xfffff000), sd_paddr, PGSIZE);
                vpentry0->sd_sect = sd_paddr;
                sd_paddr += PGSIZE;
                //printk("Write Finish!\n");
            }
            else
                sdwrite((uint8_t *)(frame0->vaddr & 0xfffff000), vpentry0->sd_sect, PGSIZE);
        }
        vpentry0->ptentry = 0;

        if(vpentry1->ptentry & PTE_D)
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
        vpentry1->ptentry = 0;
        
        /*uint32_t entryhi = get_cp0_entryhi();
        entryhi = entryhi & 0xffffff00 | current_running->pid;
        set_cp0_entryhi(entryhi);
*/
        current_running->pgtab[context >> 12].ptentry = frame0->paddr & 0xfffff000 | PTE_C | PTE_D | PTE_V;
        frame0->vpte = &current_running->pgtab[context >> 12];
        frame0->vaddr = context | current_running->pid;
        current_running->pgtab[context >> 12 | 1].ptentry = frame1->paddr & 0xfffff000 | PTE_C | PTE_D | PTE_V;
        frame1->vpte = &current_running->pgtab[context >> 12 | 1];
        frame1->vaddr = (context | 1 << 12) | current_running->pid;
    }

    if(current_running->pgtab[context >> 12].sd_sect != 0)
    {
        sdread((uint8_t *)context, current_running->pgtab[context >> 12].sd_sect, PGSIZE);
        sdread((uint8_t *)(context | 1 << 12), current_running->pgtab[context>>12|1].sd_sect, PGSIZE);
    }

}