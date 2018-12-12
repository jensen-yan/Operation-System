#include <stdio.h>
int main()
{
	FILE *fp = fopen("image1", "rb");
	FILE *fq = fopen("image", "rb");
	char buf1[8192], buf2[8192];
	fread(buf1, 1, 8192, fp);
	fread(buf2, 1, 8192, fq);
	for(int i=0; i<8192; i++)
		if(buf1[i] != buf2[i])
			printf("i=%d: right:%x, wrong:%x\n", i, buf1[i], buf2[i]);
	return 0;
}
