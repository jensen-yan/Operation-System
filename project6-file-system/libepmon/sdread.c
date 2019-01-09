int sdread(unsigned char *buf, unsigned int base, int n)
//int sdread(void *buf, unsigned int base, int n)
{

	unsigned long *tempaddr = (unsigned long *)0x8005f65c;
        if(isQEMU()){
		/*QEMU sdread*/
		typedef int (*FUNC_POINT)(unsigned char *buffer, unsigned int base, int n);
		//typedef int (*FUNC_POINT)(void *buffer, unsigned int base, int n);
		FUNC_POINT new_sdcard_read = (FUNC_POINT)0x8007b1cc;
		new_sdcard_read(buf, base, n);

        }else{
		/*LS1C sdread*/
		int i;
		typedef unsigned int (*FUNC_POINT)(unsigned long int ByteAddress, unsigned char *ReadBuffer);
		FUNC_POINT Read_Single_Block = (FUNC_POINT)0x80079cd8;

		unsigned int pos = base;
		unsigned int left = n;
		unsigned int once;
		unsigned char *indexbuf = (unsigned char *)(0xa0100200 + 0x0);
		//void *indexbuf = (void *)(0xa0100200 + 0x0);

		while (left) {
			Read_Single_Block(pos>>9, indexbuf);
			once = (512 - (pos&511)) < (left) ? ((512 - (pos&511))) : (left);

			memcopy(buf, indexbuf + (pos&511), once);

			buf += once;
			pos += once;
			left -= once;
		}
		base = pos;
	}

	return (n);

}

