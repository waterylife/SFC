
#include <assert.h>
#include "esf_net_cconn.h"
#include "esf_net_socket_tcp.h"

using namespace esf::net;
using namespace std;

static const unsigned C_READ_BUFFER_SIZE = 8192;

//////////////////////////////////////////////////////////////////////////

CConnSet::CConnSet(check_complete func, CRCPool& rc_pool)
: _func(func), _rc_pool(rc_pool)
{}

CConnSet::~CConnSet()
{
	for(iterator it = _flow_2_data.begin()
		; it != _flow_2_data.end()
		; it = _flow_2_data.begin())
	{
		CloseFlow(it->first);
	}
}

//////////////////////////////////////////////////////////////////////////

int CConnSet::AddConn(int fd, unsigned flow)
{
	MutexGuard g(_mutex);
	assert(_flow_2_data.find(flow) == _flow_2_data.end());
	
	ConnCache* rc = _rc_pool.allocate();
	if (rc == NULL)
		return -1;

	rc->_access = time(0);
	rc->_flow = flow;
	rc->_fd = fd;
	_flow_2_data[flow] = rc;
	return 0;
}

int CConnSet::FD(unsigned flow)
{
	MutexGuard g(_mutex);
	iterator it = _flow_2_data.find(flow);
	if (it == _flow_2_data.end())
		return -1;

	return it->second->_fd;
}

//////////////////////////////////////////////////////////////////////////

//
//	return value:
//	0 on close, or closed yet
//	positive on recv some data
//	negative on get error and closed
//
//  ret
//  -E_NOT_FINDFD
// -E_NEED_CLOSE
// -EAGAIN
// recvd_len > 0, recv data length
int CConnSet::Recv(unsigned flow)
{
	//
	//	first, find the fd
	//

	MutexGuard g(_mutex);
	iterator it = _flow_2_data.find(flow);
	if(it == _flow_2_data.end())
		return -E_NOT_FINDFD;
	
	ConnCache* rc = it->second;
	int fd = rc->_fd;
	
	//
	//	second, read from socket
	//
	
	char sBuffer[C_READ_BUFFER_SIZE];
	CSocketTCP sock;
	sock.attach(fd);
	sock.detach();	//	declare not bound on fd
	size_t recvd_len = 0;
	int ret = sock.receive(sBuffer, (size_t)C_READ_BUFFER_SIZE, recvd_len);

	//
	//	third, on data received
	//
	
	if (ret == 0)
	{
		if (recvd_len > 0)
		{
			rc->_r.append(sBuffer, recvd_len);
			rc->_access = time(0);
			return recvd_len;
		}
		else	//	recvd_len == 0
		{
			return -E_NEED_CLOSE;
		}
	}
	
	//
	//	fifth, on error occur
	//

	else //	ret < 0
	{
		if (ret != -EAGAIN)
		{
			return -E_NEED_CLOSE;
		}

		return -EAGAIN;
	}
}

//////////////////////////////////////////////////////////////////////////
//强制发送
// ret
// -E_NOT_FINDFD
// 0, send failed or send not complete, add epollout
// 1, send complete
int CConnSet::SendForce(unsigned flow, const char* sData, size_t iDataLen)
{
	MutexGuard g(_mutex);
	iterator it = _flow_2_data.find(flow);
	if (it == _flow_2_data.end())
		return -E_NOT_FINDFD;	//	discard it plz;
	
	ConnCache* rc = it->second;
	int fd = rc->_fd;
	
	CSocketTCP sock;
	sock.attach(fd);
	sock.detach();
	
	size_t sent_len = 0;
	int ret = 0;
	
	ret = sock.send(sData, iDataLen, sent_len);
	
	if (ret < 0)
	{
		rc->_w.append(sData, iDataLen);
	}
	else if(ret == 0 && sent_len < iDataLen)
	{
		rc->_w.append(sData + sent_len, iDataLen - sent_len);
	}
	else if(ret == 0 && sent_len == iDataLen)
	{
		return 1;
	}
	
	return 0;
}

// ret
// 0 new data not send, only send cache data
// -E_NEED_CLOSE
// send_len >= 0, if send_len > 0 send new data length, send_len = 0 new data not send
// -E_NOT_FINDFD
int CConnSet::Send(unsigned flow, const char* sData, size_t iDataLen)
{
	//
	//	first, regular lock and check
	//

	MutexGuard g(_mutex);
	iterator it = _flow_2_data.find(flow);
	if (it == _flow_2_data.end())
		return -E_NOT_FINDFD;	//	discard it plz;
	
	ConnCache* rc = it->second;
	int fd = rc->_fd;
	
	CSocketTCP sock;
	sock.attach(fd);
	sock.detach();
	
	//
	//	second, if data in cache, send it first
	//
	
	size_t sent_len = 0;
	int ret = 0;
	if (rc->_w.data_len() != 0)
	{
		ret = sock.send(rc->_w.data(), rc->_w.data_len(), sent_len);
		
		//
		//	third, if cache not sent all, append data into w cache, return
		//
		if (ret == -EAGAIN || (ret == 0 && sent_len < rc->_w.data_len()))
		{
			rc->_w.skip(sent_len);
			rc->_w.append(sData, iDataLen);
			return 0;	//	nothing sent
		}
		else if (ret < 0)
		{
			return -E_NEED_CLOSE;
		}
	}

	//
	//	fourth, if cache sent all, send new data
	//
	
	rc->_w.skip(rc->_w.data_len());
	sent_len = 0;
	ret = sock.send(sData, iDataLen, sent_len);
	if (ret < 0 && ret != -EAGAIN && ret != -EINPROGRESS)
	{
		return -E_NEED_CLOSE;
	}

	//
	//	fifth, if new data still remain, append into w cache, return
	//

	if (sent_len < iDataLen )
	{
		rc->_w.append(sData + sent_len, iDataLen - sent_len);
	}
	
	return sent_len;
}

//
//	return value:
//	0 on all sent, or absent
//	positive on sent some thing, but still some remain
//	negative on error occur
//
// ret
// -E_NOT_FINDFD
// 0, send complete
// send_len > 0, send continue
// C_NEED_SEND > 0, send continue
// -E_NEED_CLOSE
int CConnSet::SendFromCache(unsigned flow)
{
	//
	//	first, regular lock and check
	//
	MutexGuard g(_mutex);
	
	iterator it = _flow_2_data.find(flow);
	if (it == _flow_2_data.end())
		return -E_NOT_FINDFD;	//	discard it plz;
	
	ConnCache* rc = it->second;
	int fd = rc->_fd;
	if (rc->_w.data_len() == 0)
		return 0;

	CSocketTCP sock;
	sock.attach(fd);
	sock.detach();
	
	//
	//	second, if data in cache, send it first
	//
	
	size_t sent_len = 0;
	int ret = sock.send(rc->_w.data(), rc->_w.data_len(), sent_len);
	
	//
	//	third, if cache not sent all, append data into w cache, return
	//
    
	if(ret == 0)
	{
	    if(sent_len == 0)
        {
            return C_NEED_SEND;
        }
        else
        {
    		if(sent_len == rc->_w.data_len())
    		{
    			rc->_w.skip(sent_len);
    			return 0;
    		}
    		else
    		{
    			rc->_w.skip(sent_len);
    			return sent_len;	//	nothing sent
    		}
    	}
	}
	else if (ret == -EAGAIN)
	{
		// 继续发
		return C_NEED_SEND;
	}
	else
	{
		return -E_NEED_CLOSE;
	}
}

//////////////////////////////////////////////////////////////////////////
//ret
// -E_NOT_FINDFD
// -E_NEED_CLOSE
// 0, if data_len = 0, recv continue, if data_len > 0, recv complete
int CConnSet::GetMessage(unsigned flow, void* buf, unsigned buf_size, unsigned& data_len)
{
	MutexGuard g(_mutex);
	iterator it = _flow_2_data.find(flow);
	if (it == _flow_2_data.end())
		return -E_NOT_FINDFD;

	ConnCache* cc = it->second;
	int ret = _func(cc->_r.data(), cc->_r.data_len());
	if (ret < 0)
	{
		return -E_NEED_CLOSE;
	}
	else if (ret == 0)
	{
		data_len = 0;
		return 0;
	}
	else
	{
		if ((unsigned)ret > buf_size)
		{
			return -E_NEED_CLOSE;
		}
		else
		{
			memcpy(buf, cc->_r.data(), ret);
			cc->_r.skip(ret);
			data_len = ret;
			return 0;
		}
	}
}

//////////////////////////////////////////////////////////////////////////

int CConnSet::CloseFlow(unsigned flow)
{
	MutexGuard g(_mutex);
	//	find in map
	iterator it = _flow_2_data.find(flow);
	if (it == _flow_2_data.end())
		return -1;
	
	//	erase from map
	int fd = it->second->_fd;
	_rc_pool.recycle(it->second);
	close(fd);
	_flow_2_data.erase(it);
	return 0;
}

void CConnSet::CheckTimeout(time_t access_deadline, list<unsigned>& timeout_flow)
{
	MutexGuard g(_mutex);
	timeout_flow.clear();

	for(iterator it = _flow_2_data.begin(); it != _flow_2_data.end(); it++)
	{
		if (it->second->_access < access_deadline)
		{
			timeout_flow.push_back(it->second->_flow);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
///:~
