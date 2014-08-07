#include <stdio.h>
#include <string.h>

#include <mysql.h>

int main()
{
	MYSQL mysql;
	char* query;
	MYSQL_RES *res = NULL;
	MYSQL_ROW row;

	mysql_init(&mysql);

	if(!mysql_real_connect(&mysql, 
						   "localhost", 
						   "root", 
						   "cocoa12", 
						   "crashcourse",
						   3306,
						   NULL, 0)) {
		printf("Fail to connect to mysql");
		return 0;
	}

	query = "select * from customers";
	if(0 != mysql_real_query(&mysql, query, strlen(query))) {
		printf("Fail to query: %s\n", query);
		return 0;
	}

	res = mysql_store_result(&mysql);
	if(NULL == res) {
		printf("No result queryed\n");
		return 0;
	}
	while(row = mysql_fetch_row(res)) {
		int i;
		for(i = 0; i < mysql_num_fields(res); i++) {
			printf("%s\t", row[i]);
		}
		printf("\n");
	}

	mysql_free_result(res);
	mysql_close(&mysql);

	return 0;
}