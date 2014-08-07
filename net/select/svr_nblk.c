#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <stdio.h>
#include <string.h>

#include "../define.h"

int open_server(int svr_port)
{
	struct sockaddr_in srv_sock;
	fd_set readfds, writefds;
	int ret, max_fd;
	struct timeval timeout;
	int connfds[MAX_CONNECTIONS];
	int curfd = -1;
	char buf[MAX_CONNECTIONS][READ_BUF_LEN];
	int buf_len[MAX_CONNECTIONS] = {0};

	srv_sock.sin_family = AF_INET;
	srv_sock.sin_addr.s_addr = INADDR_ANY;
	srv_sock.sin_port = htons(svr_port);

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

	int val = fcntl(listen_fd, F_GETFL, 0);
	fcntl(listen_fd, F_SETFL, val | O_NONBLOCK);

	while(1) {
		printf("curfd: %d\n", curfd);
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_SET(listen_fd, &readfds);

		int i;
		for(i = 0; i <= curfd; i++) {
			printf("connfd: %d\n", connfds[i]);
			if(0 < connfds[i]) {
				FD_SET(connfds[i], &readfds);
				if(buf_len[i] > 0) FD_SET(connfds[i], &writefds);
			} 
		}

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
				max_fd = ((max_fd > new_conn_fd + 1) ? max_fd : new_conn_fd + 1);\
				val = fcntl(new_conn_fd, F_GETFL, 0);
				fcntl(new_conn_fd, F_SETFL, val | O_NONBLOCK);
			}
			for(i = 0; i <= curfd; i++) {
				if(0 == connfds[i]) continue;

				if(FD_ISSET(connfds[i], &readfds)) {
					int rd_len = read(connfds[i], &buf[i][buf_len[i]], sizeof(buf[i]) - buf_len[i]);
					printf("read len: %d\n", rd_len);
					if(-1 == rd_len && errno != EAGAIN) {
						printf("Fail to read, ret: %d, retmsg: %s\n", 
								errno, strerror(errno));
						buf_len[i] = 0;
						close(connfds[i]);
						connfds[i] = 0;
					}
					else if(0 == rd_len) {
						printf("socket closed by cient\n"); 
						buf_len[i] = 0;
						close(connfds[i]);
						connfds[i] = 0;
					}
					else {
						buf_len[i] += rd_len;
						FD_SET(connfds[i], &writefds);
					}
				}
			}
			for(i = 0; i <= curfd; i++) {
				if(0 == connfds[i]) continue;
				
				if(FD_ISSET(connfds[i], &writefds)) {
					int wd_len = write(connfds[i], buf[i], buf_len[i]);
					printf("write len: %d\n", wd_len);
					if(-1 == wd_len && EAGAIN != errno) {
						printf("Fail to write, ret: %d, retmsg: %s\n", 
								errno, strerror(errno));
						buf_len[i] = 0;
						close(connfds[i]);
						connfds[i] = 0;
					}
					else {
						buf_len[i] -= wd_len;
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

int main(int argc, char* argv[])
{
	if(argc < 2) {
		printf("Plz input 2 parameters at least\n");
		return 0;
	}

	open_server(atoi(argv[1]));

	return 0;
}
