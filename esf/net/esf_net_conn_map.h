#ifndef __CONN_MAP_H__
#define __CONN_MAP_H__

void xprintf(const char *szFormat, ...);

#ifdef _DEBUG_
#define SAY printf
#else
#define SAY xprintf
#endif

#include <map>
#include <vector>
//////////////////////////////////////////////////////////////////////////

namespace esf{ namespace net{

enum reqrsp_type
{
	conn_req_connect = 0,		//������Ŀ��
	conn_req_disconnect = 1,	//��Է��Ͽ�����
	conn_req_send = 2,		//���͵��Է�,�Զ���������,���������ӽ���֪ͨ

	conn_rsp_connect_ok = 20,		//֪ͨ���ӳɹ�
	conn_rsp_connect_failed = 21, //֪ͨ����ʧ��
	conn_rsp_disconnected = 22,	//֪ͨ���Ӷ���
	conn_rsp_data = 23,		//Я����Ӧ����
	conn_rsp_send_failed=24   //֪ͨ����ʧ��
};


enum status_type
{
	status_connecting = 0,		//������Ŀ��
	status_send_connecting = 1,		//������Ŀ��
	status_connected = 2,	
};

typedef struct
{
	unsigned int _ip;
	unsigned short  _port;
	unsigned short _reqrsp_type;
}ConnHeader;

#define CONN_HEADER_LEN		8

typedef unsigned long long ull;

typedef struct
{
	unsigned int _ip;
	unsigned short  _port;
	short connstatus;
	int lastavtive;
}stconn;

class ConnMap
{
public:
	ConnMap(int);
	~ConnMap();

	int add(unsigned queue_flow, stconn* pstconn);
	stconn * get_conn_frm_flow(unsigned queue_flow);
	int delconn(unsigned queue_flow);
	int GetExpire(std::vector<unsigned> &vec_key);

private:
	std::map<unsigned, stconn> _flow2stconn;
	int _max_conn;
};
}}

#endif