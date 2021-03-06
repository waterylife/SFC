#ifndef __ESF_SYS_TIMER_H__
#define __ESF_SYS_TIMER_H__

#include <sys/time.h>
#include <map>

namespace esf{ namespace sys {
	
	// 保存在Timer中，超时后由Timer负责delete
	class CSimpleTimerInfo
	{
	public:
		CSimpleTimerInfo(){};
		virtual ~CSimpleTimerInfo(){};
		friend class CSimpleTimerQueue;

		
		int rtti;				// 记录子类的类型信息，以便强制转换，并选择处理函数
		unsigned ret_flow;		// 需要回复接入端的flow
		unsigned ret_msg_seq;	// 需要回复请求方的seq
		unsigned ret_destid;	// 需要回复请求放的子进程，一般不用
		
	public:
		// 超时后由timer调用，不要做大量事务
		virtual void on_expire(){ return; };
		// 超时后由timer调用，可以控制timer不删除timerinfo对象
		// 默认是会删除对象
		virtual bool on_expire_delete(){ return true; };
	protected:
		time_t _access_time;
		time_t _gap;
		unsigned _msg_seq;
	};
	
	class CSimpleTimerQueue
	{
	public:
		CSimpleTimerQueue(){};
		virtual ~CSimpleTimerQueue(){};
		//
		// 安装timer
		// return 0		成功
		//	      <0	失败
		//
		int set(unsigned msg_seq, CSimpleTimerInfo* timer_info, time_t gap = 10 /* 10 seconds */);
		
		//
		// 获得timer对应的数据，并且卸载timer
		// return 0		成功
		//        <0    不存在
		//
		int get(unsigned msg_seq, CSimpleTimerInfo** timer_info);

		// 
		//  检查msg_seq对应的数据是否存在
		//	return 0	存在
		//		   >0	不存在
		//
		int exist(unsigned msg_seq);

		//
		//	删除超时数据
		//
		virtual void check_expire(time_t time_expire);
	protected:
		std::map<unsigned, CSimpleTimerInfo*> _mp_timer_info;

};

}}


#endif
