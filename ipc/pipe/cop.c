/*
* 使用范例
* 协同进程，与过滤程序uluc协同使用
*/

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main(int argc, char* argv[])
{
	int pfdin[2], pfdout[2];
	int ret;
	int pid;
	char buf[1024] = {0};

	if(3 > argc) {
		printf("Plz input 3 parameters at least\n");
		return 0;
	}


	ret = pipe(pfdin);
	if(-1 == ret) {
		printf("Fail to pipe, errno: %d, errmsg: %s\n", errno, strerror(errno));
		return 0;
	}
	ret = pipe(pfdout);
	if(-1 == ret) {
		printf("Fail to pipe, errno: %d, errmsg: %s\n", errno, strerror(errno));
		return 0;
	}

	pid = fork();
	if(0 > pid) {
		printf("Fail to fork, errno: %d, errmsg: %s\n", errno, strerror(errno));
		return 0;
	}
	else if(0 == pid) { //child process
		FILE* log = fopen("filter.log", "w");
	
		close(pfdin[0]);
		if(STDOUT_FILENO != pfdin[1]) {
			if(STDOUT_FILENO != dup2(pfdin[1], STDOUT_FILENO)) {
				fputs("Fail to dup2\n", log);
				fflush(log);
				// printf("Fail to dup2\n");
				return 0;
			}
			close(pfdin[1]);
		}
		close(pfdout[1]);
		if(STDIN_FILENO != pfdout[0]) {
			if(STDIN_FILENO != dup2(pfdout[0], STDIN_FILENO)) {
				fputs("Fail to dup2\n", log);
				fflush(log);
				// printf("Fail to dup2\n");
				return 0;
			}
			close(pfdout[0]);
		}

		fputs("ready to exec\n", log);
		fflush(log);
		if(-1 == execl(argv[1], argv[2], (char*)0)) {
			printf("Fail to execl, errno: %d, errmsg: %s\n", errno, strerror(errno));
			return 0;
		}
	}

	//parent process continue
	close(pfdin[1]);
	close(pfdout[0]);

	while(NULL != gets(buf)) {
		printf("ready to write to filter\n");
		buf[strlen(buf)] = '\n';
		if(-1 == write(pfdout[1], buf, strlen(buf))) {
			printf("Fail to write string to filter\n");
			return 0;
		}
		printf("ready to read from filter\n");
		if(0 >= read(pfdin[0], buf, sizeof(buf))) {
			printf("Fail to read string from filter\n");
			return 0;
		}
		printf("ready to write to stdout\n");
		if(EOF == puts(buf)) {
			printf("Fail to write string to stdout\n");
			return 0;
		}
	}

	printf("Read EOF from stdin, process exit.\n");
	buf[0] = EOF;
	buf[1] = '\n';
	if(-1 == write(pfdout[1], buf, strlen(buf))) {
		printf("Fail to write EOF to filter\n");
		return 0;
	}
	return 0;
}