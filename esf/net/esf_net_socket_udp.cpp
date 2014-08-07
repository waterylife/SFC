
#include "esf_net_socket_udp.h"

#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <arpa/inet.h>

using namespace std;

//////////////////////////////////////////////////////////////////////////

namespace esf
{
	namespace net
	{
		//
		//	return 0, on success
		//	return < 0, on -errno or unknown error
		//
		int CSocketUDP::create()
		{
			close();
			
			//	create and check
			errno = 0;
			int ret =::socket(AF_INET, SOCK_DGRAM, 0);
			if (ret < 0)
			{
				return errno ? -errno : ret;
			}
			else
			{
				_socket_fd = ret;
				return 0;
			}
		}
		
		void CSocketUDP::close()
		{
			if (_socket_fd != INVALID_SOCKET)
			{
				//::shutdown(_socket_fd, SHUT_RDWR);
				::close(_socket_fd);
				_socket_fd = INVALID_SOCKET;
			}
		}
		
		void CSocketUDP::detatch()
		{
			if (_socket_fd != INVALID_SOCKET)
			{
				_socket_fd = INVALID_SOCKET;
			}
		}
		
		//
		//	return 0, on success
		//	return < 0, on -errno or unknown error
		//
		
		int CSocketUDP::bind(const string &server_address, port_t port)
		{
			struct sockaddr_in address;

			bzero(&address, sizeof(address));
			address.sin_family = AF_INET;
			address.sin_addr.s_addr = inet_addr(server_address.c_str());
			address.sin_port = htons(port);

			errno = 0;
			int ret = ::bind(_socket_fd, (struct sockaddr *) &address, sizeof(address));
			return (ret < 0) ? (errno ? -errno : ret) : 0;
		}
		
		//
		//	bind on *:port
		//	return 0, on success
		//	return < 0, on -errno or unknown error
		//
		
		int CSocketUDP::bind_any(port_t port)
		{
			struct sockaddr_in address;

			bzero(&address, sizeof(address));
			address.sin_family = AF_INET;
			address.sin_addr.s_addr = htonl(INADDR_ANY);
			address.sin_port = htons(port);

			errno = 0;
			int ret = ::bind(_socket_fd, (struct sockaddr *) &address, sizeof(address));
			return (ret < 0) ? (errno ? -errno : ret) : 0;
		}
		
		int CSocketUDP::set_nonblock()
		{
			int val = fcntl(_socket_fd, F_GETFL, 0);
			
			if (val == -1)
				return errno ? -errno : val;
			
			if (val & O_NONBLOCK)
				return 0;
			
			int ret = fcntl(_socket_fd, F_SETFL, val | O_NONBLOCK | O_NDELAY);
			return (ret < 0) ? (errno ? -errno : ret) : 0;
		}

		int CSocketUDP::recvfrom(void * buf, size_t buf_len)
		{
			if (_socket_fd == INVALID_SOCKET)
			{
				return -1;
			}

			errno = 0;
			struct sockaddr_in address;
			socklen_t address_len= sizeof(address);

			int bytes =::recvfrom(	_socket_fd,
									buf,
									buf_len,
									0,
									(struct sockaddr *)&address,
									&address_len);
			if(bytes < 0)
			{
				return errno ? -errno : bytes;
			}
			else
			{
				return bytes;
			}
		}

		//
		//	return 0, on success
		//	return < 0, on -errno or unknown error
		//
		//	to receive data buffer: buf/buf_size
		//	received data length: received_len
		//
		int CSocketUDP::recvfrom(	void * buf,
									size_t buf_len,
									struct sockaddr_in * address,
									socklen_t * address_len)
		{
			if (_socket_fd == INVALID_SOCKET)
			{
				return -1;
			}

			errno = 0;
			int bytes =::recvfrom(	_socket_fd,
									buf,
									buf_len,
									0,
									(struct sockaddr *)address,
									address_len);
			if(bytes < 0)
			{
				return errno ? -errno : bytes;
			}
			else
			{
				return bytes;
			}
		}

		int CSocketUDP::sendto(	const void * buf,
					size_t buf_len,
					size_t* send_len,
					const string server_ip,
					int port)
		{
			if (_socket_fd == INVALID_SOCKET)
			{
				return -1;
			}

			struct sockaddr_in address;
			bzero(&address, sizeof(address));
			address.sin_family = AF_INET;
			address.sin_addr.s_addr = inet_addr(server_ip.c_str());
			address.sin_port = htons(port);
			

			errno = 0;
			int bytes = ::sendto(_socket_fd,
								buf,
								buf_len,
								0,
								(struct sockaddr *) &address,
								sizeof(address));
			
			if(bytes < 0)
			{
				return errno ? -errno : bytes;
			}
			else
			{
				*send_len = bytes;
				return 0;
			}		
		}

		//
		//	return 0, on success
		//	return < 0, on -errno or unknown error
		//
		//	to be sent data buffer: buf/buf_size
		//	done data length: sent_len
		//
		int CSocketUDP::sendto(	int sock_fd,
								const void * buf,
								size_t buf_len,
								size_t* send_len,
								struct sockaddr_in * address,
								socklen_t address_len)
		{
			errno = 0;
			int bytes = ::sendto(sock_fd,
								buf,
								buf_len,
								0,
								(struct sockaddr*)address,
								address_len);
			
			if(bytes < 0)
			{
				return errno ? -errno : bytes;
			}
			else
			{
				*send_len = bytes;
				return 0;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
