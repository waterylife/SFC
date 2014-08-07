
#ifndef __ESF_IPC_CACHE_ACCESS_H__
#define __ESF_IPC_CACHE_ACCESS_H__

#include "esf_ipc_cache_hash_map.h"
#include "esf_sys_binlog.h"

//////////////////////////////////////////////////////////////////////////

namespace esf{namespace ipc
{
	
	class CacheAccess
	{
	public:
		CacheAccess()
		{
			_lastdumptime = time(NULL);
			_cache_dump_min= -1;
			strcpy(_cache_dump_file,"./cache.dump");
			_cacheinit = true;
		}
		~CacheAccess(){}
		
		int open(char* mem, unsigned mem_size, bool inited
			, int node_total, int bucket_size, int n_chunks, int chunk_size);
		
		int set(const char* key, const char* data, unsigned data_len, ENodeFlag flag=NODE_FLAG_DIRTY);
		int del(const char* key);
		int get(const char* key, char* buf, unsigned buf_size
			, unsigned& data_len, bool& dirty_flag, int& time_stamp);
		int get_key(const char* key, unsigned& data_len, bool& dirty_flag, int& time_stamp);
		int mark_clean(const char* key);
		
		int get_lru_pre(const char* key, char* pre_key
			, char* buf, unsigned buf_size
			, unsigned& data_len, bool& dirty_flag, int& time_stamp);
		int oldest(char* buf, unsigned buf_size
			, char* key, unsigned& data_len, bool& dirty_flag, int& time_stamp);
		int oldest_key(char* key, unsigned& data_len, bool& dirty_flag, int& time_stamp);

		int mark_clean(int modbase, int mobres);
		int del_node(int modbase, int mobres);
		int core_dump_mem(char *buff, int maxsize,int modbase, int mobres);
		int core_recover_mem(char *buff, int buffsize);
		int core_dump(char *szcorefile);
		int core_recover(char *szcorefile);
		int StartUp();
		int CoreInit(int coredump_min,char *coredump_file,char * sBinLogBaseName, long lMaxBinLogSize, int iMaxBinLogNum);

		int time_check();
		
		// HashNode的使用率超过80％
		bool warning_80persent();
		// HashNode, ChunkNode的使用情况
		void get_node_num(unsigned &hash_node_used, unsigned &hash_node_total, 
                            unsigned &chunk_node_used, unsigned &chunk_node_total);
		/////////////////////////////////
		enum
		{
			op_set = 0,
			op_mark_clean,
			op_del
		};
		
	class dirty_iterator
	{
	public:
		dirty_iterator(const dirty_iterator& right)
			: _cache(right._cache)
			, _cur_node(right._cur_node)
		{}
		dirty_iterator& operator ++(int)
		{
			if (_cur_node == NULL)
				return *this;	//	error tolerance
			//_cur_node = _cache->get_add_list_next(_cur_node);
			for(_cur_node = _cache->get_add_list_next(_cur_node)
				; _cur_node != NULL && _cur_node->flag_ == NODE_FLAG_UNCHG
				; _cur_node = _cache->get_add_list_next(_cur_node))
				;
			
			return *this;
		}
		bool operator ==(const dirty_iterator& right)
		{return _cur_node == right._cur_node;}
		bool operator !=(const dirty_iterator& right)
		{return _cur_node != right._cur_node;}
		int get(char* key, char* buf, unsigned buf_size, unsigned& data_len)
		{
			if (_cur_node == NULL)
				return -1;

			memcpy(key,_cur_node->key_._.md5_,TMBHashKey::C_DATA_LEN);
			data_len = buf_size;
			return  _cache->merge_node_data(_cur_node, buf, (int*)&data_len);
		}
	
	protected:
		dirty_iterator(CHashMap* cache, THashNode* cur_node)
			: _cache(cache)
			, _cur_node(cur_node)
		{}
	public:	
		CHashMap * const _cache;
		THashNode* _cur_node;
	private:
		dirty_iterator& operator ++();
		void* operator->();
		int operator* ();
		friend class CacheAccess;
	};

	dirty_iterator begin(){return dirty_iterator(&_cache, _cache.get_add_list_head());}
	dirty_iterator end(){return dirty_iterator(&_cache, NULL);}
	
		CHashMap _cache;
		
		time_t _lastdumptime;
		int _cache_dump_min;
		char _cache_dump_file[256];

		CBinLog _binlog;

		bool _cacheinit;
	};
	
	//////////////////////////////////////////////////////////////////////////
	
	class CacheAccessUin
	{
	public:
		CacheAccessUin(CacheAccess& da) : _da(da){}
		~CacheAccessUin(){}
		
		int set(unsigned uin, const char* data, unsigned data_len,ENodeFlag flag=NODE_FLAG_DIRTY);
		int del(unsigned uin);
		int get(unsigned uin, char* buf, unsigned buf_size
			, unsigned& data_len, bool& dirty_flag, int& time_stamp);
		int get_key(unsigned uin, unsigned& data_len, bool& dirty_flag, int& time_stamp);
		int mark_clean(unsigned uin);

		int get_lru_pre(unsigned key, unsigned& pre_key
			, char* buf, unsigned buf_size
			, unsigned& data_len, bool& dirty_flag, int& time_stamp);
		int oldest(char* buf, unsigned buf_size
			, unsigned& uin, unsigned& data_len, bool& dirty_flag, int& time_stamp);
		int oldest_key(unsigned& uin, unsigned& data_len, bool& dirty_flag, int& time_stamp);

		int mark_clean(int modbase, int mobres);
		int del_node(int modbase, int mobres);
		int core_dump_mem(char *buff, int maxsize,int modbase, int mobres);
		int core_recover_mem(char *buff, int buffsize);
		
		int core_dump(char *szcorefile);
		int core_recover(char *szcorefile);
		int CoreInit(int coredump_hour,char *coredump_file,char * sBinLogBaseName, long lMaxBinLogSize, int iMaxBinLogNum);
		int StartUp();
		int time_check();
	    
	    // HashNode的使用率超过80％
		bool warning_80persent();
		// HashNode, ChunkNode的使用情况
		void get_node_num(unsigned &hash_node_used, unsigned &hash_node_total, 
                            unsigned &chunk_node_used, unsigned &chunk_node_total);
	
		CacheAccess& _da;
	};
}}

//////////////////////////////////////////////////////////////////////////
#endif
///:~
