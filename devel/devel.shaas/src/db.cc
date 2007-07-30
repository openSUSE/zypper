#include "db.h"
#include <iostream>

using std::string;

//Constructor

database::database(string nHost, string nUsername, string nPassword, string nDatabase, unsigned int nPort, string nSocket,  unsigned long nClient_flag){
	host = nHost;
	username = nUsername;
	password = nPassword;
	db = nDatabase;
	port = nPort;
	socket = nSocket;
	client_flag = nClient_flag;

	conn = mysql_init(NULL);
}

//Destructor

database::~database(){
}

//Connect to db

unsigned int database::connect(){
	if(mysql_real_connect(conn, host.c_str(), username.c_str(), password.c_str(), db.c_str(), port, socket.c_str(), client_flag) == NULL){

		if(mysql_error(conn) != "")
			std::cout << mysql_error(conn) << std::endl;
		return mysql_errno(conn);
	}

	return 1;
}

//Close db connection

void database::close(){
	mysql_close(conn);
}

//Execute given SQL-Statement

unsigned int database::sqlexecute(string sqlcom){

	if(mysql_query(conn, sqlcom.c_str())){
			if(mysql_error(conn) != "")
				std::cout << mysql_error(conn) << std::endl;
			return mysql_errno(conn);
	}

	if((result = mysql_store_result(conn)) == NULL){
			if(mysql_error(conn) != "")
				std::cout << mysql_error(conn) << std::endl;
			return mysql_errno(conn);
	}

	unsigned int numrows = mysql_num_rows(result);
	unsigned int numfields = mysql_num_fields(result);

	//std::cout << "Zeilen: " << numrows << " Spalten: " << numfields << std::endl;

	return 1;

}

//Pushes the fetchbuffer in a vector

std::vector< std::vector<string> > database::getResult(){

	std::vector< std::vector<string> > outResult;
	int vecCtr = 0; 
	unsigned int numfields = mysql_num_fields(result);

	while ((fetchbuffer = mysql_fetch_row(result)) != NULL){
			//if(mysql_error(conn) != "")
			//	std::cout << mysql_error(conn) << "\n";
			std::vector<string> temp;
			outResult.push_back(temp);

			for(unsigned int i = 0; i < numfields; i++){	
				outResult[vecCtr].push_back(fetchbuffer[i] ? fetchbuffer[i] : "NULL");
			}
			vecCtr++;

	}	

	return outResult;
}
