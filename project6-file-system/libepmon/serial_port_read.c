void *serial_port_read(unsigned char *buf, int size){
	unsigned char *read_port = (unsigned char *)(0xbfe48000 + 0x00);
	unsigned char *stat_port = (unsigned char *)(0xbfe48000 + 0x05);

	int i = 0;

	while ((*stat_port & 0x01))                                                                                       
	{
		char ch = *read_port;

		if (i < size)
		{
			buf[i++] = ch;
		}
	}
}
