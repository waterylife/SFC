#include "esf_net_cconn.h"
#include "esf_net_epoll_flow.h"
#include "esf_net_accept_t.h"
#include "esf_ipc_open_mq.h"
#include "esf_sys_str.h"
#include "esf_sys_so.h"
#include "esf_sys_daemon.h"
#include "esf_sys_config_file.h"

#include "esf_net_conn_map.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>

using namespace esf::sys;
using namespace esf::net;
using namespace esf::ipc;
using namespace std;

/*
conn根据ipc中提供的flow唯一确定一个连接，不同的flow产生不同的
连接(即使ip port一样),使用者有义务维护这个flow,或用disconnect命令关闭这
个flow

如果到同一个ip只有一个连接的话,flow=ip即可
当连接数超过max_conn时进行超时清理，超时时间180秒
*/

static const unsigned C_TMP_BUF_LEN = 128*1024*1024;

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		cerr<<argv[0]<<" + conf_file"<<endl;
		return 0;
	}

	InitDaemon();
	
	SAY("debug version...\n");
	
		   
	CFileConfig& page = * new CFileConfig();
	page.Init(argv[1]);
	
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
	//	load check_complete
	//
	typedef int (*protocol_convert)(const void* , unsigned, void* , unsigned &);
	
	const string comeplete_so_file = page["root\\complete_so_file"];
	const string comeplete_func_name = page["root\\complete_func_name"];
	
	CSOFile so_file;
	int ret = so_file.open(comeplete_so_file);
	assert(ret == 0);
	check_complete cc_func = (check_complete) so_file.get_func(comeplete_func_name);
	protocol_convert ipc2net_func = (protocol_convert) so_file.get_func("protocol_ipc2net");
	protocol_convert net2ipc_func = (protocol_convert) so_file.get_func("protocol_net2ipc");
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

	//
	//	connection map
	//
	ConnMap& conn_map = * new ConnMap(max_conn);

	//
	//	for(;;)
	//
	char *recv_buffer = new char[C_TMP_BUF_LEN];
	char *temp_buffer = new char[C_TMP_BUF_LEN];	
	unsigned data_len = 0;
	
	for(;;)
	{
		//超时处理
		std::vector<unsigned> vec_key;
		conn_map.GetExpire(vec_key);
		for(int i=0;i<(int)vec_key.size();i++)
		{
			SAY("flow %d expired ,delete.\n",vec_key[i]);
			conn_map.delconn(vec_key[i]);
			cc->CloseFlow(vec_key[i]);
		}
		
		//处理网络包
		CEPollFlowResult result = epoll->wait(1);
		for(CEPollFlowResult::iterator it = result.begin()
			; it != result.end()
			; it++)
		{
			unsigned flow = it.flow();
			int currfd = cc->FD(flow);
			
			stconn* _pstconn = conn_map.get_conn_frm_flow(flow);
			if (!_pstconn)
				continue;
			_pstconn->lastavtive = time(0);
			
			ConnHeader conn_hd;
			conn_hd._ip = _pstconn->_ip;
			conn_hd._port = _pstconn->_port;
			
			//断连
			if (!(it->events & (EPOLLOUT|EPOLLIN)))
			{
				conn_map.delconn(flow);	
				cc->CloseFlow(flow);
				SAY("disconnect from flow=%d,%d:%d\n",flow,_pstconn->_ip,_pstconn->_port);			
				continue;
			}

			//连接成功，或可发送，conn一定只是缓存里可发送的数据
			if (it->events & EPOLLOUT)
			{	
				//主动连接,向上层汇报
				if (_pstconn->connstatus == status_connecting)
				{
					SAY("EPOLLOUT: flow %d connecting success, response connect_ok to client.\n",flow);
					_pstconn->connstatus = status_connected;

					conn_hd._reqrsp_type= conn_rsp_connect_ok;
					memcpy(recv_buffer,&conn_hd,CONN_HEADER_LEN);
					rsp_mq->enqueue(recv_buffer, CONN_HEADER_LEN, flow);	

					//取消EPOLLOUT监视
					epoll->modify(currfd, flow, EPOLLIN | EPOLLERR|EPOLLHUP|EPOLLET);
				}
				//直接发送连接不上报, connecting的状态当connected来处理
				else if ((_pstconn->connstatus == status_send_connecting)
				    || (_pstconn->connstatus == status_connected))
				{
					SAY("EPOLLOUT: flow %d send_connect success,  no response connect_ok to client.\n",flow);

					//缓存里的发一把
					// -E_NOT_FINDFD
					// 0, send complete
					// send_len > 0, send continue
					// C_NEED_SEND > 0, send continue
					// -E_NEED_CLOSE
					ret = cc->SendFromCache(flow);
					
                    if (ret == -E_NEED_CLOSE)
					{
						SAY("EPOLLOUT: SendFromCache failed, flow=%d,disconnect..\n",flow);
						cc->CloseFlow(flow);
						conn_map.delconn(flow);
						continue;
					}
                    else if(ret == 0)
                    {
                        //缓存发送完毕,去除EPOLLOUT
                        epoll->modify(currfd, flow, EPOLLIN | EPOLLERR|EPOLLHUP|EPOLLET);
                    }
                    else
                    {
                        //缓存发送未完毕,EPOLLOUT继续存在
                        //如果E_NOT_FINDFD，epoll已经自动的将该fd删除了,无需处理
                        assert(ret > 0 || ret == -E_NOT_FINDFD);
                    }					
				}
				continue;
			}

			if (!(it->events & EPOLLIN))
			{
				continue;
			}

			//接收消息
			//int connnect_failed=0;
			int socket_need_close = 0;
			for(unsigned i = 0; i < 10000; i++)
			{
				//  -E_NOT_FINDFD
				// -E_NEED_CLOSE
				// -EAGAIN
				// recvd_len > 0, recv data length
				ret = cc->Recv(flow);
				SAY("cc->Recv %d bytes flow=%d\n",ret,flow);			
				if (ret == -E_NEED_CLOSE)
				{	
					socket_need_close = 1;
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

			//if (connnect_failed)
			//	continue;

			//读出
			do
			{
				data_len = 0;
				// -E_NOT_FINDFD
				// -E_NEED_CLOSE
				// 0, if data_len = 0, recv continue, if data_len > 0, recv complete
				ret = cc->GetMessage(flow, recv_buffer+CONN_HEADER_LEN, C_TMP_BUF_LEN-CONN_HEADER_LEN, data_len);			
				SAY("GetMessage data_len = %d, in flow %d\n",data_len,flow);
				if (ret == 0)
				{
					if (data_len > 0)
					{		
						conn_hd._reqrsp_type= conn_rsp_data;
						
						//转码
						unsigned conver_len = C_TMP_BUF_LEN;
						ret = net2ipc_func(recv_buffer+CONN_HEADER_LEN,data_len,temp_buffer,conver_len);
						if (ret == -1)
						{
							SAY("net2ipc_func failed ret =%d\n",ret);
							break;
						}
						else if(ret == 1)
						{
							//未改变
							SAY("net2ipc_func not convert.\n");
						}
						else
						{
							data_len = conver_len;
							memcpy(recv_buffer+CONN_HEADER_LEN,temp_buffer,conver_len);
						}						
						memcpy(recv_buffer,&conn_hd,CONN_HEADER_LEN);
						rsp_mq->enqueue(recv_buffer, data_len+CONN_HEADER_LEN, flow);
					}
				}
				else if(ret == -E_NEED_CLOSE)
				{
					SAY("get message from flow %d failed.\n",flow);	
					conn_map.delconn(flow);	
					cc->CloseFlow(flow);
					break;
				}
				else
				{
					assert(ret == -E_NOT_FINDFD);
					break;
				}
			}while(data_len > 0);
			

			if(socket_need_close)
			{
				SAY("%d:%d disconnect me, notify client.\n",_pstconn->_ip,_pstconn->_port);

				conn_hd._reqrsp_type= conn_rsp_disconnected;						
				memcpy(recv_buffer,&conn_hd,CONN_HEADER_LEN);
				rsp_mq->enqueue(recv_buffer, CONN_HEADER_LEN, flow);

				conn_map.delconn(flow);	
				cc->CloseFlow(flow);					
			}

		}

		//
		//rsp queue
		//此处管道fd没有加入epoll，单独处理
		//处理来自请求管道的消息
		for(unsigned i = 0; i < 10000; i++)
		{
			int ret = 0 ;
			data_len = 0;
			unsigned queue_flow = 0;
			ret = req_mq->dequeue(recv_buffer, C_TMP_BUF_LEN, data_len, queue_flow);
			if (data_len == 0)
			{
				break;
			}
			
			//转码
			unsigned conver_len = C_TMP_BUF_LEN-CONN_HEADER_LEN;
			ret = ipc2net_func(recv_buffer+CONN_HEADER_LEN,data_len-CONN_HEADER_LEN,temp_buffer,conver_len);
			if (ret == -1)
			{
				SAY("ipc2net_func failed ret =%d\n",ret);
				break;
			}
			else if(ret == 1)
			{
				//未改变
				SAY("ipc2net_func not convert.\n");
			}
			else
			{
				data_len = conver_len + CONN_HEADER_LEN;
				memcpy(recv_buffer+CONN_HEADER_LEN,temp_buffer,conver_len);
			}


			ConnHeader *pconn_hd = (ConnHeader*)recv_buffer;	
			stconn* _pstconn = conn_map.get_conn_frm_flow(queue_flow);
			
			if (_pstconn)
			{
				//关闭原有连接
				if ((_pstconn->_ip != pconn_hd->_ip) || (_pstconn->_port != pconn_hd->_port))	
				{
					cc->CloseFlow(queue_flow);
					conn_map.delconn(queue_flow);
					_pstconn = NULL;
					SAY("flow %d ip,port change,close first\n",queue_flow);
				}
				else
				{
					SAY("flow %d in old link.\n",queue_flow);
					_pstconn->lastavtive = time(0);
				}
			}
			else
			{
				SAY("flow %d in new link.\n",queue_flow);
			}
			
			//case 1
			if (pconn_hd->_reqrsp_type== conn_req_connect)
			{
				//new link
				if (!_pstconn)
				{
					SAY("req to connect %d:%d\n",pconn_hd->_ip, pconn_hd->_port);	
					CSocketTCP socket;
					socket.create();
					socket.set_nonblock();
					ret = socket.connect(pconn_hd->_ip, pconn_hd->_port);
					if ((ret != 0 )&& (ret != -EWOULDBLOCK) && (ret != -EINPROGRESS))
					{
						continue;
					}
					
					//连接并加入组中
					stconn m_stconn;
					m_stconn._ip = pconn_hd->_ip;
					m_stconn._port = pconn_hd->_port;
					m_stconn.connstatus = status_connecting;
					m_stconn.lastavtive = time(0);
					ret = conn_map.add(queue_flow,&m_stconn);
					if (ret < 0)
						continue;
					
					cc->AddConn(socket.fd(), queue_flow);
					epoll->add(socket.fd(), queue_flow, EPOLLIN | EPOLLET | EPOLLERR | EPOLLOUT|EPOLLHUP);
					socket.detach();
				}
				else //already exist, reply success
				{ 
					SAY("req to connect %d:%d, but already connected.\n",pconn_hd->_ip, pconn_hd->_port);			
					pconn_hd->_reqrsp_type = conn_rsp_connect_ok;
					memcpy(recv_buffer,pconn_hd,CONN_HEADER_LEN);
					rsp_mq->enqueue(recv_buffer, CONN_HEADER_LEN, queue_flow);
				}
			}
			//case 2
			else if (pconn_hd->_reqrsp_type == conn_req_disconnect)
			{
				if (_pstconn)
				{
					SAY("req to disconnect %d:%d.\n",pconn_hd->_ip, pconn_hd->_port);

					//组中删除
					ret = conn_map.delconn(queue_flow);
					if (ret < 0)
						continue;
					
					cc->CloseFlow(queue_flow);
				}
				
				//响应
				pconn_hd->_reqrsp_type = conn_rsp_disconnected;
				memcpy(recv_buffer,pconn_hd,CONN_HEADER_LEN);
				rsp_mq->enqueue(recv_buffer, CONN_HEADER_LEN, queue_flow);					
			}
			
			//case 3
			else if (pconn_hd->_reqrsp_type == conn_req_send)
			{
				if (_pstconn)
				{
					if (_pstconn->connstatus != status_connected)
						SAY("%d:%d in connecting , data push to cache to wait...\n",pconn_hd->_ip, pconn_hd->_port);				
					else
						SAY("req to send on a connect link %d:%d.\n",pconn_hd->_ip, pconn_hd->_port);
					
					int icurrfd = cc->FD(queue_flow);
					
					// 0 new data not send, only send cache data
					// -E_NEED_CLOSE
					// send_len > 0 send new data length
					// -E_NOT_FINDFD
					ret = cc->Send(queue_flow, recv_buffer + CONN_HEADER_LEN, data_len - CONN_HEADER_LEN);
					if(ret < 0)
					{
						if (ret == -E_NEED_CLOSE)
						{
							SAY("send to flow %d failed ,ret= %d, disconnect.\n",queue_flow,ret);				
							cc->CloseFlow(queue_flow);
							conn_map.delconn(queue_flow);
						}
						else
							assert(ret == -E_NOT_FINDFD);
					}
					else
					{
						if((unsigned)ret < data_len - CONN_HEADER_LEN)  
						{
							//未发送完，继续监视EPOLLOUT，网络发了一部分
							epoll->modify(icurrfd, queue_flow, EPOLLOUT|EPOLLIN | EPOLLERR|EPOLLHUP|EPOLLET);
						}
						else
						{
							//发送完毕，网络 or cache都发送完
							assert((unsigned)ret == data_len - CONN_HEADER_LEN);
						}
					}
				}
				else  //之前没有，新建，并不通知上层先建立，而是自动建立，如果建立失败则告知上层
				{
					CSocketTCP socket;
					socket.create();
					socket.set_nonblock();
					ret = socket.connect(pconn_hd->_ip, pconn_hd->_port);
					if ((ret != 0 )&& (ret != -EWOULDBLOCK) && (ret != -EINPROGRESS))
					{
						//响应
						pconn_hd->_reqrsp_type = conn_rsp_connect_failed;
						memcpy(recv_buffer,pconn_hd,CONN_HEADER_LEN);
						rsp_mq->enqueue(recv_buffer, CONN_HEADER_LEN, queue_flow);
						SAY("connect failed %d:%d.\n",pconn_hd->_ip, pconn_hd->_port);					
						continue;
					}

					SAY("connect first %d:%d before send.\n",pconn_hd->_ip, pconn_hd->_port);			
					//连接并加入组中
					stconn m_stconn;
					m_stconn._ip = pconn_hd->_ip;
					m_stconn._port = pconn_hd->_port;
					m_stconn.connstatus = status_send_connecting;	
					m_stconn.lastavtive = time(0);
					ret = conn_map.add(queue_flow,&m_stconn);
					if (ret < 0)
						continue;
						
					cc->AddConn(socket.fd(), queue_flow);
					
					//放入缓存
					// -E_NOT_FINDFD
					// 0, send failed or send not complete, add epollout
					// 1, send complete
					ret = cc->SendForce(queue_flow, recv_buffer + CONN_HEADER_LEN, data_len - CONN_HEADER_LEN);
					if(ret == 1)
					{
						epoll->add(socket.fd(), queue_flow, EPOLLIN | EPOLLET | EPOLLERR | EPOLLHUP);
					}
					else if(ret == 0)
    				{
						epoll->add(socket.fd(), queue_flow, EPOLLIN | EPOLLET | EPOLLERR | EPOLLOUT|EPOLLHUP);
    				}
					else
    				{
    	                // 因为单线程,刚刚加入到cc，不可能出现-E_NOT_FINDFD
    				    assert(false);
    				}
    				socket.detach();
				}
			}
		}
	}

}		

