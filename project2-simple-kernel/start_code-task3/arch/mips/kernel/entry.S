#include "asm.h"

.equ    KERNEL, 0
.equ    USER,   156

.macro STI
	mfc0	k0, CP0_STATUS
	ori		k0, 0x1
#	li		k1, (STATUS_CU0 | STATUS_IE)
#	ori		k1, 0x2
#	or		k0, k0, k1
	mtc0	k0, CP0_STATUS
.endm

.macro CLI
	mfc0	k0, CP0_STATUS
	li		k1, 0xfffffffe
	and		k0, k0, k1
#	li		k1, (STATUS_CU0 | STATUS_IE)
#	or		k0, k0, k1
#	xori	k0, 0x1
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
    
    jr  ra
END(asm_start)

LEAF(GET_CP0_STATUS)
	mfc0	v0, CP0_STATUS
	jr		ra
END(GET_CP0_STATUS)

LEAF(SET_CP0_STATUS)
	mtc0	a0, CP0_STATUS
	jr		ra
END(SET_CP0_STATUS)

LEAF(GET_CP0_COUNT)
	mfc0	v0, CP0_COUNT
	jr		ra
END(GET_CP0_COUNT)

LEAF(SET_CP0_COUNT)
	mtc0	a0, CP0_COUNT
	jr		ra
END(SET_CP0_COUNT)

LEAF(GET_CP0_COMPARE)
	mfc0	v0, CP0_COMPARE
	jr		ra
END(GET_CP0_COMPARE)

LEAF(SET_CP0_COMPARE)
	mtc0	a0, CP0_COMPARE
	jr		ra
END(SET_CP0_COMPARE)

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
	nop
	sw		k1, OFFSET_STATUS(k0)
	mfhi	k1
	sw		k1, OFFSET_HI(k0)
	mflo	k1
	sw		k1, OFFSET_LO(k0)
	mfc0	k1, CP0_BADVADDR
	nop
	sw		k1, OFFSET_BADVADDR(k0)
	mfc0	k1, CP0_CAUSE
	nop
	sw		k1, OFFSET_CAUSE(k0)
	mfc0	k1, CP0_EPC
	nop
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
	mthi	k1
	lw		k1, OFFSET_LO(k0)
	mtlo	k1
	lw		k1, OFFSET_BADVADDR(k0)
	mtc0	k1, CP0_BADVADDR
	lw		k1, OFFSET_CAUSE(k0)
	mtc0	k1, CP0_CAUSE
	lw		k1, OFFSET_EPC(k0)
	mtc0	k1, CP0_EPC
.endm

NESTED(do_scheduler, 0, ra)
    SAVE_CONTEXT(KERNEL)
    jal     scheduler
    RESTORE_CONTEXT(KERNEL)
    jr      ra
END(do_scheduler)

.global exception_handler_begin
.global exception_handler_end

NESTED(exception_handler_entry, 0, sp)
exception_handler_begin:
    // TODO close interrupt
	CLI
	SAVE_CONTEXT(USER)
    // jmp exception_handler[i] which decided by CP0_CAUSE
	// Leve2 exception Handler.
	mfc0	k0, CP0_CAUSE
	andi	k0, k0, CAUSE_EXCCODE
	la		k1, exception_handler
	add		k0, k0, k1
	lw		k0, 0(k0)
	jr		k0
exception_handler_end:
END(exception_handler_entry)

NESTED(handle_int, 0, sp)
    // interrupt handler
    // Leve3 exception Handler.
	mfc0	a0, CP0_STATUS
	mfc0	a1, CP0_CAUSE
	jal		interrupt_helper

	j		exception_handler_finish
END(handle_int)

NESTED(handle_syscall, 0, sp)
    // system call handler
	add		a3, a2, zero
	add		a2, a1, zero
	add		a1, a0, zero
	add		a0, v0, zero
	jal		system_call_helper

	#RESTORE_CONTEXT(USER)
	#mfc0	k0, CP0_EPC
	#addi	k0, k0, 4			# k0 = k0 + 4
	#mtc0	k0, CP0_EPC
	
	j		exception_handler_finish 
END(handle_syscall)

NESTED(handle_other, 0, sp)
    // other exception handler
	#RESTORE_CONTEXT(USER)
	j		exception_handler_finish
END(handle_other)

LEAF(exception_handler_finish)
	#mfc0	k0, CP0_STATUS
	#xori	k0, 0x2
	#mtc0	k0, CP0_STATUS
	RESTORE_CONTEXT(USER)
	STI
	eret
END(exception_handler_finish)