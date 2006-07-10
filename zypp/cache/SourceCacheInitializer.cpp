/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <vector>

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/cache/SourceCacheInitializer.h"
#include "zypp/target/store/PersistentStorage.h"
#include "zypp/cache/Utils.h"

#define ZYPP_DB_FILE "/var/lib/zypp/zypp.db"

using namespace sqlite3x;
using namespace std;

//////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace cache
{ /////////////////////////////////////////////////////////////////
  

static const char * SOURCES_TABLE_SCHEMA = "create table sources ( id integer primary key autoincrement, alias varchar unique, type varchar, description varchar,  url varchar, path varchar,  enabled integer, autorefresh integer, timestamp varchar, checksum varchar);";

// id 0, alias 1 , type 2, desc 3, url 4, path 5, enabled 6, autorefresh 7, timestamp 8, checksum 9
  
SourceCacheInitializer::SourceCacheInitializer( const Pathname &root_r, const Pathname &db_file )
  : _root(root_r), _just_initialized(false)
{
  try
  {
    _con.reset( new sqlite3_connection(db_file.asString().c_str()) );
  }
  catch(exception &ex) {
    ERR << "Exception Occured: " << ex.what() << endl;
  } 

  try
  {
    if( ! tablesCreated() )
    {
      createTables();
      _just_initialized = true;
      _con->close();
      MIL << "Source cache initialized" << std::endl;
    }
    else
    {
      MIL << "Source cache already initialized" << std::endl;
    }
  }
  catch(exception &ex)
  {
    ERR << "Exception Occured: " << ex.what() << endl;
  }
  
}

bool SourceCacheInitializer::justInitialized() const
{
  return _just_initialized;
}

SourceCacheInitializer::~SourceCacheInitializer()
{
 
}

bool SourceCacheInitializer::tablesCreated() const
{
	unsigned int count = _con->executeint("select count(*) from sqlite_master where type='table';");
	return ( count == 1 );
}

void SourceCacheInitializer::createTables()
{
  sqlite3_transaction trans(*_con);

  {
	 _con->executenonquery(SOURCES_TABLE_SCHEMA);
    
  }
  
  trans.commit();
}

std::ostream & SourceCacheInitializer::dumpOn( std::ostream & str ) const
{
  return str;
}

}
}

