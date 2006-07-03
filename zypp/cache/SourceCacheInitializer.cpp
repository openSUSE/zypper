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
  
#define SOURCES_TABLE_SCHEMA "create table sources ( alias varchar primary key, type varchar, description varchar,  url varchar, path varchar,  enabled integer, autorefresh integer, timestamp varchar, checksum varchar);"   
// alias 0 , type 1, desc 2, url 3, path 4, enabled 5, autorefresh 6, timestamp 7, checksum 8
  
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

