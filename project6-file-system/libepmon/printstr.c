void printstring(char *buf){
	typedef void (*FUNC_POINT)(char *s);
	FUNC_POINT  printstr = (FUNC_POINT)0x8007b980;
	//FUNC_POINT  printstr = (FUNC_POINT)0x80010200;
	printstr(buf);
}
