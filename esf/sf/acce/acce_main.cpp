
#include "esf_net_epoll_flow.h"
#include "esf_net_accept_t.h"
#include "esf_net_incl.h"
#include "esf_net_cconn.h"
#include "esf_ipc_open_mq.h"
#include "esf_ipc_thread_sync.h"
#include "esf_sys_str.h"
#include "esf_sys_so.h"
#include "esf_sys_daemon.h"
#include "esf_sys_config_file.h"

#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>

using namespace esf::sys;
using namespace esf::net;
using namespace esf::ipc;
using namespace std;

//////////////////////////////////////////////////////////////////////////
class CLoadGrid
{
public:
	enum LoadRet
	{
		LR_NORMAL =0,
		LR_FULL = 1
	};
	CLoadGrid(unsigned grid_count
		, unsigned grid_distant
		, unsigned max_req_water_mark
		, timeval start_time)
	{
		_curr_grid = 0;
		_req_water_mark = 0;
		_start_time = start_time;
		_grid_count = grid_count;
		_grid_distant = grid_distant;
		_max_req_water_mark  = max_req_water_mark;
		_grid_array = new unsigned[_grid_count];
		memset(_grid_array, 0, _grid_count*sizeof(unsigned));

		_virtual_last_grid = 0;
	}
	~CLoadGrid()
	{ 
		if (_grid_array!=NULL) delete []_grid_array;
	}

	int check_load(timeval cur_time)
	{
		double time_used = (cur_time.tv_sec -_start_time.tv_sec )*1000000.0
							+ cur_time.tv_usec -_start_time.tv_usec;
		
		if (time_used <0)
		{
			// something error, reset!
			_start_time = cur_time;
			_curr_grid = 0;
			_req_water_mark = 0;
			memset(_grid_array, 0, _grid_count*sizeof(unsigned));
			_virtual_last_grid = 0;
			return 0;
		}

		_virtual_curr_grid = (unsigned(time_used/1000.0))/_grid_distant;
		_curr_grid = _virtual_curr_grid % _grid_count;

		if(_virtual_curr_grid != _virtual_last_grid)
		{
			//两次检测之间跨越的格子数，之间的需要清零
			unsigned grid_spand = _virtual_curr_grid - _virtual_last_grid;
			_virtual_last_grid = _virtual_curr_grid;
			
			grid_spand = (grid_spand > _grid_count) ? _grid_count : grid_spand;
			for(unsigned i=0; i< grid_spand; i++)
			{
				_req_water_mark -= _grid_array[(_curr_grid - i + _grid_count)%_grid_count];
				_grid_array[(_curr_grid - i + _grid_count)%_grid_count] = 0;
			}
			

			if (_req_water_mark >= _max_req_water_mark)
			{
				printf("LR_FULL.\n");
				return LR_FULL;
			}
			
		}

		_grid_array[_curr_grid] ++;
		_req_water_mark ++;
        
		return LR_NORMAL;
	}

	void fetch_load(unsigned& milliseconds, unsigned& req_cnt)
	{
		milliseconds = _grid_distant*_grid_count;
		req_cnt = _req_water_mark;
	}

	unsigned _grid_count;
	unsigned _grid_distant;
	unsigned _max_req_water_mark;
	unsigned* _grid_array;
private:
	unsigned _req_water_mark;
	unsigned _curr_grid;

	unsigned _virtual_last_grid;
         unsigned _virtual_curr_grid;
	timeval _start_time;
};

class CHandleConnectImpl: public CHandleConnectImplBase
{
public:
	CHandleConnectImpl(CFifoSyncMQ* rsp_mq
			,CMutex* mutex)
	{
		_rsp_mq = rsp_mq;
		_mutex = mutex;
	}
	
	int reply(unsigned flow, const char* data, unsigned data_len)
	{
		MutexGuard g(*_mutex);
		return _rsp_mq->enqueue(data, data_len, flow);
	}

protected:
	CFifoSyncMQ* _rsp_mq;
	CMutex* _mutex;
		
};

//////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		cerr<<argv[0]<<" + conf_file"<<endl;
		return 0;
	}

       InitDaemon();
	
	CFileConfig& page = * new CFileConfig();
	page.Init(argv[1]);
	
	//
	//
	//
	rlimit rlim;
	rlim.rlim_cur = 120000;
	rlim.rlim_max = 120000;
	setrlimit(RLIMIT_NOFILE, &rlim);
	
	//
	//	open mq
	//
	const string req_mq_conf_path = page["root\\req_mq_conf"];
	CFifoSyncMQ* req_mq = GetMQ(req_mq_conf_path);
	
	const string rsp_mq_conf_path = page["root\\rsp_mq_conf"];
	CFifoSyncMQ* rsp_mq = GetMQ(rsp_mq_conf_path);

	//
	// net load 
	//
	unsigned grid_count = from_str<unsigned>(page["root\\grid_num"]);
	unsigned grid_distant = from_str<unsigned>(page["root\\grid_distant"]);
	unsigned req_water_mark = from_str<unsigned>(page["root\\req_water_mark"]);
	timeval time_now;
	gettimeofday(&time_now,NULL);
	CLoadGrid* pload_grid = new CLoadGrid(grid_count,grid_distant,req_water_mark,time_now);
	//
	//	load check_complete
	//
	const string comeplete_so_file = page["root\\complete_so_file"];
	const string comeplete_func_name = page["root\\complete_func_name"];
	CSOFile so_file;
	int ret = so_file.open(comeplete_so_file);
	assert(ret == 0);
	check_complete cc_func = (check_complete) so_file.get_func(comeplete_func_name);

	typedef CHandleConnect* (*app_constructor)();
	CHandleConnect* handle;
	app_constructor constructor = (app_constructor) so_file.get_func("create_handle");
	if (constructor == NULL)
	{
		handle = new CHandleConnect();
	}else
	{
		handle = constructor();
	}

	CMutex mq_mutex;
	CHandleConnectImpl *impl = new CHandleConnectImpl(rsp_mq,&mq_mutex);
	handle->init(impl);
	//
	//	raw cache pool
	//
	CRCPool* rc_pool = new CRCPool();
	unsigned max_conn = from_str<unsigned>(page["root\\max_conn"]);
	CMemPool* m_poll = new CMemPool();
	rc_pool->pre_allocate(max_conn, *m_poll);

	//
	//	cached conn set
	//
	CConnSet* cc = new CConnSet(cc_func, *rc_pool);
	
	//
	//	epoll
	//
	CEPollFlow* epoll = new CEPollFlow();
	epoll->create(120000);	//	non-sense
	epoll->add(rsp_mq->fd(), 0, EPOLLIN);

	// add mq fd to epoll


	//
	//	open acceptor
	//
	const string bind_ip = page["root\\bind_ip"];
	const unsigned short bind_port = from_str<unsigned short>(page["root\\bind_port"]);
	CAcceptThread* accept_t = new CAcceptThread(*cc, *epoll);
	accept_t->_handle = handle;
	ret = accept_t->open(bind_ip, bind_port);
	assert(ret == 0);
	pthread_t p;
	pthread_create(&p, NULL, CAcceptThread::run, accept_t);
	
	time_t last_time = time(0);
	//
	//	for(;;)
	//
	for(;;)
	{
		//
		//
		//
		time_t cur_time = time(0);
		static const unsigned C_EXPIRED_TIME = 180;
		if ((cur_time - last_time) > 60)
		{
			list<unsigned> timeout_list;
			cc->CheckTimeout(cur_time - C_EXPIRED_TIME, timeout_list);
			for(list<unsigned>::iterator lit = timeout_list.begin()
					; lit != timeout_list.end()
					; lit++)
			{
				ret = cc->CloseFlow(*lit);
				if (ret)
				{
					cerr << "close flow failed, error: "<<ret<<"\t"<<"flow: "<<*lit<<endl;
				}
			}
	
			last_time = cur_time;
		}
        
		static const unsigned C_TMP_BUF_LEN = (1<<27);                
		static char tmp_buffer[C_TMP_BUF_LEN];                       

		CEPollFlowResult result = epoll->wait(1);
		for(CEPollFlowResult::iterator it = result.begin()
			; it != result.end()
			; it++)
		{
			if (it.fd() != rsp_mq->fd())
			{
				unsigned flow = it.flow();
				if (!(it->events & (EPOLLIN | EPOLLOUT))) 
				{
					cc->CloseFlow(flow);
				}
				if (it->events & EPOLLOUT)
				{
					// -E_NOT_FINDFD
					// 0, send complete
					// send_len > 0, send continue
					// C_NEED_SEND > 0, send continue
					// -E_NEED_CLOSE
					int ret = cc->SendFromCache(flow);

					if (ret == -E_NEED_CLOSE)
					{
						cc->CloseFlow(flow);
						continue;
					}
					else if (ret == 0)
					{
						//缓存发送完毕,去除EPOLLOUT
						int fd = cc->FD(flow);
						epoll->modify(fd, flow, EPOLLIN | EPOLLET);
					}
					else
					{
						//缓存发送未完毕,EPOLLOUT继续存在
						//如果E_NOT_FINDFD，epoll已经自动的将该fd删除了,无需处理
						assert(ret > 0 || ret == -E_NOT_FINDFD);
					}
				}

				if (!(it->events & EPOLLIN))
				{
					continue;
				}

				for(unsigned i = 0; i < 10000; i++)
				{
					//  -E_NOT_FINDFD
					// -E_NEED_CLOSE
					// -EAGAIN
					// recvd_len > 0, recv data length
					ret = cc->Recv(flow);

					if (ret == -E_NEED_CLOSE)
					{
						cc->CloseFlow(flow);
						break;
					}
					else if(ret == -E_NOT_FINDFD || ret == -EAGAIN)
					{
						//如果是EAGAIN，跳出循环，下一轮epoll激活后再Recv
						//如果或者E_NOT_FINDFD，epoll已经自动的将该fd删除了,无需处理
						break;
					}
					else
					{
						//确定收到了数据
						assert(ret > 0);
					}
				}

				unsigned data_len = 0;
				
				// add while{...}
				while (true)
				{
					// -E_NOT_FINDFD
					// -E_NEED_CLOSE
					// 0, if data_len = 0, recv continue, if data_len > 0, recv complete
					ret = cc->GetMessage(flow, tmp_buffer, sizeof(tmp_buffer), data_len);
					if (ret == 0)
					{
						if (data_len > 0)
						{
							gettimeofday(&time_now,NULL);
							ret = pload_grid->check_load(time_now);
							
							
							if(data_len == 10)
							{
								unsigned milliseconds;
								unsigned req_cnt;
								pload_grid->fetch_load(milliseconds, req_cnt);
								*(unsigned*)(tmp_buffer+2) = milliseconds;
								*(unsigned*)(tmp_buffer+6) = req_cnt;
								//enqueue to rsp mq directly
								MutexGuard g(mq_mutex);
								ret = rsp_mq->enqueue(tmp_buffer, 10, flow);
								break;
							}

							if (ret == CLoadGrid::LR_FULL)
							{
								cc->CloseFlow(flow);
								break;
							}

							ret = req_mq->enqueue(tmp_buffer, data_len, flow);
							if (ret)
							{
								cc->CloseFlow(flow);
								break;
							}
						}
						else if (data_len == 0)
						{
							break;
						}
					}
					else if (ret == -E_NEED_CLOSE)
					{
						cc->CloseFlow(flow);
						break;
					}
					else 
					{
						assert(ret == -E_NOT_FINDFD);
						break;
					}
				}

			}
			else // mq event
			{
				//
				//	rsp queue
				//

				for(unsigned i = 0; i < 10000; i++)
				{                                     
					unsigned data_len = 0;
					unsigned flow = UINT_MAX;
					{
						MutexGuard g(mq_mutex);
						ret = 
							rsp_mq->dequeue(tmp_buffer, sizeof(tmp_buffer), data_len, flow);
					}
					assert(ret == 0);
					if (data_len == 0 && flow == UINT_MAX)
						break;			
					if (data_len == 0 && flow != UINT_MAX)
						cc->CloseFlow(flow);

					// 0 new data not send, only send cache data
					// -E_NEED_CLOSE
					// send_len > 0 send new data length
					// -E_NOT_FINDFD
					ret = cc->Send(flow, tmp_buffer, data_len);
					if(ret < 0)
					{
						if (ret == -E_NEED_CLOSE)
						{
							cc->CloseFlow(flow);
						}
						else
							assert(ret == -E_NOT_FINDFD);
					}
					else
					{
						if((unsigned)ret < data_len)
						{
							//未发送完，继续监视EPOLLOUT
							int fd = cc->FD(flow);
							epoll->modify(fd, flow, EPOLLET | EPOLLIN | EPOLLOUT | EPOLLERR);
						}
						else
						{
							//发送完毕
							assert((unsigned)ret == data_len);
						}
					}
				}
			}
		}
	}
}		

//////////////////////////////////////////////////////////////////////////
///:~
