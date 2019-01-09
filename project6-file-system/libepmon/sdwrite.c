int sdwrite(unsigned char *buf, unsigned int base, int n)
//int sdwrite(void *buf, unsigned int base, int n)
{

	unsigned long *tempaddr = (unsigned long *)0x8005f65c;
        if(isQEMU()){
		/*QEMU sdwrite*/
		typedef int (*FUNC_POINT)(unsigned int base, int n, unsigned char *buffer);
		FUNC_POINT new_sdcard_write = (FUNC_POINT)0x8007aba4;
		new_sdcard_write(base, n, buf);
        }else{
		/*LS1C sdwrite*/
		int i;
		typedef unsigned int (*FUNC_POINT)(unsigned long int ByteAddress, unsigned char *ReadBuffer);
		FUNC_POINT Read_Single_Block = (FUNC_POINT)0x80079cd8;

		unsigned int pos = base;
		unsigned int left = n;
		unsigned int once;
		unsigned char *indexbuf = (unsigned char *)(0xa0100400 + 0x0);
		//void *indexbuf = (void *)(0xa0100400 + 0x0);

		while (left) {
			once = (512 - (pos&511)) < (left) ? ((512 - (pos&511))) : (left);
			memcopy(indexbuf+(pos&511), buf, once);

			Write_Single_Block(pos>>9, indexbuf);

			buf += once;
			pos += once;
			left -= once;
		}
		base = pos;
	}

        return (n);
}

