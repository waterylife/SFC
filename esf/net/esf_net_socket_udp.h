
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
//	return value �Ĺ���
//	��������ɹ������� 0��
//	�������ʧ�ܣ�����һ������������һ��errno���෴������-1�������κ�����
//	û�з�����������������е�������ݣ����������������ʽ��
//	����accept���Ƿ�ɹ��ڷ���ֵ�б�ʾ��accept��������fd���Բ�����ʽ����
//	����recv�����ʧ�ܣ��Է���ֵ��ʾ������Ǹ�������ʾerrno������-EAGAIN
//	�����recv���յ��Է��رյ���Ϣ��recv������Ȼ����0�������յ������ݳ�����0
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


