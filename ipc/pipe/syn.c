/*
* 使用范例
* 使用管道进行父子进程间的同步
*/

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int pfd1[2]; //parent --> child
int pfd2[2]; //parent <-- child

int tell_wait()
{
	if(-1 == pipe(pfd1) || -1 == pipe(pfd2)) {
		printf("Fail to pipe pfd1, errno: %d, errmsg: %s\n", errno, strerror(errno));
		return -1;
	}
	return 0;
}

int tell_child()
{
	char c = 'p';
	if(1 > write(pfd1[1], (const void*)&c, sizeof(c))) {
		printf("fail to write bytes to child, errno: %d, errmsg: %s\n", errno, strerror(errno));
		return -1;
	}
	return 0;
}

int wait_child()
{
	char buf[1024];
	if(1 > read(pfd2[0], buf, sizeof(buf))) {
		printf("fail to read bytes from child, errno: %d, errmsg: %s\n", errno, strerror(errno));
		return -1;
	}
	return 0;
}

int tell_parent()
{
	char c = 'c';
	if(1 > write(pfd2[1], (const void*)&c, sizeof(c))) {
		printf("fail to write bytes to parent, errno: %d, errmsg: %s\n", errno, strerror(errno));
		return -1;
	}
	return 0;
}

int wait_parent()
{
	char buf[1024];
	if(1 > read(pfd1[0], buf, sizeof(buf))) {
		printf("fail to read bytes from parent, errno: %d, errmsg: %s\n", errno, strerror(errno));
		return -1;
	}
	return 0;
}