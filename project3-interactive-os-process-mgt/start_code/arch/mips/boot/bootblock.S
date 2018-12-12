.text
.global main

main:
	# use your own code in here!
	# 1) call BIOS print string "It's bootblock!"
la	$a0, msg
jal 0x8007b980
	# 2) call BIOS read kernel in SD card.
li	$a0, 0xa0800200
li	$a1, 0x200
li	$a2, 0x200
lb	$t0, 0xa08001fe
mul $a2, $t0, $a2
jal	0x8007b1cc
	# 3) jump to kernel.
jal	0xa0800200
# 4) while(1) --> stop here
stop:
	j stop
.data

msg: .ascii "It's a bootloader...\n"

# 1. PMON read SD card function address
# read_sd_card();
read_sd_card: .word 0x8007b1cc

# 2. PMON print string function address
# printstr(char *string)
printstr: .word 0x8007b980

# 3. PMON print char function address
# printch(char ch)
printch: .word 0x8007ba00

# 4. kernel address (move kernel to here ~)
kernel : .word 0xa0800200

# 5. kernel main address (jmp here to start kernel main!)
kernel_main : .word 0xa0800200
