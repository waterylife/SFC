
#ifndef __ESF_NET_MEM_POOL_H__
#define __ESF_NET_MEM_POOL_H__

#include <map>
#include <list>
#include <set>

//////////////////////////////////////////////////////////////////////////

namespace esf {	namespace net {
		//
		//	memory poll, allocate fixed size memory
		//	dont protect overflow, let it b
		//	
		//

		class CMemPool
		{
		public:
			CMemPool()
			{
				_allocated_size = 0;
				_water_mark = C_MAX_WATER_MARK;
			}
			~CMemPool(){}

			void* allocate(size_t size, size_t& result_size);
			int recycle(void* mem, size_t mem_size);

		private:
			int extend(size_t size, std::list<void*>* l, std::set<void*>* s);
			int extend_new_size(size_t size);
			int release(size_t size, std::list<void*>* l, std::set<void*>* s);

			unsigned release_size(size_t block_size, unsigned free_count, unsigned stub_count);
			size_t fit_block_size(unsigned size)
			{
				unsigned i = 10;
				size = (size>>10);
				for(; size; i++, size = (size>>1));
				return 1 << (i<10 ? 10 : i);
			}
			
			size_t fit_extend_set(unsigned size);
			
			int release_size(size_t mem_size);
			
			typedef std::map<unsigned, std::list<void*>*> mml;
			typedef std::map<unsigned, std::set<void*>*> mms;

			mml _free;
			mms _stub;

			unsigned _allocated_size;
			unsigned _water_mark;

			static const unsigned C_MAX_POOL_SIZE = 2000 * (1<<20);	//	< (2<<31)
			static const unsigned C_MAX_WATER_MARK = 1<<30;	//	< (2<<31)
		};
}}

//////////////////////////////////////////////////////////////////////////
#endif
///:~
