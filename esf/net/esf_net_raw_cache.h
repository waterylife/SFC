
#ifndef __ESF_NET_RAW_CACHE_H__
#define __ESF_NET_RAW_CACHE_H__

#include <list>
#include "esf_net_mem_pool.h"

//////////////////////////////////////////////////////////////////////////

namespace esf { namespace net {
		
		class CRawCache
		{
		public:
			CRawCache(esf::net::CMemPool& mp);
			~CRawCache();
			
			char* data();
			unsigned data_len();

			void append(const char* data, size_t data_len);
			void skip(unsigned length);

		private:
			esf::net::CMemPool& _mp;

			char* _mem;
			size_t _block_size;

			unsigned _data_head;
			unsigned _data_len;
		};

		class ConnCache
		{
		public:
			ConnCache(esf::net::CMemPool& mp) : _r(mp), _w(mp){}
			unsigned _flow;
			int _fd;
			time_t _access;

			CRawCache _r;
			CRawCache _w;
		};

		class CRCPool
		{
		public:
			CRCPool();
			~CRCPool();

			int pre_allocate(unsigned pool_size, esf::net::CMemPool& mp);

			ConnCache* allocate();
			void recycle(ConnCache* rc);

		private:
			std::list<ConnCache*> _rc;
			unsigned _pool_count;
		};
}}

//////////////////////////////////////////////////////////////////////////
#endif
///:~
