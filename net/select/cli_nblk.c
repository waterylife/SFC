#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <stdio.h>
#include <string.h>

#include "../define.h"

//采用非阻塞select方式为connection建立超时机制
//timeout单位毫秒
//成功：返回fd
//失败：返回-1
int open_connection(int srv_port, int timeout)
{
	int ret;
	struct sockaddr_in srv_addr;
	int connfd, maxfd;
	fd_set readfds, writefds;
	struct timeval timeout_val;

	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = INADDR_ANY;
	srv_addr.sin_port = htons(srv_port);

	connfd = socket(AF_INET, SOCK_STREAM, 0);

	int val = fcntl(connfd, F_GETFL, 0);
	fcntl(connfd, F_SETFL, val | O_NONBLOCK);

	ret = connect(connfd, (const struct sockaddr*)&srv_addr, sizeof(const struct sockaddr));
	if(-1 == ret && EINPROGRESS != errno) {
		printf("Fail to connect, ret: %d, retmsg: %s\n", 
				errno, strerror(errno));
		goto err;
	}
	else if(0 == ret) {
		printf("connect to server immediately\n");
		goto success;
	}

	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_SET(connfd, &readfds);
	FD_SET(connfd, &writefds);

	timeout_val.tv_sec = timeout / 1000;
	timeout_val.tv_usec = (timeout % 1000) % 1000;

	ret = select(connfd + 1, &readfds, &writefds, NULL, &timeout_val);
	if(0 == ret) {
		printf("select timeout\n");
		goto err;
	}

	if(FD_ISSET(connfd, &readfds) || FD_ISSET(connfd, &writefds)) {
		int error = 0;
		int err_len = sizeof(error);
		if(0 > getsockopt(connfd, SOL_SOCKET, SO_ERROR, &error, &err_len) ||
			0 != error) {
			printf("get socket opt error, error: %d\n", error);
			goto err;
		}
		goto success;
	}
	else {
		printf("no connect fd readable or writeable\n");
		goto err;
	}

err:
	close(connfd);
	return -1;

success:
	return connfd;
}

int send_and_recv_msg_from_srv(int connfd)
{
	fd_set readfds, writefds;
	int ret;
	struct timeval timeout;
	int connfds[MAX_CONNECTIONS];
	char to_buf[READ_BUF_LEN], fr_buf[READ_BUF_LEN];
	int maxfd;
	char *toiptr, *tooptr, *friptr, *froptr;
	int stdineof = 0;


	int val = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, val | O_NONBLOCK);	
	val = fcntl(STDOUT_FILENO, F_GETFL, 0);
	fcntl(STDOUT_FILENO, F_SETFL, val | O_NONBLOCK);	
	val = fcntl(connfd, F_GETFL, 0);
	fcntl(connfd, F_SETFL, val | O_NONBLOCK);	

	maxfd = ((connfd > STDOUT_FILENO) ? connfd : STDOUT_FILENO) + 1;

	toiptr = tooptr = to_buf;
	friptr = froptr = fr_buf;

	while(1) {
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);

		if(stdineof == 0 && toiptr < &to_buf[READ_BUF_LEN]) {
			printf("set stdin for read\n");
			FD_SET(STDIN_FILENO, &readfds);
		}
		if(friptr < &fr_buf[READ_BUF_LEN]) {
			printf("set connfd  for read\n");
			FD_SET(connfd, &readfds);
		}
		if(toiptr != tooptr) {
			printf("set connfd for write\n");
			FD_SET(connfd, &writefds);
		}
		if(friptr != froptr) {
			printf("set stdout for write\n");
			FD_SET(STDOUT_FILENO, &writefds);
		}

		printf("max fd: %d\n", maxfd);

		ret = select(maxfd, &readfds, &writefds, NULL, NULL);
		if(0 == ret) {
			printf("select timeout, ret: %d, retmsg: %s\n", errno, strerror(errno));
			break;
		}
		else if(-1 == ret) {
			printf("Fail to select, ret: %d, retmsg: %s\n", errno, strerror(errno));
			break;
		}
		else {
			if(FD_ISSET(STDIN_FILENO, &readfds)) { //从标准输入读
				printf("stdin is in readfds\n");
				int rd_len = read(STDIN_FILENO, toiptr, &to_buf[READ_BUF_LEN] - toiptr);
				printf("read len from stdin: %d\n", rd_len);
				if(-1 == rd_len && errno != EAGAIN) {
					printf("fail to read from stdin, ret: %d, retmsg: %s\n", errno, strerror(errno));
					break;
				}
				else if(0 == rd_len) {
					stdineof = 1;
				}
				else {
					toiptr += rd_len;
					FD_SET(connfd, &writefds);
				}
			}
			if(FD_ISSET(connfd, &readfds)) { //从socket读
				printf("connfd is in readfds\n");
				int rd_len = read(connfd, friptr, &fr_buf[READ_BUF_LEN] - friptr);
				printf("read len from connfd: %d\n", rd_len);
				if(-1 == rd_len && errno != EAGAIN) {
					printf("fail to read from socket, ret: %d, retmsg: %s\n", errno, strerror(errno));
					break;
				}
				else if(0 == rd_len) {
					printf("server close socket.\n");
					break;
				}
				else {
					friptr += rd_len;
					FD_SET(STDOUT_FILENO, &writefds);
				}
			}
			if(FD_ISSET(connfd, &writefds)) { //写到socket
				printf("connfd is in writefds\n");
				int wd_len = write(connfd, tooptr, toiptr - tooptr);
				printf("write len to connfd: %d\n", wd_len);
				if(-1 == wd_len && errno != EAGAIN) {
					printf("fail to write to socket, errno: %d, errmsg: %s\n", errno, strerror(errno));
					break;
				}
				else {
					tooptr += wd_len;
					if(toiptr == tooptr) {
						toiptr = tooptr = to_buf; //缓冲区数据已被处理完毕，重置指针
						if(stdineof == 1) { 
							shutdown(connfd, SHUT_WR);
						}
					}
				}
			}
			if(FD_ISSET(STDOUT_FILENO, &writefds)) { //写到标准输出
				printf("stdout is in writefds\n");
				int wd_len = write(STDOUT_FILENO, froptr, friptr - froptr);
				printf("write len to stdout: %d\n", wd_len);
				if(-1 == wd_len && errno != EAGAIN) {
					printf("fail to write to stdout, errno: %d, errmsg: %s\n", errno, strerror(errno));
					break;
				}
				else {
					froptr += wd_len;
					if(friptr == froptr) {
						friptr = froptr = fr_buf; //缓冲区数据已被处理完毕，重置指针
					}
				}
			}
		} //else

	} //while

	return 0;
}

int main(int argc, char* argv[])
{
	if(argc < 3) {
		printf("plz input 3 parameters at least!\n");
		return 0;
	}

	int connfd;
	connfd = open_connection(atoi(argv[1]), atoi(argv[2]));
	if(-1 != connfd) {
		send_and_recv_msg_from_srv(connfd);
	}

	return 0;
}
