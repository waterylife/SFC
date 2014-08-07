
#ifndef __ESF_NET_ACCEPT_H__
#define __ESF_NET_ACCEPT_H__

#include <string>
#include "esf_net_cconn.h"
#include "esf_net_epoll_flow.h"
#include "esf_net_incl.h"

//////////////////////////////////////////////////////////////////////////

namespace esf { namespace net {

	class CAcceptThread
	{
	public:
		static void* run(void* instance);
		
		CAcceptThread(CConnSet& cc, CEPollFlow& epoll);
		int open(const std::string& bind_ip, unsigned short bind_port);
		void real_run();

		CHandleConnect* _handle;
	private:
		CSocketTCP _socket;
		CConnSet& _cc;
		CEPollFlow& _epoll;
		unsigned _flow;
	};
}}

//////////////////////////////////////////////////////////////////////////
#endif
///:~
