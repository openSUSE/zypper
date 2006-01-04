#include <sqlite3.h>
#include "zypp/Patch.h"
#include "zypp/Edition.h"
#include <iostream>
#include <ctime>

#define DEBUG(X) std::cout << X << std::endl

class SQLite
{
private:
	sqlite3 *m_db;
	char *zErrMsg;
	char **result;
	int rc;
	int nrow,ncol;
	int m_db_open;
public:
	std::vector<std::string> vcol_head;
	std::vector<std::string> vdata;
	SQLite (std::string tablename="init.db"): zErrMsg(0), rc(0),m_db_open(0)
	{
		rc = sqlite3_open(tablename.c_str(), &m_db);
		if( rc )
		{
			fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(m_db));
			sqlite3_close(m_db);
		}
		m_db_open = 1;
	}

	int exe(std::string s_exe)
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

	sqlite3 * db()
	{
		return m_db;
	}

	~SQLite()
	{
		sqlite3_close(m_db);
	}
};

class PersistentStorageIf
{
public:
	PersistentStorageIf()
	{
	}

	virtual bool isDatabaseInitialized() = 0;
	virtual void initDatabaseForFirstTime() = 0;
	virtual void insertTest() = 0;
	virtual void selectTest() = 0;

	virtual void openDatabase() = 0;
	virtual void closeDatabase() = 0;

	virtual ~PersistentStorageIf() = 0;
	virtual void errorHandler(const char *error_prefix, char *msg) = 0;
};


class SQLitePersistentStorage : public PersistentStorageIf
{
public:
	SQLitePersistentStorage();

	bool
	isDatabaseInitialized();
	void initDatabaseForFirstTime();
	void insertTest();
	void selectTest();

	void
	openDatabase()
	{
	}

	void
	closeDatabase()
	{
	}

	~SQLitePersistentStorage()
	{
		delete m_db;
	}

	void
	errorHandler(const char *error_prefix, char *msg)
	{
	}

	private:
	SQLite *m_db;
};

SQLitePersistentStorage::SQLitePersistentStorage()
{
	m_db = new SQLite("database.db");
	// check if the table exists
	if (!isDatabaseInitialized())
	{
		DEBUG("Database not initialized");
		initDatabaseForFirstTime();
		if (!isDatabaseInitialized())
			DEBUG("Error, cant init database");
		else
			DEBUG("Database initialized");
	}
	else
	{
		DEBUG("Database already initialized");
	}
}

bool
SQLitePersistentStorage::isDatabaseInitialized()
{
	if (m_db->exe("select * from objects") == SQLITE_ERROR)
		return false;
	return true;
}

void
SQLitePersistentStorage::initDatabaseForFirstTime()
{
	// FIXME add modified_on?
	m_db->exe("create table objects ( id integer not null primary key, type varchar(255) not null, status varchar(255) not null ); create table attributes ( object_id integer not null, key varchar(255) not null, value varchar(255) not null );");
}

void
SQLitePersistentStorage::insertTest()
{
	int k = 0;
	int rc;
	char **result;
	int nrow;
	int ncol;
	char *zErrMsg;
	for (; k < 100; k++)
	{
		rc = sqlite3_get_table( m_db->db(), "insert into objects (type, status) values (\"sometype\", \"somestatus\");", &result, &nrow, &ncol, &zErrMsg);
		sqlite3_free_table(result);
		//m_db->exe("insert into objects (type, status) values (\"sometype\", \"somestatus\");");
	}
}

void
SQLitePersistentStorage::selectTest()
{
	
}

int main()
{
	time_t seconds1;
	time_t seconds2;
	SQLitePersistentStorage *ps = new SQLitePersistentStorage;
	seconds1 = time(NULL);
	ps->insertTest();
	seconds2 = time(NULL);
	std::cout << seconds2 - seconds1 << " seconds..." << std::endl;
	return 0;
}

/*

*/
