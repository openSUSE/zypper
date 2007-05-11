#include <mysql/mysql.h>
#include <iostream>

#define host "lorien.suse.de"
#define username "rpmread"
#define password "Salahm1"
#define database "rpm"

MYSQL *conn;

int main(int argc, char *argv[]){
	
	conn = mysql_init(NULL);
	mysql_real_connect(conn,host,username,password,database,0,NULL,0);

	if(conn == NULL)
		std::cout << "DB connection failed!";	

	MYSQL_RES *res_set;
	MYSQL_ROW row;
	unsigned int i;

	mysql_query(conn,"SELECT * FROM PackReqProv");
	res_set = mysql_store_result(conn); 
	unsigned int numrows = mysql_num_rows(res_set); 

	std::cout << "Anzahl: " << numrows << std::endl;

	while ((row = mysql_fetch_row(res_set)) != NULL){
		std::cout << row[5] << std::endl;
	}	

	mysql_close(conn);
	return 0; 

}
