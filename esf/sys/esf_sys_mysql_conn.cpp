

#include "esf_sys_mysql_conn.h"

////////////////////////////////////////

DBConf::DBConf()
{
}

void DBConf::set_db_no(char *db_num)
{
    parse_no(db_num, db_num_);
		
}

void DBConf::parse_no(char* nums, std::set<int>& num_set)
{
    char* p = strchr(nums, '[');
    if (p != NULL)
    {
        p++;
        for(; p[0] != '\0' && p[0] != ']';)
        {
 		while(*p != '\0' && !isdigit(*p)){++p;}
		if(*p == '\0')
		{
			break;
		}
            int db_no = atoi(p);
            num_set.insert(db_no);
		while(*p != '\0' && isdigit(*p)) {p++;}
		if(*p == '\0')
		{	
			break;
		}

            if (p[0] == ',')
            {
                p++;
                continue;
            }
            else if (p[0] == '-')
            {
                p++;
                int end_no = atoi(p);
                for (int i = db_no + 1; i < end_no; i++)
                {
                    num_set.insert(i);
                }                
            }
        }
    }
}


void DBConf::print_conf()
{
    DEBUG_P_NO_TIME(LOG_DEBUG, "db:%s; tbl: %s; ip:%s; port:%d; user:%s; pwd:%s; unix_socket %s;  "
		"total_db_num %d, tbl_num_per_db_ %d, db_nos:", 
        db_.c_str(), tbl_.c_str(), ip_.c_str(), port_, user_.c_str(), pswd_.c_str(), socket_.c_str(),
        total_db_num_, tbl_num_per_db_);
    for (set<int>::iterator pos = db_num_.begin(); pos != db_num_.end(); pos++)
    {
        DEBUG_P_NO_TIME(LOG_DEBUG, "%d,",*pos);
    }
    DEBUG_P_NO_TIME(LOG_DEBUG, "\n");

}

////////////////////////////////////////

MysqlConn::MysqlConn() : conn_(NULL)
{
}

MysqlConn::~MysqlConn()
{
}

int MysqlConn::initConn(DBConf &db_conf)
{
	conf_ = db_conf;
	
	DEBUG_P(LOG_TRACE, "init db conn...\n");

	conf_.print_conf();

	conn_ = mysql_init(NULL);
	if(!conn_)
	{
		DEBUG_P(LOG_FATAL, "mysqlConn init fail .\n");
		return -1;
	}

#if MYSQL_VERSION_ID > 50012
       mysql_options(conn_, MYSQL_OPT_RECONNECT, "1");
#endif

	if (mysql_real_connect(conn_, 
					conf_.ip_.size() > 0 ? conf_.ip_.c_str() : NULL,
					conf_.user_.size() > 0 ? conf_.user_.c_str() : NULL,
					conf_.pswd_.size()> 0 ? conf_.pswd_.c_str() : NULL,
					NULL,
					conf_.port_,
					conf_.socket_.size() > 0 ? conf_.socket_.c_str() : NULL,
					0) == NULL)
	{
		DEBUG_P(LOG_FATAL, "mysql connect fail : error %d, %s.\n", 
			mysql_errno(conn_), mysql_error(conn_));
		return -1;
	}
	
    DEBUG_P(LOG_TRACE,  "MysqlConn::%s connected to db ok.\n", __FUNCTION__);
    return 0;
}

void MysqlConn::finiConn()
{
	DEBUG_P(LOG_TRACE, "MysqlConn::%s  disconnect to db.\n", __FUNCTION__);
	mysql_close(conn_);
}

MYSQL* MysqlConn::getMysql()
{   

    return conn_;
}

void MysqlConn::returnMysql(MYSQL* mysql)
{
   
}

int MysqlConn::execUpdate(const char* sql, int& insertID, int& affected, int sql_len/* =0 */)
{
	MYSQL* conn = getMysql();
	assert(conn);

	int ret;
	if (sql_len == 0)
		sql_len = strlen(sql);

	ret = mysql_real_query(conn, sql, sql_len);
	if(ret)
	{
		DEBUG_P(LOG_NORMAL, "MysqlConn::%s query fail, ret %d, error:  %d, %s,  try reconnect .\n", 
			__FUNCTION__, ret, mysql_errno(conn), mysql_error(conn));
		if((ret = reconnect()) ==0)
		{
			DEBUG_P(LOG_TRACE, "MysqlConn::%s reconnect ok.\n", 
				__FUNCTION__);
			ret = mysql_real_query(conn, sql, sql_len);
		}		
	}

	if(ret)
	{
		DEBUG_P(LOG_ERROR, "MysqlConn::%s  fail to query , error : %d, %s .\n",
			__FUNCTION__, mysql_errno(conn_), mysql_error(conn_));
		returnMysql(conn);
		return -1;
	}	

	insertID = mysql_insert_id(conn);
	affected = mysql_affected_rows(conn);

	returnMysql(conn);

//	DEBUG_P(LOG_TRACE, "MysqlConn::%s query OK,  sql: %s, "
//	    "insertd_id: %d, affected_rows: %d\n", __FUNCTION__, sql, insertID, affected);

	return 0;
}

int MysqlConn::execSelect(const char* sql, MysqlResult& resultSet, int sql_len/* =0 */)
{
	MYSQL* conn = getMysql();
	assert(conn);

	int ret;
	if (sql_len == 0)
		sql_len = strlen(sql);
	ret = mysql_real_query(conn, sql, sql_len);
	if(ret)
	{
		DEBUG_P(LOG_NORMAL, "MysqlConn::%s query fail, ret %d, error:  %d, %s,  try reconnect .\n", 
			__FUNCTION__, ret, mysql_errno(conn), mysql_error(conn));
		if((ret = reconnect()) ==0)
		{
			DEBUG_P(LOG_TRACE, "MysqlConn::%s reconnect ok.\n", 
				__FUNCTION__);
			ret = mysql_real_query(conn, sql, sql_len);
		}		
	}

	if(ret)
	{
		DEBUG_P(LOG_ERROR, "MysqlConn::%s  fail to query , error : %d, %s .\n",
			__FUNCTION__, mysql_errno(conn_), mysql_error(conn_));
		returnMysql(conn);
		return -1;
	}	

	MYSQL_RES* result = NULL;
	if ((result = mysql_store_result(conn)) == NULL)
	{
		DEBUG_P(LOG_ERROR, "MysqlConn::%s mysql_store_result failed, error: %d, %s\n", 
			__FUNCTION__, mysql_errno(conn), mysql_error(conn));
		returnMysql(conn);
		return -1;
	}

	resultSet.result(result);

	returnMysql(conn);

	//DEBUG_P(LOG_TRACE, "MysqlConn::%s mysql_real_query succeed, sql: %s\n", __FUNCTION__, sql);

	return 0;
}
int MysqlConn::reconnect()
{
	mysql_close(conn_);
	conn_ = NULL;
	conn_ = mysql_init(NULL);
	if(!conn_)
	{
		DEBUG_P(LOG_FATAL, "MysqlConn::%s mysql_init fail .\n", __FUNCTION__);
		return -1;
	}
	if (mysql_real_connect(conn_, 
					conf_.ip_.size() > 0 ? conf_.ip_.c_str() : NULL,
					conf_.user_.size() > 0 ? conf_.user_.c_str() : NULL,
					conf_.pswd_.size()> 0 ? conf_.pswd_.c_str() : NULL,
					NULL,
					conf_.port_,
					conf_.socket_.size() > 0 ? conf_.socket_.c_str() : NULL,
					0) == NULL)
	{
		DEBUG_P(LOG_FATAL, "MysqlConn::%s  connect fail: error %d, %s.\n", 
			__FUNCTION__, mysql_errno(conn_), mysql_error(conn_));
		return -1;
	}
	return 0;
}

bool MysqlConn::inScope(int num)
{
    set<int>::iterator it = conf_.db_num_.find(num);
    if (it != conf_.db_num_.end())
    {
        return true;
    }
    return false;
}



