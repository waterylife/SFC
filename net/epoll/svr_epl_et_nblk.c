#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <string.h>
#include <stdio.h>

#include "../define.h"

//非阻塞方式将socket输入缓冲区的数据全部读完
int socket_to_read(int fd, char* buf, int buf_len)
{
	int ret, rd_len = 0;

	while(1) {
		ret = read(fd, buf + rd_len, buf_len - rd_len);
		// printf("ret: %d\n", ret);
		if(0 == ret) return rd_len; 
		if(-1 == ret && EAGAIN == errno) return rd_len;
		if(-1 == ret && EAGAIN != errno) {
			// printf("-------FAIL-----------------\n");
			return -1;
		}
		rd_len += ret;
	}
}

//非阻塞方式将socket输出缓冲区写满
int socket_to_send2(int fd, char* buf, int buf_len)
{
	sleep(3);
	printf("buf: %x, buf_len: %d\n", buf, buf_len);
	int ret, wr_len = 0;

	while(1) {
		ret = write(fd, buf + wr_len, buf_len - wr_len);
		if(-1 == ret && EAGAIN != errno) return -1;
		wr_len += ret;
		if((-1 == ret && EAGAIN == errno) || ret == buf_len) return wr_len;
	}
}

/*int send_read_contents(int epoll_fd, int fd, char* buf, int* wrptr, int* rdptr)
{
	int ret;
	static struct epoll_event ev;

	printf("wrptr: %d, rdptr: %d\n", *wrptr, *rdptr);

	ret = socket_to_send(fd, buf + *wrptr, *rdptr - *wrptr);
	if(-1 == ret) {
		printf("Fail to send, errno: %d, errmsg: %s\n", errno, strerror(errno));
		return -1;
	}

	if(*wrptr == *rdptr) {
		*wrptr = *rdptr = 0;
		ev.events = EPOLLOUT;
		ev.data.fd = fd;
		ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &ev);
	}
	if(*wrptr < *rdptr) {
		printf("Add fd to epoll\n");
		ev.events = EPOLLOUT | EPOLLIN;
		ev.data.fd = fd;
		if(-1 == epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev)) {
			printf("Fail to modify fd to EPOLLOUT AND EPOLLIN, errno: %d, errmsg: %s\n", errno, strerror(errno));
			return -1;
		}
	}

	return 0;
}*/

int open_server(int srv_port, int timeout)
{
	int ret;
	int listen_fd, epoll_fd;
	int fd_val;
	struct sockaddr_in srv_addr;
	struct epoll_event evs[MAX_EVENTS];
	struct epoll_event ev;
	char buf[MAX_EVENTS][READ_BUF_LEN];
	int rdptr[MAX_EVENTS] = {0};
	int wrptr[MAX_EVENTS] = {0};

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);

	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = INADDR_ANY;
	srv_addr.sin_port = htons(srv_port);

	ret = bind(listen_fd, (struct sockaddr*)&srv_addr, sizeof(struct sockaddr));
	if(0 != ret) {
		printf("Fail to bind, errno: %d, errmsg: %s\n", errno, strerror(errno));
		return -1;
	}

	ret = listen(listen_fd, 0);
	if(0 != ret) {
		printf("Fail to listen, errno: %d, errmsg: %s\n", errno, strerror(errno));
		return -1;
	}

	fd_val = fcntl(listen_fd, F_GETFL, 0);
	fcntl(listen_fd, F_SETFL, fd_val | O_NONBLOCK);

	epoll_fd = epoll_create(MAX_EVENTS);
	if(-1 == epoll_fd) {
		printf("Fail to create epoll fd, errno: %d, errmsg: %s\n", errno, strerror(errno));
		return -1;
	}
	
	ev.events = EPOLLIN;
	ev.data.fd = listen_fd;
	ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev);
	if(-1 == ret) {
		printf("Fail to epoll_ctl, errno: %d, errmsg: %s\n", errno, strerror(errno));
		return -1;
	}

	while(1) {
		printf("ready to wait\n");
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
			if(listen_fd == evs[i].data.fd) { //处理新连接
				printf("new connection\n");
				while(1) {
					int conn_fd = accept(listen_fd, NULL, NULL);
					if(0 < conn_fd) {
						fd_val = fcntl(conn_fd, F_GETFL, 0);
						fcntl(conn_fd, F_SETFL, fd_val | O_NONBLOCK);

						ev.events = EPOLLIN;
						ev.data.fd = conn_fd;
						ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev);
						if(-1 == ret) {
							printf("Fail to epoll_ctl, errno: %d, errmsg: %s\n", errno, strerror(errno));
							return -1;
						}
					}
					if(-1 == conn_fd &&
						EAGAIN != errno &&
						EWOULDBLOCK != errno &&
						ECONNABORTED != errno &&
						EPROTO != errno &&
						EINTR != errno) {
						printf("Fail to accept, errno: %d, errmsg: %s\n", errno, strerror(errno));
						return -1;
					}
					if(-1 == conn_fd && EAGAIN == errno) break;
				}
				continue;
			} //listen_fd
			else if(evs[i].events & EPOLLIN) { //处理读
				printf("new data to read\n");
				ret = socket_to_read(evs[i].data.fd, 
									 buf[evs[i].data.fd] + wrptr[evs[i].data.fd], 
									 sizeof(buf[evs[i].data.fd]) - rdptr[evs[i].data.fd]);
				if(-1 == ret) {
					printf("Fail to read, errno: %d, errmsg: %s\n", errno, strerror(errno));
					return -1;
				}
				if(0 == ret) {
					printf("Socket was closed by client\n");
					close(evs[i].data.fd);
					// ev.events = EPOLLIN;
					ev.data.fd = evs[i].data.fd;
					ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, evs[i].data.fd, &ev);
				}
				else {
					rdptr[evs[i].data.fd] += ret;
					ev.events = EPOLLOUT | EPOLLIN;
					ev.data.fd = evs[i].data.fd;
					if(-1 == epoll_ctl(epoll_fd, EPOLL_CTL_MOD, evs[i].data.fd, &ev)) {
						printf("Fail to modify fd to EPOLLOUT AND EPOLLIN, errno: %d, errmsg: %s\n", errno, strerror(errno));
						return -1;
					}
					/*send_read_contents(epoll_fd, evs[i].data.fd, buf[evs[i].data.fd], 
								       &(wrptr[evs[i].data.fd]), &(rdptr[evs[i].data.fd]));*/
				}
			} //EPOLL_IN
			else if(evs[i].events & EPOLLOUT) { //处理写
				printf("new data to write\n");
				printf("wrptr: %d, rdptr: %d\n", wrptr[evs[i].data.fd], rdptr[evs[i].data.fd]);
				ret = socket_to_send2(evs[i].data.fd,
									 buf[evs[i].data.fd] + wrptr[evs[i].data.fd],
									 rdptr[evs[i].data.fd] - wrptr[evs[i].data.fd]);
				printf("send ret: %d\n", ret);
				if(-1 == ret) {
					printf("Fail to send, errno: %d, errmsg: %s\n", errno, strerror(errno));
					return -1;
				}
				wrptr[evs[i].data.fd] += ret;
				if(wrptr[evs[i].data.fd] == rdptr[evs[i].data.fd]) {
					wrptr[evs[i].data.fd] = rdptr[evs[i].data.fd] = 0;
					ev.events = EPOLLIN;
					ev.data.fd = evs[i].data.fd;
					if(-1 == epoll_ctl(epoll_fd, EPOLL_CTL_MOD, evs[i].data.fd, &ev)) {
						printf("Fail to modify fd to EPOLLIN, errno: %d, errmsg: %s\n", errno, strerror(errno));
						return -1;
					}
					printf("------------\n");
				}

				/*send_read_contents(epoll_fd, evs[i].data.fd, buf[evs[i].data.fd], 
								   &wrptr[evs[i].data.fd], &rdptr[evs[i].data.fd]);*/
			} //EPOLL_OUT
		}
	} //while

	close(epoll_fd);
}

int main(int argc, char* argv[])
{
	if(3 > argc) {
		printf("Plz input 3 parameters at least\n");
		return 0;
	}

	open_server(atoi(argv[1]), atoi(argv[2]));

	return 0;
}
