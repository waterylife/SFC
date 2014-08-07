#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <errno.h>

#include <stdio.h>
#include <string.h>

#include "../define.h"

int echo_to_client(int connfd)
{
	int ret;
	char read_buf[READ_BUF_LEN];

	memset((void*)read_buf, 0, sizeof(read_buf));
	ret = read(connfd, read_buf, sizeof(read_buf));
	if(0 < ret) {
		printf("read content from client: %s\n", read_buf);
		ret = send(connfd, read_buf, strlen(read_buf), 0);
		if(-1 == ret) {
			printf("Fail to send, ret: %d, retmsg: %s\n", 
				errno, strerror(errno));
		}
		return 0;
	}

	return -1;
}

int open_server()
{
	struct sockaddr_in srv_sock;
	fd_set readfds;
	int ret, max_fd;
	struct timeval timeout;
	int connfds[MAX_CONNECTIONS];
	int curfd = -1;

	srv_sock.sin_family = AF_INET;
	srv_sock.sin_addr.s_addr = INADDR_ANY;
	srv_sock.sin_port = htons(9875);

	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	memset(connfds, 0, sizeof(connfds));

	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	max_fd = listen_fd + 1;

	ret = bind(listen_fd, (struct sockaddr*)&srv_sock, sizeof(struct sockaddr));
	if(0 != ret) {
		printf("Fail to bind, ret: %d, retmsg: %s\n", errno, strerror(errno));
		return -1;
	}

	ret = listen(listen_fd, 0);
	if(0 != ret) {
		printf("Fail to listen, ret: %d, retmsg: %s\n", errno, strerror(errno));
		return -1;
	}

	while(1) {
		printf("curfd: %d\n", curfd);
		FD_ZERO(&readfds);
		FD_SET(listen_fd, &readfds);
		int i;
		for(i = 0; i <= curfd; i++) {
			printf("connfd: %d\n", connfds[i]);
			if(0 < connfds[i]) FD_SET(connfds[i], &readfds);
		}

		printf("max_fd: %d\n", max_fd);

		ret = select(max_fd, &readfds, NULL, NULL, NULL);
		if(0 == ret) {
			printf("select timeout, ret: %d, retmsg: %s\n", errno, strerror(errno));
			break;
		}
		else if(0 < ret) {
			if(FD_ISSET(listen_fd, &readfds)) {
				int new_conn_fd = accept(listen_fd, NULL, NULL);
				if(-1 == new_conn_fd) {
					printf("Fail to accept, ret: %d, retmsg: %s\n", 
						errno, strerror(errno));
					continue;
				}
				connfds[++curfd] = new_conn_fd;
				max_fd = ((max_fd > new_conn_fd + 1) ? max_fd : new_conn_fd + 1);
			}
			for(i = 0; i <= curfd; i++) {
				if(FD_ISSET(connfds[i], &readfds)) {
					printf("000000\n");
					ret = echo_to_client(connfds[i]);
					if(-1 == ret) {
						close(connfds[i]);
						connfds[i] = 0;
					}
				}
			}
		}
		else {
			printf("Fail to select, ret: %d, retmsg: %s\n", errno, strerror(errno));
			break;
		}
	}

	return 0;
}

int main()
{
	open_server();

	return 0;
}