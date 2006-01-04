/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_SQLITE_WRAPPER_H
#define ZYPP_SQLITE_WRAPPER_H

#include <sqlite3.h>
#include <string>
#include <iostream>
#include <vector>

class SQLiteWrapper
{
public:
	SQLiteWrapper(std::string tablename);
	~SQLiteWrapper();

	int exe(std::string s_exe);
	sqlite3 * db();

	std::vector<std::string> vcol_head;
	std::vector<std::string> vdata;
private:
	sqlite3 *m_db;
	char *zErrMsg;
	char **result;
	int rc;
	int nrow,ncol;
	int m_db_open;
};

#endif

