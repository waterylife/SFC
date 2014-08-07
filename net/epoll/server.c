#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <errno.h>

#include <string.h>
#include <stdio.h>

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

int open_server_lt()
{
	int ret;
	int listen_fd, epoll_fd;
	struct sockaddr_in srv_addr;
	struct epoll_event evs[MAX_EVENTS];
	struct epoll_event ev;

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);

	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = INADDR_ANY;
	srv_addr.sin_port = htons(9001);

	ret = bind(listen_fd, (struct sockaddr*)&srv_addr, sizeof(struct sockaddr));
	if(0 != ret) {
		printf("Fail to bind, ret: %d, retmsg: %s\n", errno, strerror(errno));
		return -1;
	}

	ret = listen(listen_fd, 0);
	if(0 != ret) {
		printf("Fail to listen, ret: %d, retmsg: %s\n", errno, strerror(errno));
		return -1;
	}

	epoll_fd = epoll_create(MAX_EVENTS);
	if(-1 == epoll_fd) {
		printf("Fail to create epoll fd, ret: %d, retmsg: %s\n", errno, strerror(errno));
		return -1;
	}
	
	ev.events = EPOLLIN;
	ev.data.fd = listen_fd;
	ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev);
	if(-1 == ret) {
		printf("Fail to epoll_ctl, ret: %d, retmsg: %s\n", errno, strerror(errno));
		return -1;
	}

	while(1) {
		int nready = epoll_wait(epoll_fd, evs, MAX_EVENTS, -1);
		if(0 == nready) {
			printf("epoll timeout\n");
			return -1;
		}
		if(-1 == nready) {
			printf("Fail to epoll_wait, errno: %d, errmsg: %s\n", errno, strerror(errno));
			return -1;
		}
		int i = 0;
		for(; i < nready; i++) {
			if(listen_fd == evs[i].data.fd) {
				int conn_fd = accept(listen_fd, NULL, NULL);
				if(-1 == conn_fd) {
					printf("Fail to accept, errno: %d, errmsg: %s\n", errno, strerror(errno));
					return -1;
				}
				ev.events = EPOLLIN;
				ev.data.fd = conn_fd;
				ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev);
				if(-1 == ret) {
					printf("Fail to epoll_ctl, ret: %d, retmsg: %s\n", errno, strerror(errno));
					return -1;
				}
				continue;
			}
			if(evs[i].events & EPOLLIN) {
				ret = echo_to_client(evs[i].data.fd);
				if(-1 == ret) {
					ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, evs[i].data.fd, NULL);
					if(-1 == ret) {
						printf("Fail to epoll_ctl, ret: %d, retmsg: %s\n", errno, strerror(errno));
						return -1;
					}
					close(evs[i].data.fd);
				}
			}
		}
	} //while

	close(epoll_fd);
}

int main()
{
	open_server_lt();

	return 0;
}