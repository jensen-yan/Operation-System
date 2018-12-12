#include "asm.h"

.equ    KERNEL, 0
.equ    USER,   156

.macro STI
	mfc0	k0, CP0_STATUS
	ori		k0, 0x1
	mtc0	k0, CP0_STATUS
.endm

.macro CLI
	mfc0	k0, CP0_STATUS
	li		k1, 0xfffffffe
	and		k0, k0, k1
	mtc0	k0, CP0_STATUS
.endm

LEAF(asm_start)
    mtc0    zero, CP0_STATUS
    mtc0    zero, CP0_WATCHLO
    mtc0    zero, CP0_WATCHHI

    mfc0    t0, CP0_CONFIG
    and     t0, ~0x7
    ori     t0, 0x2
    mtc0    t0, CP0_CONFIG
    
    jr  	ra
	nop
END(asm_start)

LEAF(TLB_flush)
	li		a3, 0
	mtc0	zero, CP0_PAGEMASK
	li		a2, 64
	lui		t0, 0x8000
1:
	mtc0	t0, CP0_ENTRYHI
	mtc0	zero, CP0_ENTRYLO0
	mtc0	zero, CP0_ENTRYLO1
	mtc0	a3, CP0_INDEX
	addiu	a3, 1
	tlbwi
	bne		a3, a2, 1b
	nop
	jr		ra
	nop
END(TLB_flush)

LEAF(get_cp0_status)
	mfc0	v0, CP0_STATUS
	jr		ra
	nop
END(get_cp0_status)

LEAF(set_cp0_status)
	mtc0	a0, CP0_STATUS
	jr		ra
	nop
END(set_cp0_status)

LEAF(get_cp0_cause)
	mfc0	v0, CP0_CAUSE
	jr		ra
	nop
END(get_cp0_cause)

LEAF(get_cp0_COUNT)
	mfc0	v0, CP0_COUNT
	jr		ra
	nop
END(get_cp0_COUNT)

LEAF(set_cp0_COUNT)
	mtc0	a0, CP0_COUNT
	jr		ra
	nop
END(set_cp0_COUNT)

LEAF(get_cp0_COMPARE)
	mfc0	v0, CP0_COMPARE
	jr		ra
	nop
END(get_cp0_COMPARE)

LEAF(set_cp0_COMPARE)
	mtc0	a0, CP0_COMPARE
	jr		ra
	nop
END(set_cp0_COMPARE)

LEAF(get_cp0_entryhi)
	mfc0	v0, CP0_ENTRYHI
	jr		ra
	nop
END(get_cp0_entryhi)

LEAF(get_cp0_context)
	mfc0	v0, CP0_CONTEXT
	jr		ra
	nop
END(get_cp0_context)

LEAF(set_cp0_entryhi)
	mtc0	a0, CP0_ENTRYHI
	jr		ra
	nop
END(set_cp0_entryhi)

LEAF(get_cp0_entrylo0)
	mfc0	v0, CP0_ENTRYLO0
	jr		ra
	nop
END(get_cp0_entrylo0)

LEAF(get_cp0_entrylo1)
	mfc0	v0, CP0_ENTRYLO1
	jr		ra
	nop
END(get_cp0_entrylo1)

LEAF(get_cp0_index)
	mfc0	v0, CP0_INDEX
	jr		ra
	nop
END(get_cp0_index)

LEAF(set_cp0_entrylo0)
	mtc0	a0, CP0_ENTRYLO0
	jr		ra
	nop
END(set_cp0_entrylo0)

LEAF(set_cp0_entrylo1)
	mtc0	a0, CP0_ENTRYLO1
	jr		ra
	nop
END(set_cp0_entrylo1)

.macro SAVE_CONTEXT offset
    // TODO save context
    lw      k0, current_running
	addi	k0, k0, \offset
    sw      AT, OFFSET_REG1(k0)
    sw      v0, OFFSET_REG2(k0)
    sw      v1, OFFSET_REG3(k0)
	sw      a0, OFFSET_REG4(k0)
	sw      a1, OFFSET_REG5(k0)
	sw      a2, OFFSET_REG6(k0)
	sw      a3, OFFSET_REG7(k0)
	sw      t0, OFFSET_REG8(k0)
	sw      t1, OFFSET_REG9(k0)
	sw      t2, OFFSET_REG10(k0)
	sw      t3, OFFSET_REG11(k0)
	sw      t4, OFFSET_REG12(k0)
	sw      t5, OFFSET_REG13(k0)
	sw      t6, OFFSET_REG14(k0)
	sw      t7, OFFSET_REG15(k0)
	sw      s0, OFFSET_REG16(k0)
	sw      s1, OFFSET_REG17(k0)
	sw      s2, OFFSET_REG18(k0)
	sw      s3, OFFSET_REG19(k0)
	sw      s4, OFFSET_REG20(k0)
	sw      s5, OFFSET_REG21(k0)
	sw      s6, OFFSET_REG22(k0)
	sw      s7, OFFSET_REG23(k0)
	sw      t8, OFFSET_REG24(k0)
	sw      t9, OFFSET_REG25(k0)
	sw      gp, OFFSET_REG28(k0)
	sw      sp, OFFSET_REG29(k0)
    sw      fp, OFFSET_REG30(k0)
	sw      ra, OFFSET_REG31(k0)
	mfc0	k1, CP0_STATUS
	sw		k1, OFFSET_STATUS(k0)
	mfc0	k1, CP0_ENTRYHI
	sw		k1, OFFSET_HI(k0)
	#mflo	k1
	#sw		k1, OFFSET_LO(k0)
	mfc0	k1, CP0_BADVADDR
	sw		k1, OFFSET_BADVADDR(k0)
	#mfc0	k1, CP0_CAUSE
	#sw		k1, OFFSET_CAUSE(k0)
	mfc0	k1, CP0_EPC
	sw		k1, OFFSET_EPC(k0)
.endm

.macro RESTORE_CONTEXT offset
    // TODO restore context
    lw      k0, current_running
	addi	k0, k0, \offset
    lw      AT, OFFSET_REG1(k0)
    lw      v0, OFFSET_REG2(k0)
    lw      v1, OFFSET_REG3(k0)
	lw      a0, OFFSET_REG4(k0)
	lw      a1, OFFSET_REG5(k0)
	lw      a2, OFFSET_REG6(k0)
	lw      a3, OFFSET_REG7(k0)
	lw      t0, OFFSET_REG8(k0)
	lw      t1, OFFSET_REG9(k0)
	lw      t2, OFFSET_REG10(k0)
	lw      t3, OFFSET_REG11(k0)
	lw      t4, OFFSET_REG12(k0)
	lw      t5, OFFSET_REG13(k0)
	lw      t6, OFFSET_REG14(k0)
	lw      t7, OFFSET_REG15(k0)
	lw      s0, OFFSET_REG16(k0)
	lw      s1, OFFSET_REG17(k0)
	lw      s2, OFFSET_REG18(k0)
	lw      s3, OFFSET_REG19(k0)
	lw      s4, OFFSET_REG20(k0)
	lw      s5, OFFSET_REG21(k0)
	lw      s6, OFFSET_REG22(k0)
	lw      s7, OFFSET_REG23(k0)
	lw      t8, OFFSET_REG24(k0)
	lw      t9, OFFSET_REG25(k0)
	lw      gp, OFFSET_REG28(k0)
	lw      sp, OFFSET_REG29(k0)
    lw      fp, OFFSET_REG30(k0)
	lw      ra, OFFSET_REG31(k0)
	lw		k1, OFFSET_STATUS(k0)
	#ori		k1, 0x2
	#xori	k1, 0x2
	mtc0	k1, CP0_STATUS
	lw		k1, OFFSET_HI(k0)
	mtc0	k1, CP0_ENTRYHI
	#lw		k1, OFFSET_LO(k0)
	#mtlo	k1
	lw		k1, OFFSET_BADVADDR(k0)
	mtc0	k1, CP0_BADVADDR
	#lw		k1, OFFSET_CAUSE(k0)
	#mtc0	k1, CP0_CAUSE
	lw		k1, OFFSET_EPC(k0)
	mtc0	k1, CP0_EPC
.endm

NESTED(do_scheduler, 0, ra)
    SAVE_CONTEXT(KERNEL)
    jal     scheduler
	nop
    RESTORE_CONTEXT(KERNEL)
    jr      ra
	nop
END(do_scheduler)

.global exception_handler_begin
.global exception_handler_end
.global TLBexception_handler_begin
.global TLBexception_handler_end
.global first_run_handle

NESTED(TLBexception_handler_entry, 0, sp)
TLBexception_handler_begin:
	//TODO: TLB exception entry
	//jmp exception_handler[i] which decided by CP0_CAUSE
	CLI
	SAVE_CONTEXT(USER)
	#mtc0	zero, CP0_PAGEMASK
	#nop
	#mfc0	k0, CP0_CONTEXT
	#sll		k0, k0, 9
	#srl		k0, k0, 10
	#lw		k1, pgtab
	#add		k1, k1, k0
	#lw		k0, 0(k1)
	#lw		k1, 4(k1)
	#srl		k0, k0, 6
	#srl		k1, k1, 6
	#mtc0	k0, CP0_ENTRYLO0
	#mtc0	k1, CP0_ENTRYLO1
	#tlbwr
	#jal		do_TLB_Refill
	#RESTORE_CONTEXT(USER)
	#STI
	#eret
	j		handle_TLB
	nop
TLBexception_handler_end:
END(TLBexception_handler_entry)

NESTED(exception_handler_entry, 0, sp)
exception_handler_begin:
    // TODO close interrupt
	CLI
    // jmp exception_handler[i] which decided by CP0_CAUSE
	// Leve2 exception Handler.
	mfc0	k0, CP0_CAUSE
	andi	k0, k0, CAUSE_EXCCODE
	la		k1, exception_handler
	add		k0, k0, k1
	lw		k0, 0(k0)
	jr		k0
	nop
exception_handler_end:
END(exception_handler_entry)

NESTED(handle_int, 0, sp)
    // interrupt handler
    // Leve3 exception Handler.
	SAVE_CONTEXT(USER)
	mfc0	a0, CP0_STATUS
	mfc0	a1, CP0_CAUSE
	addiu	sp, sp, -8
	jal		interrupt_helper
	nop
	addiu	sp, sp, 8
	
first_run_handle:
	//restore, reset count and compare reg
	RESTORE_CONTEXT(USER)
	mtc0	zero, CP0_COUNT
	li		k0, 200000
	mtc0	k0, CP0_COMPARE
	STI
	eret
END(handle_int)

NESTED(handle_syscall, 0, sp)
    // system call handler
	SAVE_CONTEXT(USER)
	add		a3, a2, zero
	add		a2, a1, zero
	add		a1, a0, zero
	add		a0, v0, zero
	addiu	sp, sp, -16
	jal		system_call_helper
	nop
	addiu	sp, sp, 16

	#RESTORE_CONTEXT(USER)
	#mfc0	k0, CP0_EPC
	#addi	k0, k0, 4		# k0 = k0 + 4
	#mtc0	k0, CP0_EPC
	
	RESTORE_CONTEXT(USER)
	STI
	eret
END(handle_syscall)

NESTED(handle_TLB, 0, sp)
	mtc0	zero, CP0_PAGEMASK
	nop
	jal		do_TLB_Refill
	nop
	RESTORE_CONTEXT(USER)
	STI
	eret
END(handle_TLB)

NESTED(handle_other, 0, sp)
    // other exception handler
	RESTORE_CONTEXT(USER)
	STI
	eret
END(handle_other)