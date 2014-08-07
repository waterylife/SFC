
#ifndef __ESF_SYS_PROC_H__
#define __ESF_SYS_PROC_H__

#include <map>
#include <string>

#include "esf_ipc_mq.h"
#include "esf_ipc_cache_access.h"

//////////////////////////////////////////////////////////////////////////

namespace esf{

	class CacheProc
	{
	public:
		CacheProc(){}
		virtual ~CacheProc(){}
		virtual void run(const std::string& conf_file) = 0;
		std::map<std::string, esf::ipc::CFifoSyncMQ*> _mqs;
		std::map<std::string, esf::ipc::CacheAccess*> _caches;
	};
}

//////////////////////////////////////////////////////////////////////////
#endif
///:~

