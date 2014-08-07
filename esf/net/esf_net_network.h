
#ifndef __ESF_NET_NETWORK_H__
#define __ESF_NET_NETWORK_H__

#include <string>

#include "esf_sys_buffer.h"
#include "esf_sys_heap_guardian.h"

using namespace esf::sys;

namespace esf{namespace net{

//////////////////////////////////////////////////////////////////////////

//
//	connection: an established server-client contract
//	including: session holding, presentation convertion, and application processing
//

class Connection
{
public:
	virtual ~Connection(){}
	
	virtual CBuffer& Recv() = 0;
	virtual int Send(const char* sSendBuf, size_t iSendLen) = 0;
	virtual bool SendAvailable() = 0;
	virtual int SendCache() = 0;
	
	virtual bool IsClosed() = 0;
	virtual int FD() = 0;
	virtual void AttachFD(int) = 0;
};

//
//	connection allocator, memory allocate policy, connection establish policy
//

class ConnAllocator
{
public:
	virtual ~ConnAllocator(){}
	virtual ptr<Connection> AllocateConn() = 0;
};

//////////////////////////////////////////////////////////////////////////

//
//	decorator, factory method, add network policy, not memory allocat policy
//

class ConnFactoryDrr : public ConnAllocator
{
public:
	ConnFactoryDrr(ptr<ConnAllocator> allocator):_allocator(allocator){}
	virtual ~ConnFactoryDrr(){}
	virtual bool SetSAP(unsigned short iPort, const std::string& sHost) = 0;
	virtual ptr<Connection> AllocateConn() = 0;
	
protected:
	ptr<ConnAllocator> _allocator;
};

class CAcceptor : public ConnFactoryDrr
{
public:
	CAcceptor(ptr<ConnAllocator> allocator):ConnFactoryDrr(allocator){}
	virtual ~CAcceptor(){}
	virtual bool SetSAP(unsigned short iPort, const std::string& sHost) = 0;
	virtual ptr<Connection> AllocateConn() = 0;
};

class Connector : public ConnFactoryDrr
{
public:
	Connector(ptr<ConnAllocator> allocator):ConnFactoryDrr(allocator){}
	virtual ~Connector(){}
	virtual bool SetSAP(unsigned short iPort, const std::string& sHost) = 0;
	virtual ptr<Connection> AllocateConn() = 0;
};

//////////////////////////////////////////////////////////////////////////

class nw_ex:public bt_ex{public: nw_ex(const std::string& s):bt_ex(s){}};

class conn_closed : public nw_ex {public: conn_closed(const std::string& s) : nw_ex(s){}};

//////////////////////////////////////////////////////////////////////////
}}	//	namespace

#endif
///:~
