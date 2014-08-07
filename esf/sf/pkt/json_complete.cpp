#include <arpa/inet.h>

extern "C"
{
int json_complete_func(const void* data, unsigned data_len)
{
	char *_data = (char *)data;
	const unsigned MAX_LEN = 1024*1024*128;

	if (data_len <= 0)
	{
	    return 0;
	}

	if (MAX_LEN < data_len )
	{
	    return -1;
	}

	for( int i=data_len; i>0; i-- )
	{
		if (_data[i-1] == '\n' && _data[i-2] =='\r' )
		{
			return i;
		}
	}

	return 0;
}
}
