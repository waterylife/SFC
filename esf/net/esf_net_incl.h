#ifndef __ESF_NET_INCL_H__
#define __ESF_NET_INCL_H__

class CHandleConnectImplBase
{
public:
	virtual int reply(unsigned flow, const char* data, unsigned data_len)=0;
	virtual ~CHandleConnectImplBase(){};
};
class CHandleConnect
{
public:
	void init(CHandleConnectImplBase *impl){_impl = impl;}
	int reply(unsigned flow, const char* data, unsigned data_len)
   	{return _impl->reply(flow, data, data_len);}
	virtual int handle_connect(unsigned flow){ return 0;}
	CHandleConnectImplBase *_impl;

	virtual ~CHandleConnect(){};
};

#endif

