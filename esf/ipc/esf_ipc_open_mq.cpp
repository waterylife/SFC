
#include "esf_ipc_open_mq.h"
#include "esf_sys_config_file.h"
#include "esf_sys_str.h"

using namespace esf::ipc;
using namespace esf::sys;

//////////////////////////////////////////////////////////////////////////

CFifoSyncMQ* esf::ipc::GetMQ(const string& conf_file)
{
	CFileConfig conf;
	conf.Init(conf_file);
	
	const int shm_key = from_str<int>(conf["root\\shm_key"]);
	const unsigned shm_size = from_str<unsigned>(conf["root\\shm_size"]);
	
	CShmMQ* shm_q = new CShmMQ();
	int ret = shm_q->init(shm_key, shm_size);
	assert(ret == 0);
	
	const int sem_key = from_str<int>(conf["root\\sem_key"]);
	const unsigned sem_size = from_str<unsigned>(conf["root\\sem_size"]);
	const unsigned sem_index = from_str<unsigned>(conf["root\\sem_index"]);
	
	CSemLockMQ* sem_q = new CSemLockMQ(*shm_q);
	ret = sem_q->init(sem_key, sem_size, sem_index);
	assert(ret == 0);
	
	const string fifo_path = conf["root\\fifo_path"];
	const unsigned wait_sec = from_str<unsigned>(conf["root\\wait_sec"]);
	const unsigned wait_usec = from_str<unsigned>(conf["root\\wait_usec"]);
	
	CFifoSyncMQ* fifo_q = new CFifoSyncMQ(*sem_q);
	ret = fifo_q->init(fifo_path, wait_sec, wait_usec);
	assert(ret == 0);
	return fifo_q;
}

//////////////////////////////////////////////////////////////////////////
///:~
