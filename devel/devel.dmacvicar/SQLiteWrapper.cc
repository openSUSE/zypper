/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "zypp/base/Logger.h"

#include "SQLiteWrapper.h"

SQLiteWrapper::SQLiteWrapper(std::string tablename="init.db"): zErrMsg(0), rc(0),m_db_open(0)
{
	rc = sqlite3_open(tablename.c_str(), &m_db);
	if( rc )
	{
		DBG << "Can't open database: " << sqlite3_errmsg(m_db) << std::endl;
		sqlite3_close(m_db);
	}
	m_db_open = 1;
}

int SQLiteWrapper::exe(std::string s_exe)
{
	rc = sqlite3_get_table(
		m_db,              /* An open database */
		s_exe.c_str(),       /* SQL to be executed */
		&result,       /* Result written to a char *[]  that this points to */
		&nrow,             /* Number of result rows written here */
		&ncol,          /* Number of result columns written here */
		&zErrMsg          /* Error msg written here */
	);

	if(vcol_head.size() > 0) {vcol_head.clear();}
	if(vdata.size()>0) {vdata.clear();}
	if( rc == SQLITE_OK )
	{
		for(int i=0; i < ncol; ++i)
			vcol_head.push_back(result[i]); /* First row heading */
		for(int i=0; i < ncol*nrow; ++i)
			vdata.push_back(result[ncol+i]);
	}
	sqlite3_free_table(result);
	return rc;
}

sqlite3 * SQLiteWrapper::db()
{
	return m_db;
}

SQLiteWrapper::~SQLiteWrapper()
{
	sqlite3_close(m_db);
}

