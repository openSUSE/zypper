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
#include "zypp2/cache/CacheInitializer.h"
#include "zypp2/cache/schema/schema.h"
#include "zypp/target/store/PersistentStorage.h"
#include "zypp2/cache/Utils.h"

#define ZYPP_DB_FILE "/var/lib/zypp/zypp.db"

using namespace sqlite3x;
using namespace std;

//////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace cache
{ /////////////////////////////////////////////////////////////////
  

  
CacheInitializer::CacheInitializer( const Pathname &root_r, const Pathname &db_file )
  : _root(root_r), _just_initialized(false)
{
  try
  {
    _con.reset( new sqlite3_connection( (_root + db_file).asString().c_str()) );
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

bool CacheInitializer::justInitialized() const
{
  return _just_initialized;
}

CacheInitializer::~CacheInitializer()
{
 
}

bool CacheInitializer::tablesCreated() const
{
  unsigned int count = _con->executeint("select count(*) from sqlite_master where type='table';");
  return ( count == 1 );
}

void CacheInitializer::createTables()
{
  sqlite3_transaction trans(*_con);
  {
    char ** statements = getsql();
    int i = 0;
    while ( statements[i] != 0 )
    {
      ERR << "Executing " << statements[i] << endl;
      _con->executenonquery(statements[i]);
      i++;
    }
  }
  trans.commit();
}

std::ostream & CacheInitializer::dumpOn( std::ostream & str ) const
{
  return str;
}

}
}

