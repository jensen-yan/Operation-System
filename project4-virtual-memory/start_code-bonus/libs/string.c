#include "string.h"

int strlen(char *src)
{
	int i;
	for (i = 0; src[i] != '\0'; i++)
	{
	}
	return i;
}

void memcpy(uint8_t *dest, uint8_t *src, uint32_t len)
{
	for (; len != 0; len--)
	{
		*dest = *src;
		dest++, src++;
	}
}

void memset(void *dest, uint8_t val, uint32_t len)
{
	uint8_t *dst = (uint8_t *)dest;

	for (; len != 0; len--)
	{
		*dst++ = val;
	}
}

void bzero(void *dest, uint32_t len)
{
	memset(dest, 0, len);
}

int strcmp(char *str1, char *str2)
{
	while (*str1 && *str2 && (*str1 == *str2))
	{
		str1++, str2++;
	}

	if (*str1 == '\0' && *str2 == '\0')
	{
		return 0;
	}

	if (*str1 == '\0')
	{
		return -1;
	}

	return 1;
}

char *strcpy(char *dest, char *src)
{
	char *tmp = dest;

	while (*src)
	{
		*dest = *src;
		dest++, src++;
	}

	*dest = '\0';

	return tmp;
}

int itoa(char *str, int radix)
{
	int l = strlen(str);
	int ans = 0, i;

	for(i = 0; i < l; i++)
	{
		if(radix <= 10 || (str[i] <= '9' && str[i] >= '0'))
			ans = ans*radix + str[i]-'0';
		else if(str[i] >= 'a' && str[i] <= 'f')
			ans = ans*radix + str[i]-'a' + 10;
		else if(str[i] >= 'A' && str[i] <= 'F')
			ans = ans*radix + str[i]-'A' + 10;
	}
	
	return ans;
}