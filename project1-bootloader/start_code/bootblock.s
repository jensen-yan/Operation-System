.text
.global main

main:
	# 1) task1 call BIOS print string "It's bootblock!"
#print string
la $a0, msg
jal 0x8007b980
	# 2) task2 call BIOS read kernel in SD card and jump to kernel start
la $a0, 0xa0800200
li $a1, 0x200   #offset in sd card <- 0x200
li $a2, 0x200
lb $t0, 0xa08001ff
mul $a2, $t0, $a2
jal 0x8007b1cc  #read_sd_card

jal 0xa0800200
# while(1) --> stop here
stop:
	j stop

.data

msg: .ascii "It's a bootloader...\n"
kernelsz: .word 0xa08001fe
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
