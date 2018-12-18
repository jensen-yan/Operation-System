#include "common.h"
#include "string.h"

void port_write_ch(char ch)
{
	typedef void (*FUNC_POINT)(char s);
	FUNC_POINT  _printch = (FUNC_POINT)0x8007ba00;
	_printch(ch);
}

void port_write(char *str)
{
	typedef void (*FUNC_POINT)(char *s);
	FUNC_POINT  _printstr = (FUNC_POINT)0x8007b980;
	_printstr(str);
}