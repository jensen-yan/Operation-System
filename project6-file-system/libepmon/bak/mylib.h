#ifndef __MY_LIB_H__
#define __MY_LIB_H__

void printstring(char *s);
void printchar(char ch);

int sdwrite(unsigned char *buf, unsigned int base, int n);
int sdread(unsigned char *buf, unsigned int base, int n);

//void *memcpy(void *s1, const void *s2, int n);

#endif
