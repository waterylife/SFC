
#ifndef __ESF_NET_SOCKET_UDP_H__
#define __ESF_NET_SOCKET_UDP_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <string>

//////////////////////////////////////////////////////////////////////////
//	open for extension, open for modification
//////////////////////////////////////////////////////////////////////////

//
//	return value 的规则，
//	如果操作成功，返回 0，
//	如果操作失败，返回一个负数，代表一个errno的相反数或者-1不代表任何意义
//	没有返回正数的情况，所有的输出数据，都以输出参数的形式。
//	例如accept，是否成功在返回值中表示，accept产生的新fd，以参数形式传出
//	再如recv，如果失败，以返回值表示，如果是负数，表示errno，比如-EAGAIN
//	如果在recv中收到对方关闭的消息，recv操作仍然返回0，但是收到的数据长度是0
//

namespace esf
{ 
	namespace net
	{
		typedef in_addr_t ip_4byte_t;	//	unsigned int
		typedef uint16_t port_t;		//	unsigned short
		
		class CSocketUDP
		{
			// Construction
		public:
			CSocketUDP():_socket_fd(INVALID_SOCKET)
			{
			}
			~CSocketUDP()
			{
				close();
			}
			int create();
			
			int fd() const {return _socket_fd;};
			bool socket_is_ok() const {return (_socket_fd != INVALID_SOCKET);}
			void close();

			void detatch();
			
			int bind(const std::string& server_address, port_t port);
			int bind_any(port_t port);

			int set_nonblock();

			int recvfrom(void * buf, size_t buf_len);

			int recvfrom(	void * buf,
							size_t buf_len,
							struct sockaddr_in * address,
							socklen_t * address_len);

			int sendto(	const void * buf,
						size_t buf_len,
						size_t* send_len,
						const std::string server_ip,
						int port);

			static int sendto(
						int sock_fd,
						const void * buf,
						size_t buf_len,
						size_t* send_len,
						struct sockaddr_in * address,
						socklen_t address_len);
			
		private:
			int _socket_fd;
			static const int INVALID_SOCKET = -1;
		};
	}
}

//////////////////////////////////////////////////////////////////////////
#endif


