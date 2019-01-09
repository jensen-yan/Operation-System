unsigned int Write_Single_Block(unsigned long int ByteAddress,unsigned char *WriteBuffer)
{

	typedef unsigned char (*FUNC_POINT1)(unsigned char value);	//flash_writeb_cmd
	typedef void (*FUNC_POINT2)(unsigned int IOData);		//SD_2Byte_Write
	typedef void (*FUNC_POINT3)(unsigned int IOData);		//SD_Write
	typedef unsigned short (*FUNC_POINT4)(void);			//SD_Read
	typedef unsigned int (*FUNC_POINT5)(unsigned int CMDIndex, unsigned long CMDArg, unsigned int ReaType, unsigned int CSLowRSV);	//SD_CMD_Write
	
	FUNC_POINT1 flash_writeb_cmd	=	(FUNC_POINT1)0x800794c0;
	FUNC_POINT2 SD_2Byte_Write	=	(FUNC_POINT2)0x800794f8;
	FUNC_POINT3 SD_Write		=	(FUNC_POINT3)0x80079528;
	FUNC_POINT4 SD_Read		=	(FUNC_POINT4)0x80079564;
	FUNC_POINT5 SD_CMD_Write	=	(FUNC_POINT5)0x80079588;

	unsigned int temp, Response, MaximumTimes;
	MaximumTimes = 10;
	
	for (temp=0; temp<MaximumTimes; temp++) {
		/* Send CMD24 */
		Response = SD_CMD_Write(24, ByteAddress, 1, 1);
		if (Response == 0xff00){
			break;
		}
	}
	
	/* Provide 8 extra clock after CMD response */
	flash_writeb_cmd(0xff);

	/* Send Start Block Token */
	SD_Write(0x00fe);
	for (temp=0; temp<256; temp++) {
		/* Data Block */
		SD_2Byte_Write(((WriteBuffer[2*temp])<<8) | (WriteBuffer[2*temp+1]));
	}
	/* Send 2 Bytes CRC */
	SD_2Byte_Write(0xffff);
	
	Response = SD_Read();
	while (SD_Read()!=0xffff) {;}
	
	//set_cs(1);
	*(volatile unsigned char *)( ((void *)(0xa0000000 | (unsigned int)(0x1fe80000+0x05))) ) = (0xFF);	//set_cs(1)
	
	/* Provide 8 extra clock after data response */
	flash_writeb_cmd(0xff);
	
	return Response;
}
