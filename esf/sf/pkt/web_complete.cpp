#include <iostream>
#include <arpa/inet.h>
using namespace std;

extern "C"
{
int net_complete_func(const void* data, unsigned data_len)
{
	char *_data = (char *)data;
	const unsigned MAX_LEN = 1024*1024*128;

	if (data_len <= 0)
	{
	    return 0;
	}

	// first char must be '0'
	if (_data[0] != '0')
	{
	    return -1;
	}
	if (data_len < 5)
	{
	    return 0;
	}

	// get data length 
	unsigned cont_len = *(unsigned*)(_data+1);
	
	cont_len = ntohl(cont_len);

	unsigned pkt_len = 1+sizeof(unsigned)+cont_len+1;

	if (MAX_LEN < pkt_len)
	{
	    printf("pkt_len > MAX_LEN\n");
	    return -1;
	}
	
	// a full pkt?	
	if (data_len > pkt_len) // run over
	{
	    
	    if (_data[pkt_len-1] != '0')
	    {
		return -1;
	    }
	    return pkt_len;
	}
	else if (data_len == pkt_len) // full
	{
	    return pkt_len;
	}
	else // not full
	{
	    return 0;
	}
}
}
