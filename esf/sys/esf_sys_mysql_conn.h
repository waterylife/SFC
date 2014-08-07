
#ifndef __ESF_SYS_MYSQL_CONN_H__
#define __ESF_SYS_MYSQL_CONN_H__

#include "mysql.h"
#include <string>
#include <set>
#include "esf_sys_debug_log.h"

using namespace std;

#define QUERY_DB_TIMEOUT 5


class DBConf
{
public:
	DBConf();
	void set_db_no(char *db_num);
	void print_conf();

	void parse_no(char* nums, std::set<int>& num_set);
public:
	string db_;//db前缀
	string tbl_;//tbl前缀

	string tbl2_;//tbl前缀

	string ip_;
	int port_;
	string user_;
	string pswd_;
	string socket_;
	
	set<int> db_num_;//本服务器上的db后缀编号
	
	int total_db_num_;//总共的db数,包括别的机器上
	int tbl_num_per_db_;
};

class MysqlResult
{
protected:
    MYSQL_RES* result_;
	
public:
    MysqlResult()
    {
        result_ = NULL;
    }

    MysqlResult(MYSQL_RES* resultSet)
    {
        result_ = resultSet;
    }
    ~MysqlResult()
    {
        if (result_ != NULL)
        {
            mysql_free_result(result_);
            result_ = NULL;
        }
    }
    void result(MYSQL_RES* rst) 
    {
        if (result_ != NULL)
        {
            mysql_free_result(result_);
            result_ = NULL;
        }
        result_ = rst;
    }
    MYSQL_RES* result() { return result_; }
};

// the database connection class
class MysqlConn
{
public:

public:
	MysqlConn();
	~MysqlConn();

	int initConn(DBConf &db_conf);
	void finiConn();

	MYSQL* getMysql();
	void returnMysql(MYSQL* mysql);


	bool inScope(int num);


	// execute update sql statement(update, insert, delete)
	int execUpdate(const char* sql, int& insertID, int& affected, int sql_len=0);

	// execute select sql statement
	int execSelect(const char* sql, MysqlResult& resultSet, int sql_len=0);

private:
	int reconnect();
	DBConf conf_;    
	MYSQL* conn_;
	
};

#endif /* C4AS_DBCONN_H_ */
