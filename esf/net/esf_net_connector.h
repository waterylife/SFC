
#ifndef __ESF_NET_CONNECTOR_H__
#define __ESF_NET_CONNECTOR_H__

#include <string>
#include <assert.h>

#include "esf_net_driver.h"
#include "esf_net_network.h"
#include "esf_net_simple_sock.h"

namespace esf{namespace net{

//////////////////////////////////////////////////////////////////////////

class ConnectorImp : public Connector
{
public:
	ConnectorImp(ptr<ConnAllocator> allocator) : Connector(allocator){}
	virtual ~ConnectorImp(){}
	
	virtual bool SetSAP(unsigned short iPort, const std::string& sHost)
	{
		_port = iPort;
		_host = sHost;
		return true;
	}
	
	virtual ptr<Connection> AllocateConn()
	{
		ptr<Connection> conn = _allocator->AllocateConn();
		net::CSimpleSocketTcp tmp;
		tmp.create();
		tmp.connect(_host, _port);
		conn->AttachFD(tmp.fd());
		tmp.attach(-1);
		return conn;
	}

protected:
	unsigned short _port;
	std::string _host;
};

//////////////////////////////////////////////////////////////////////////
}}	//	namespace 
#endif//
///:~
