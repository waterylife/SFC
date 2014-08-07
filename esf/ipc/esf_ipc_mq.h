
#ifndef __ESF_IPC_MQ_H__
#define __ESF_IPC_MQ_H__

#include "esf_ipc_sv.h"	


namespace esf { namespace ipc
{
	static const int E_DEQUEUE_BUF_NOT_ENOUGH = -13001;
	class CShmMQ
	{
	public:
		typedef struct tagMQStat
		{
			unsigned _used_len;
			unsigned _free_len;
			unsigned _total_len;
			unsigned _shm_key;
			unsigned _shm_id;
			unsigned _shm_size;
		}TMQStat;

	public:
		CShmMQ();
		~CShmMQ();
		int init(int shm_key, unsigned shm_size);
		int enqueue(const void* data, unsigned data_len, unsigned flow);
		int dequeue(void* buf, unsigned buf_size, unsigned& data_len, unsigned& flow);
		
		void get_stat(TMQStat& mq_stat)
		{
			unsigned head = *_head;
			unsigned tail = *_tail;
			
			mq_stat._used_len = (tail>=head) ? tail-head : tail+_block_size-head;
			mq_stat._free_len = head>tail? head-tail: head+_block_size-tail;
			mq_stat._total_len = _block_size;
			mq_stat._shm_key = _shm->key();
			mq_stat._shm_id = _shm->id();
			mq_stat._shm_size = _shm->size();
		}

	private:
		esf::sys::ptr<esf::ipc::CShm> _shm;

		unsigned* _head;
		unsigned* _tail;
		char* _block;
		unsigned _block_size;

		static const unsigned C_HEAD_SIZE = 8;
	};

	class CSemLockMQ
	{
	public:
		typedef struct tagSemLockMQStat
		{
			CShmMQ::TMQStat _mq_stat;
			unsigned _sem_key;
			unsigned _sem_id;
			unsigned _sem_index;
			unsigned _sem_size;
		}TSemLockMQStat;

	public:
		CSemLockMQ(CShmMQ& mq);
		~CSemLockMQ();
		
		int init(int sem_key, unsigned sem_size, unsigned sem_index);
		int enqueue(const void* data, unsigned data_len, unsigned flow);
		int dequeue(void* buf, unsigned buf_size, unsigned& data_len, unsigned& flow);
		void get_stat(TSemLockMQStat& mq_stat)
		{
			_mq.get_stat(mq_stat._mq_stat);
			mq_stat._sem_key = _sem->key();
			mq_stat._sem_id = _sem->id();
			mq_stat._sem_size = _sem->size();
			mq_stat._sem_index = _sem_index;
		}
		
	private:
		CShmMQ& _mq;
		esf::sys::ptr<esf::ipc::CSem> _sem;
		unsigned _sem_index;
	};
	
	class CFifoSyncMQ
	{
	public:
		typedef struct tagFifoSyncMQStat
		{
			CSemLockMQ::TSemLockMQStat _semlockmq_stat;
			unsigned _wait_sec;
			unsigned _wait_usec;
		}TFifoSyncMQStat;

	public:
		CFifoSyncMQ(CSemLockMQ& mq);
		~CFifoSyncMQ();
		
		int init(const std::string& fifo_path, unsigned wait_sec, unsigned wait_usec);
		int enqueue(const void* data, unsigned data_len, unsigned flow);
		int dequeue(void* buf, unsigned buf_size, unsigned& data_len, unsigned& flow);
		void get_stat(TFifoSyncMQStat& mq_stat)
		{
			_mq.get_stat(mq_stat._semlockmq_stat);
			mq_stat._wait_sec = _wait_sec;
			mq_stat._wait_usec = _wait_usec;
		}
		int fd(){ return _fd; };
		
	private:
		int select_fifo();

		CSemLockMQ& _mq;
		int _fd;
		unsigned _wait_sec;
		unsigned _wait_usec;
	};

}}

//////////////////////////////////////////////////////////////////////////
#endif
///:~
