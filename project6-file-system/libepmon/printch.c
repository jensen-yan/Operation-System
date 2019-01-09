void printchar(char ch){
	typedef void (*FUNC_POINT)(char ch);
	FUNC_POINT  printch = (FUNC_POINT)0x8007ba00;
	//FUNC_POINT  printchar = (FUNC_POINT)0x80010210;
	printch(ch);
}
