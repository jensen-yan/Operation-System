int isQEMU()
{
	unsigned long *tempaddr = (unsigned long *)0x8005f65c;
        if(*(unsigned long *)(tempaddr + 0) == 0x27bdffa8){
		return 1;
        }else{
		return 0;
	}
}
