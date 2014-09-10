
#ifndef __ESF_IPC_CACHE_HASH_MAP_H__
#define __ESF_IPC_CACHE_HASH_MAP_H__

#include <cstring>
#include <cassert>
#include "esf_ipc_cache_chunk_alloc.h"

namespace esf{namespace ipc
{
	
	typedef enum tagENodeFlag
	{
		NODE_FLAG_UNCHG = 0x00,
		NODE_FLAG_DIRTY = 0x01,	
	}ENodeFlag;
	
#pragma pack(1)
	typedef struct tagTMBHashKey
	{
		static const unsigned C_DATA_LEN = 16;
		union un_key
		{
			char md5_[C_DATA_LEN];
			unsigned uin_;
		};
		un_key _;

		tagTMBHashKey(){memset(_.md5_, 0, C_DATA_LEN);}
		tagTMBHashKey& operator =(const tagTMBHashKey& right)
		{
			memcpy(_.md5_, right._.md5_, C_DATA_LEN);
			return *this;
		}
	}TMBHashKey;
	
	typedef struct tagTHashNode
	{
		TMBHashKey key_;					   //Ë÷Òý
		int chunk_len_;				 //CHUNKÖÐµÄÊý¾Ý³¤¶È
		BC_MEM_HANDLER chunk_head_;   //CHUNK ¾ä±ú
		BC_MEM_HANDLER node_prev_;	//½ÚµãÁ´±íÇ°Ö¸Õë
		BC_MEM_HANDLER node_next_;	//½ÚµãÁ´±íºóÖ¸Õë
		BC_MEM_HANDLER add_prev_;	 //¸½¼ÓÁ´±íÇ°Ö¸Õë
		BC_MEM_HANDLER add_next_;	 //¸½¼ÓÁ´±íºóÖ¸Õë
		int add_info_1_;				//×îºó·ÃÎÊÊ±¼ä
		int add_info_2_;//·ÃÎÊ´ÎÊý
		int flag_;//Ôà±ê¼Ç
	}THashNode;
	
	typedef struct tagTHashMap
	{
		int node_total_;				//½Úµã×ÜÊý
		int bucket_size_;			   //HASHÍ°µÄ´óÐ¡
		int used_node_num_;			 //Ê¹ÓÃµÄ½ÚµãÊý
		int used_bucket_num_;		   //HASHÍ°Ê¹ÓÃÊý
		BC_MEM_HANDLER add_head_;	 //¸½¼ÓÁ´±íÍ·Ö¸Õë
		BC_MEM_HANDLER add_tail_;	 //¸½¼ÓÁ´±íÎ²Ö¸Õë
		BC_MEM_HANDLER free_list_;	//¿Õ¼ä½ÚµãÁ´±íÍ·Ö¸Õë
		BC_MEM_HANDLER bucket[1];	 //HASHÍ°
	}THashMap;
	
#pragma pack()
	
	inline bool operator== (const TMBHashKey &lhs, const TMBHashKey &rhs)
	{
		return !memcmp(lhs._.md5_, rhs._.md5_, TMBHashKey::C_DATA_LEN);
	}
	
	class CHashMap
	{
	public:
		enum HASH_MAP_ERROR
		{
			HASH_MAP_ERROR_BASE = -1000,	
				HASH_MAP_ERROR_INVALID_PARAM = HASH_MAP_ERROR_BASE -1,	//·Ç·¨²ÎÊý
				HASH_MAP_ERROR_NODE_NOT_EXIST = HASH_MAP_ERROR_BASE -2,	//½Úµã²»´æÔÚ
				HASH_MAP_ERROR_NODE_HAVE_EXIST = HASH_MAP_ERROR_BASE -3,	//½ÚµãÒÑ¾­´æÔÚ
				HASH_MAP_ERROR_NO_FREE_NODE = HASH_MAP_ERROR_BASE -4,	//Ã»ÓÐ¿ÕÏÐ½Úµã
		};
		
	public:
		CHashMap();
		~CHashMap();	
		
		THashMap* hash_map_;   //ÄÚ´æ¿éÖÐµÄHASHMAP ½á¹¹
		//³õÊ¼»¯ HASH_MAP ÄÚ´æ¿é
		int open(char* pool, bool init, int node_total, int bucket_size, int n_chunks, int chunk_size);
		
		// Ê¹ÓÃ <key> ½øÐÐ²éÑ¯.
		THashNode* find_node(TMBHashKey &key);	
		//²åÈë½Úµã, Èç¹û¾É½Úµã´æÔÚ, Ôò·µ»ØÊ§°Ü
		THashNode* insert_node(TMBHashKey &key, void* new_data, int new_len);
		//ÐÞ¸Ä½Úµã
		THashNode* update_node(THashNode* node, void* new_data, int new_len, 
			char* old_data = NULL, int* old_len = NULL);
		//insert or update
		THashNode* replace_node(TMBHashKey &key, void* new_data, int new_len, char* old_data = NULL, int* old_len = NULL);
		//删除结点. 同时会将节点从附加链表中清除
		//返回值 = 0 表示成功, < 0 表示失败(如节点不存在,也返回失败)
		int delete_node(THashNode* node, char* data = NULL, int* data_len = NULL);
		
		int merge_node_data(THashNode* node, char* data, int* data_len);
		
		// ·µ»Øµ±Ç°½ÚµãÊ¹ÓÃÊý
		int used_node_num() { return hash_map_->used_node_num_; }
		int free_node_num() { return hash_map_->node_total_ - hash_map_->used_node_num_; }
		int get_node_total() { return hash_map_->node_total_; }
		int get_bucket_used() { return hash_map_->used_bucket_num_; }
		int free_bucket_num() {return hash_map_->bucket_size_ - hash_map_->used_bucket_num_; }
		int get_bucket_size() {return hash_map_->bucket_size_;}
		int get_used_chunk_num() { return allocator_.get_used_chunk_num(); }
		int get_chunk_total() { return allocator_.get_chunk_total(); }
		
		CChunkAllocator* chunks() {return &allocator_; };
		
		// ¼ÆËãHASH_MAPËùÐèÇóµÄÄÚ´æ¿é³ß´ç
		static int get_pool_size(int node_total, int bucket_size)
		{
			int head_size = sizeof(THashMap) - sizeof(BC_MEM_HANDLER[1]);
			int bucket_total_size = bucket_size * sizeof(BC_MEM_HANDLER);
			int node_total_size = node_total * sizeof(THashNode);
			
			int pool_size = head_size + bucket_total_size + node_total_size;
			return pool_size;		
		}
		// È¡HASH_MAP ºÍCHUNKµÄÄÚ´æ¿é³ß´ç
		static int get_total_pool_size(int node_total, int bucket_size, int n_chunks, int chunk_size)
		{
			return get_pool_size(node_total, bucket_size) + CChunkAllocator::get_pool_size(n_chunks, chunk_size);
		}
		/*
		//´òÓ¡¸½¼ÓÁ´±í, <num> Ö¸¶¨´òÓ¡µÄÊýÄ¿. <num> = 0, ´òÓ¡ËùÓÐ½Úµã.
		void print_add_list(int num = 0);
		//´òÓ¡»ù±¾µÄÍ³¼ÆÐÅÏ¢
		void print_stat();
		*/
		//transform handler to address
		THashNode *handler2ptr(BC_MEM_HANDLER handler);
		
		//transform address to handler
		BC_MEM_HANDLER ptr2handler(THashNode* ptr);
		
		//¸½¼ÓÁ´±í²Ù×÷·½·¨
		void insert_add_list_head(THashNode* node);
		void insert_add_list_tail(THashNode* node);
		void delete_from_add_list(THashNode* node);
		THashNode* get_add_list_prev(THashNode* node);
		THashNode* get_add_list_next(THashNode* node);
		THashNode* get_add_list_head();
		THashNode* get_add_list_tail();
		////////////////	
		
		void set_node_flag(THashNode * node, ENodeFlag f){assert(node); node->flag_ = (int)f;}
		ENodeFlag get_node_flag(THashNode *node){assert(node); return (ENodeFlag)node->flag_;}
		THashNode* get_bucket_list_head(unsigned bucket_id);
		THashNode* get_bucket_list_prev(THashNode* node);
		THashNode* get_bucket_list_next(THashNode* node);
		
	protected:
		
		void init_pool_data(int node_total, int bucket_size);
		int verify_pool_data(int node_total, int bucket_size);
		
		//¸ù¾ÝË÷Òý¼ÆËãHASHÍ°Öµ
		int get_bucket_id(TMBHashKey &key);
		int get_bucket_list_len(int bucket_id); //È¡HASHÍ°µÄÅö×²Êý
		
		//½«½Úµã²åÈëµ½¿ÕÏÐÁ´±í
		void free_list_insert(THashNode *node);
		//´Ó¿ÕÏÐÁ´±íÖÐÈ¡½Úµã
		THashNode *free_list_remove();
		
		//½ÚµãÁ´±í²Ù×÷·½·¨
		void insert_node_list(THashNode* node);
		void delete_from_node_list(THashNode* node);
		
		//³õÊ¼»¯½Úµã
		void init_node(THashNode* node);
		//½«½ÚµãÖÃÎª¿ÕÏÐÄ£Ê½
		void free_node(THashNode *node);
		//½«½ÚµãÖÃÎªÊ¹ÓÃÄ£Ê½
		void use_node(THashNode *node, TMBHashKey &key, int chunk_len, BC_MEM_HANDLER chunk_head);
	public:		
		char *pool_;		//ÄÚ´æ¿éÆðÊ¼µØÖ·
		char *pool_tail_;   //ÄÚ´æ¿é½áÊøµØÖ·
	protected:	
	//	THashMap* hash_map_;   //ÄÚ´æ¿éÖÐµÄHASHMAP ½á¹¹
		THashNode* hash_node_; //ÄÚ´æ¿éÖÐµÄHASH½ÚµãÊý×é
		CChunkAllocator allocator_; //CHUNK·ÖÅäÆ÷
	private:
		//Ô­À´ÔÚhash functionÀï¼ÆËã£¬ÏÖÔÚÔÚ³õÊ¼»¯Ê±¼ÆËãÒ»´Î¡£
		int right_rotate_;
};

//////////////////////////////////////////////////////////////////////////

inline int CHashMap::get_bucket_id(TMBHashKey &key)
{
	return ((unsigned)key._.uin_) % ((unsigned)hash_map_->bucket_size_);
}

inline THashNode* CHashMap::handler2ptr(BC_MEM_HANDLER handler)
{
	if (handler == INVALID_BC_MEM_HANDLER)
		return NULL;
	
	return (THashNode*)(pool_ + handler);
}

inline BC_MEM_HANDLER CHashMap::ptr2handler(THashNode* ptr)
{
	char *tmp_ptr = (char *)ptr;
	if((tmp_ptr < pool_) || (tmp_ptr >= pool_tail_))
		return INVALID_BC_MEM_HANDLER;
	else
		return (BC_MEM_HANDLER)(tmp_ptr - pool_);	
}

inline void CHashMap::free_list_insert(THashNode *node)
{
	//insert to free list's head
	node->node_next_ = hash_map_->free_list_;
	BC_MEM_HANDLER node_hdr = ptr2handler(node);
	hash_map_->free_list_ = node_hdr;
}

inline THashNode* CHashMap::free_list_remove()
{
	//get head node from free list
	if(hash_map_->free_list_ == INVALID_BC_MEM_HANDLER)
		//ERROR_RETURN_NULL(HASH_MAP_ERROR_NO_FREE_NODE, "no free node");
		return NULL;
	
	THashNode* head_node = handler2ptr(hash_map_->free_list_);
	hash_map_->free_list_ = head_node->node_next_;
	head_node->node_next_ = INVALID_BC_MEM_HANDLER;	
	return head_node;
}

inline void CHashMap::init_node(THashNode* node)
{
	memset(node->key_._.md5_, 0, TMBHashKey::C_DATA_LEN);
	node->chunk_len_ = 0;
	node->add_info_1_ = 0;
	node->add_info_2_ = 0;
	node->flag_ = NODE_FLAG_UNCHG;
	
	node->chunk_head_ = INVALID_BC_MEM_HANDLER;
	node->node_next_= INVALID_BC_MEM_HANDLER;
	node->node_prev_= INVALID_BC_MEM_HANDLER;
	node->add_next_= INVALID_BC_MEM_HANDLER;
	node->add_prev_= INVALID_BC_MEM_HANDLER;
}

inline THashNode*  CHashMap::get_bucket_list_head(unsigned bucket_id)
{
	assert(bucket_id < (unsigned)hash_map_->bucket_size_);
	BC_MEM_HANDLER node_hdr = hash_map_->bucket[bucket_id];
	return node_hdr != INVALID_BC_MEM_HANDLER ? handler2ptr(node_hdr) : NULL; 
}

inline THashNode*  CHashMap::get_bucket_list_prev(THashNode* node)
{
	assert(node);
	return node->node_prev_!= INVALID_BC_MEM_HANDLER ? handler2ptr( node->node_prev_) : NULL;
}
inline THashNode*  CHashMap::get_bucket_list_next(THashNode* node)
{
	assert(node);
	return node->node_next_!= INVALID_BC_MEM_HANDLER ? handler2ptr( node->node_next_) : NULL;
}

inline int CHashMap::merge_node_data(THashNode* node, char* data, int* data_len)
{
	return allocator_.merge(node->chunk_head_, node->chunk_len_, data, data_len);
}

}}

//////////////////////////////////////////////////////////////////////////
#endif
///:~
