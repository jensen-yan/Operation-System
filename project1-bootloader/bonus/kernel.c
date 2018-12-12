#define PORT 0xbfe48000

void (*printstr)(char *str);

void __attribute__((section(".entry_function"))) _start(void)
{
	// Call PMON BIOS printstr to print message "Hello OS!"
	char *str = "Hello OS!";
	printstr = (void *)0x8007b980;
	(*printstr)(str-0x200);
	asm("b -0x204");
	return;
}
