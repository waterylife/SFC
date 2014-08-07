
#include "esf_sys_proc.h"
#include "esf_ipc_open_mq.h"
#include "esf_ipc_sv.h"
#include "esf_sys_config_file.h"
#include "esf_sys_str.h"
#include "esf_sys_so.h"
#include "esf_sys_daemon.h"

#include <sys/file.h>
#include <iostream>
#include <list>

using namespace std;
using namespace esf;
using namespace esf::ipc;
using namespace esf::sys;



/////////////////////////////////////////////

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
	
	typedef CacheProc* (*app_constructor)();
	const string app_so_file = page["root\\app_so_file"];
	const string create_app_func = page["root\\create_app_func"];
	CSOFile so_file;
	int ret = so_file.open(app_so_file);
	assert(ret == 0);
	app_constructor constructor = (app_constructor) so_file.get_func(create_app_func);
	CacheProc* proc = constructor();
	
	//
	//	open mq
	//
	
	const map<string, string>& mqs = page.GetPairs("root\\mq");
	for(map<string, string>::const_iterator it = mqs.begin()
		; it != mqs.end()
		; it++)
	{
		CFifoSyncMQ* mq = GetMQ(it->second);
		assert(mq);
		proc->_mqs[it->first] = mq;
	}

	//
	//	open mem cache if exist
	//

	//	get all cache name
	const vector<string>& caches = page.GetSubPath("root\\cache");
	list< ptr<CShm> > shm_stub;
	for(vector<string>::const_iterator it = caches.begin()
		; it != caches.end()
		; it++)
	{
		const string cache_name = *it;
		int shm_key = from_str<int>(page["root\\cache\\" + cache_name + "\\shm_key"]);
		unsigned shm_size = from_str<unsigned>(page["root\\cache\\" + cache_name + "\\shm_size"]);
		unsigned node_total = from_str<unsigned>(page["root\\cache\\" + cache_name + "\\node_total"]);
		unsigned bucket_size = from_str<unsigned>(page["root\\cache\\" + cache_name + "\\bucket_size"]);
		unsigned chunk_total = from_str<unsigned>(page["root\\cache\\" + cache_name + "\\chunk_total"]);
		unsigned chunk_size = from_str<unsigned>(page["root\\cache\\" + cache_name + "\\chunk_size"]);

		bool bInited = true;
		ptr<CShm> shm;
		try
		{
			shm = CShm::create_only(shm_key, shm_size);
			memset(shm->memory(), 0, shm->size());
			bInited = true;
		}
		catch (ipc_ex& ex)
		{
			shm = CShm::open(shm_key, shm_size);
			bInited = false;
		}
		shm_stub.push_back(shm);

		CacheAccess* ca = new CacheAccess();
		ret = ca->open(shm->memory(), shm->size(), bInited, node_total, bucket_size, chunk_total, chunk_size);
		assert(ret == 0);
		
		proc->_caches[*it] = ca;
	}

	const string app_conf_file = page["root\\app_conf_file"];
	proc->run(app_conf_file);
	return 0;
}

//////////////////////////////////////////////////////////////////////////
///:~
