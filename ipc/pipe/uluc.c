/*
* 使用范例
* 将从标准输入读入的字符转换成小写字符，然后按行输出的标准输出的字符过滤程序
*/

#include <stdio.h>
#include <ctype.h>

int main()
{
	int  c;
	
	FILE* log = fopen("filter.log", "w");

	while(EOF != (c = getchar())) {
		fputs("Got char\n", log);
		fflush(log);

		if(!islower(c)) {
			c = tolower(c);
		}
		putchar(c);
		if('\n' == c) {
			fflush(stdout);
			fputs("Flushed\n", log);
			fflush(log);
		}
	}

	return 0;
}
