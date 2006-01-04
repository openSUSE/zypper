/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	devel/devel.dmacvicar/SQLiteBackend.cc
*
*/
#include <iostream>
#include <ctime>
#include "zypp/base/Logger.h"

#include "SQLiteBackend.h"
#include "SQLiteWrapper.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace storage
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : SQLiteBackend::Private
//
///////////////////////////////////////////////////////////////////
class SQLiteBackend::Private
{
	public:
	SQLiteWrapper *sqliteWrapper;
};

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : SQLiteBackend
//
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : SQLiteBackend::SQLiteBackend
//	METHOD TYPE : Ctor
//
SQLiteBackend::SQLiteBackend()
{
	// FIXME duncan - try PRAGMA synchronous = OFF somewhere
		d->sqliteWrapper = new SQLiteWrapper("database.db");
	// check if the table exists
	if (!isDatabaseInitialized())
	{
		DBG << "Database not initialized" << std::endl;
		initDatabaseForFirstTime();
		if (!isDatabaseInitialized())
			DBG << "Error, cant init database" << std::endl;
		else
			DBG << "Database initialized" << std::endl;
	}
	else
	{
		DBG << "Database already initialized" << std::endl;
	}
}

bool
SQLiteBackend::isDatabaseInitialized()
{
	// Duncan: FIX and change to SELECT name FROM 'sqlite_master' WHERE type='table' and name=
	if (d->sqliteWrapper->exe("select * from objects") == SQLITE_ERROR)
		return false;
	return true;
}

void
SQLiteBackend::initDatabaseForFirstTime()
{
	// FIXME add modified_on?
	d->sqliteWrapper->exe("create table objects ( id integer not null primary key, type varchar(255) not null, status varchar(255) not null ); create table attributes ( object_id integer not null, key varchar(255) not null, value varchar(255) not null );");
}

void
SQLiteBackend::insertTest()
{
	int k = 0;
	int rc;
	char **result;
	int nrow;
	int ncol;
	char *zErrMsg;
	for (; k < 100; k++)
	{
		rc = sqlite3_get_table( d->sqliteWrapper->db(), "insert into objects (type, status) values (\"sometype\", \"somestatus\");", &result, &nrow, &ncol, &zErrMsg);
		sqlite3_free_table(result);
		//m_db->exe("insert into objects (type, status) values (\"sometype\", \"somestatus\");");
	}
}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : SQLiteBackend::~SQLiteBackend
//	METHOD TYPE : Dtor
//
SQLiteBackend::~SQLiteBackend()
{}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : SQLiteBackend::doTest()
//	METHOD TYPE : Dtor
//
void SQLiteBackend::doTest()
{}

/******************************************************************
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : std::ostream &
*/
std::ostream & operator<<( std::ostream & str, const SQLiteBackend & obj )
{
	return str;
}

		/////////////////////////////////////////////////////////////////
	} // namespace devel.dmacvicar
	///////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////
} // namespace devel
///////////////////////////////////////////////////////////////////
