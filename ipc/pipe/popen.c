/*
* popen,pclose函数使用
*/

#include <stdio.h>

int main(int argc, char* argv[])
{
	FILE* fpin;
	char buf[1024] = {0};

	if(2 > argc) {
		printf("Plz input 2 parameters at least\n");
		return 0;
	}

	fpin = popen(argv[1], "r");
	if(NULL == fpin) {
		printf("Fail to popen\n");
		return 0;
	}

	while(NULL != fgets(buf, sizeof(buf), fpin)) {
		fputs(buf, stdout);
	}

	pclose(fpin);

	return 0;
}