#ifndef INCLUDE_MM_H_
#define INCLUDE_MM_H_

#include "queue.h"
#include "type.h"

#define TLB_ENTRY_NUMBER 64

// A virtual address 'la' has a two-part structure as follows:
//
// +--------------20-----------------+---------12----------+
// |       Page Table Index          | Offset within Page  |
// |                                 |                     |
// +---------------------------------+---------------------+
//  \------      PTX(va)       -----/

#define NPGTAB           5
#define PGSIZE           4096    //bytes of a page
#define NPTENTRIES       524288  //# PTEs per page table (0.5M)

#define PTXSHIFT         12      //offset of PTX in a linear address

//page table index
#define PTX(va)          (((uint32_t)va >> PTXSHIFT) & 0xfffff)

#define PGROUNDUP(sz)    (((sz)+PGSIZE-1) & ~(PGSIZE-1))
#define PGROUNDDOWN(a)   ((a) & ~(PGSIZE-1))

//Page table entry flags
// 31                              12 11 9  8   7   6   5  4    0
// +--------------20-----------------+----+---+---+---+---+------+
// |       Page Table Index          |  C | D | V | G | R |      |
// +---------------------------------+----+---+---+---+---+------+
#define PTE_G            0x040 //global
#define PTE_V            0x080 //valid
#define PTE_D            0x100 //dirty
#define PTE_C            0x400 //Consist

typedef struct pte {
    uint32_t ptentry;
    uint32_t sd_sect;
    uint8_t  asid;
} pte_t;

pte_t pgtab[NPTENTRIES];

//free physical space
#define MAXPFRAME        0x1fff
#define NPFRAMES         4096 //page frame number
typedef struct pgframe {
    uint32_t paddr; //physical addr
    uint32_t vaddr; //virtual addr
    pte_t * vpte;
    struct pgframe *prev;
    struct pgframe *next;
} pgframe_t;
pgframe_t pmem[NPFRAMES];
queue_t freelist;
queue_t busylist;

//SD-card address
extern uint32_t sd_paddr;

#define NSECTORS        8 //sectors per page frame

void freerange(uint32_t, uint32_t);

void do_TLB_Refill();
void do_TLB_Invalid();
void do_page_swap();

#endif
