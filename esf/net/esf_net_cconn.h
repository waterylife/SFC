
#ifndef __ESF_NET_CCONN_H__
#define __ESF_NET_CCONN_H__

#include <time.h>
#include <ext/hash_map>
#include <list>

#include "esf_net_socket_tcp.h"
#include "esf_net_raw_cache.h"
#include "esf_net_mem_pool.h"
#include "esf_ipc_thread_sync.h"

//////////////////////////////////////////////////////////////////////////

//
//	1 ʹ��hash_map����map
//	2 ����һ�����ӽ�����new�Ĵ�����late allocate
//	3 cache������Դ��
//	4 ������Դ�أ�Ԥ������Դ����̬��չ�ͻ��ա�
//

#define  E_NEED_CLOSE  39999
#define  E_NOT_FINDFD  39998
#define  C_NEED_SEND  29999

namespace esf{ namespace net
{
	typedef int (*check_complete)(const void* data, unsigned data_len);
	
	class CConnSet
	{
	public:
		CConnSet(check_complete func, CRCPool& rc_pool);
		~CConnSet();
		
		//	about the fd
		int AddConn(int fd, unsigned flow);
		int FD(unsigned flow);

		//	operation on the flow
		int Recv(unsigned flow);
		int GetMessage(unsigned flow, void* buf, unsigned buf_size, unsigned& data_len);
		int SendForce(unsigned flow, const char* sData, size_t iDataLen);
		int Send(unsigned flow, const char* sData, size_t iDataLen);
		int SendFromCache(unsigned flow);
		int CloseFlow(unsigned flow);
		
		void CheckTimeout(time_t access_deadline, std::list<unsigned>& timeout_flow);
		
	private:
		typedef std::map<unsigned, ConnCache*>::iterator iterator;
		std::map<unsigned, ConnCache*> _flow_2_data;	//	the ugly duplex mapping!!
		
		int _iConnType;
		check_complete _func;
		
		CRCPool& _rc_pool;
		CMutex _mutex;
	};
}}

//////////////////////////////////////////////////////////////////////////
#endif
///:~
