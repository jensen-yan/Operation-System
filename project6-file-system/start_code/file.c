#include <stdio.h>
#include <string.h>
char buf[1000];
int main()
{
	int i;
	for(i=0; i<1000; i++){
		bzero(buf, sizeof(buf));
		sprintf(buf, "%3d\n", i);
		printf("len=%d ", strlen(buf));
	}
	return 0;
}
