
#include <string.h>
#include <assert.h>

#include "esf_net_raw_cache.h"

using namespace esf::net;
using namespace std;

//////////////////////////////////////////////////////////////////////////

CRCPool::CRCPool()
{
	_rc.clear();
	_pool_count = 0;
}
CRCPool::~CRCPool()
{
	for(list<ConnCache*>::iterator it = _rc.begin()
		; it != _rc.end()
		; it++)
	{
		delete *it;
	}
}

int CRCPool::pre_allocate(unsigned pool_size, CMemPool& mp)
{
	if (_pool_count != 0)
		return false;

	_pool_count = pool_size;
	for(unsigned i = 0; i < pool_size; i++)
	{
		ConnCache* rc = new ConnCache(mp);
		_rc.push_front(rc);
	}
	return 0;
}

ConnCache* CRCPool::allocate()
{
	if (_pool_count == 0)
		return NULL;

	ConnCache* rc = _rc.front();
	_rc.pop_front();

	_pool_count--;
	return rc;
}

void CRCPool::recycle(ConnCache* rc)
{
	assert(rc != NULL);
	rc->_fd = -1;
	rc->_flow = 0;
	rc->_r.skip(rc->_r.data_len());
	rc->_w.skip(rc->_w.data_len());
	_rc.push_front(rc);

	_pool_count++;
}

//////////////////////////////////////////////////////////////////////////

CRawCache::CRawCache(CMemPool& mp)
: _mp(mp), _mem(NULL), _block_size(0), _data_head(0), _data_len(0)
{}

CRawCache::~CRawCache(){}

char* CRawCache::data()
{
	if (_data_len == 0)
		return NULL;

	assert(_data_head < _block_size);
	return _mem + _data_head;
}

unsigned CRawCache::data_len(){return _data_len;}

void CRawCache::append(const char* data, size_t data_len)
{
	//assert(data_len < 0x00010000);

	if (_mem == NULL)
	{
		_mem = (char*) _mp.allocate(data_len, _block_size);
		assert(_mem);
		
		memcpy(_mem, data, data_len);

		_data_head = 0;
		_data_len = data_len;
		
		return;
	}
	
	//
	//	data_len < _block_size - _data_head - _data_len
	//	append directly
	//

	if (data_len + _data_head + _data_len <= _block_size)
	{
		memcpy(_mem + _data_head + _data_len, data, data_len);
		_data_len += data_len;
	}

	//
	//	_block_size-_data_len <= data_len
	//	reallocate new block. memmove, recycle
	//

	else if (data_len + _data_len > _block_size)
	{
		size_t new_block_size = 0;
		char* mem = (char*) _mp.allocate(data_len+_data_len, new_block_size);
		assert(mem);

		memcpy(mem, _mem + _data_head, _data_len);
		memcpy(mem + _data_len, data, data_len);
		_mp.recycle(_mem, _block_size);

		_mem = mem;
		_block_size = new_block_size;
		_data_head = 0;
		_data_len += data_len;
	}

	//
	//	_block_size - _data_head - _data_len < data_len < _block_size-_datalen
	//	memmove data to block head, append new data
	//

	else
	//if ((data_len + _data_head + _data_len > _block_size) && (data_len + _data_len < _block_size))
	{
		memmove(_mem, _mem+_data_head, _data_len);
		memcpy(_mem+_data_len, data, data_len);

		_data_head = 0;
		_data_len += data_len;
	}
}

void CRawCache::skip(unsigned length)
{
	//	empty data
	if (_mem == NULL)
		return;

	//	not enough data
	else if (length >= _data_len)
	{
		_mp.recycle(_mem, _block_size);
		_mem = NULL;
		_block_size = _data_head = _data_len = 0;
		_data_head = 0;
		_data_len = 0;
	}

	//	skip part of data
	else
	{
		_data_head += length;
		_data_len -= length;
	}
}

//////////////////////////////////////////////////////////////////////////
///:~
