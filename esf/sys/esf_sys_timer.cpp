#include "esf_sys_timer.h"

using namespace std;
using namespace esf::sys;

int CSimpleTimerQueue::set(unsigned msg_seq, CSimpleTimerInfo* timer_info, time_t gap/* = 10 seconds */)
{
	time_t cur_time = time(NULL);
	
	timer_info->_msg_seq = msg_seq;
	timer_info->_access_time = cur_time;
	timer_info->_gap = gap;
	_mp_timer_info[msg_seq] = timer_info;
	return 0;
}

int CSimpleTimerQueue::get(unsigned msg_seq, CSimpleTimerInfo** timer_info)
{
	map<unsigned, CSimpleTimerInfo*>::iterator it = _mp_timer_info.find(msg_seq);
	if (it == _mp_timer_info.end())
	{
		*timer_info = NULL;
		return -1;
	}
	*timer_info = it->second;
	_mp_timer_info.erase(it);
	return 0;
}

int CSimpleTimerQueue::exist(unsigned msg_seq)
{
	if (_mp_timer_info.find(msg_seq) != _mp_timer_info.end())
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

void CSimpleTimerQueue::check_expire(time_t time_expire)
{
	time_t cur_time = time(NULL);
	map<unsigned, CSimpleTimerInfo*>::iterator it = _mp_timer_info.begin();
	while (it != _mp_timer_info.end())
	{
		CSimpleTimerInfo *timer_info  = it->second;
		if ((cur_time - timer_info->_access_time) > timer_info->_gap)
		{
		    timer_info->on_expire();
			// 控制是否delete超时的对象
			if (timer_info->on_expire_delete() == true)
			{
				delete timer_info;
			}
			_mp_timer_info.erase(it++);
		}
		else
		{
			it++;
		}
	}
}
