
#ifndef __ESF_NET_SOCKET_TCP_H__
#define __ESF_NET_SOCKET_TCP_H__

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

namespace esf { namespace net {

	typedef in_addr_t ip_4byte_t;	//	unsigned int
	typedef uint16_t port_t;		//	unsigned short
		
	class CSocketTCP
	{
		// Construction
		public:
			CSocketTCP():_socket_fd(INVALID_SOCKET), _close_protect(true){}
			~CSocketTCP(){if (_close_protect)close();}
			int create();
			
			int fd() const {return _socket_fd;};
			bool socket_is_ok() const {return (_socket_fd != INVALID_SOCKET);}
			void close();

			// Operations
			int bind(const std::string& server_address, port_t port);
			int bind_any(port_t port);
			int listen();
			int accept(CSocketTCP& client_socket);

			int connect(ip_4byte_t addr, port_t port);
			int connect(const std::string & addr, port_t port);
			
			int receive	(void * buf, size_t buf_size, size_t& received_len, int flag = 0);
			int send	(const void * buf, size_t buf_size, size_t& sent_len, int flag = 0);

			int shutdown();
			int set_nonblock();
			int set_reuseaddr();

			void detach(){_close_protect = false;}
			void attach(int fd){if (_close_protect){close();_close_protect = true;} _socket_fd = fd;}
			
			int get_peer_name(ip_4byte_t& peer_address, port_t& peer_port);
			int get_sock_name(ip_4byte_t& socket_address, port_t& socket_port);
			
			int get_peer_name(std::string& peer_address, port_t& peer_port);
			int get_sock_name(std::string& socket_address, port_t& socket_port);
			
		private:
			CSocketTCP(const CSocketTCP& sock); // no implementation
			void operator = (const CSocketTCP& sock); // no implementation

			int _socket_fd;
			bool _close_protect;
			static const int INVALID_SOCKET = -1;
	};	
}}

//////////////////////////////////////////////////////////////////////////
#endif
///:~
