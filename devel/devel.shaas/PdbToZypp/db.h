#include <mysql/mysql.h>
#include <string>
#include <vector>

#ifndef DB_H
#define DB_H

using std::string;

class database{
	private:
		string host;
		string username;
		string password;
		string db;
		unsigned int port;
		string socket;
		unsigned long client_flag;
		MYSQL* conn; 
		MYSQL_ROW fetchbuffer;
		MYSQL_RES* result;

	public:
		database(string host, string username, string password, string db, unsigned int port = 0, string socket = "", unsigned long client_flag = 0);
		~database();
		unsigned int connect();
		void close();
		unsigned int sqlexecute(string sqlcom);
		std::vector< std::vector<string> > getResult();
};
		

#endif
