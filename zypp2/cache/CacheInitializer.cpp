/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <vector>
#include <sstream>
#include <fstream>

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Measure.h"
#include "zypp2/cache/CacheInitializer.h"
#include "zypp2/cache/schema/schema.h"
#include "zypp/target/store/PersistentStorage.h"
#include "zypp2/cache/Utils.h"

#include "schema.h"

#define ZYPP_DB_FILE "/var/lib/zypp/zypp.db"

using namespace sqlite3x;
using namespace std;
using zypp::debug::Measure;

//////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace cache
{ /////////////////////////////////////////////////////////////////

struct CacheInitializer::Impl
{
  Impl( const Pathname &root_r )
  : root(root_r), just_initialized(false)
  {
  }
  //typedef std::map<media::MediaNr, media::MediaAccessId> MediaMap
  shared_ptr<sqlite3x::sqlite3_connection> con;
  Pathname root;
  bool just_initialized;
};

CacheInitializer::CacheInitializer( const Pathname &root_r, const Pathname &db_file )
  : _pimpl( new Impl( root_r ) )
{
  try
  {
    _pimpl->con.reset( new sqlite3_connection( ( _pimpl->root + db_file).asString().c_str()) );

    if( ! tablesCreated() )
    {
      createTables();
      _pimpl->just_initialized = true;
      _pimpl->con->close();
      MIL << "Source cache initialized" << std::endl;
    }
    else
    {
      MIL << "Source cache already initialized" << std::endl;
    }
  }
  catch( const exception &ex )
  {
    ZYPP_RETHROW(Exception(ex.what()));
    //ERR << "Exception Occured: " << ex.what() << endl;
  }

}

bool CacheInitializer::justInitialized() const
{
  return _pimpl->just_initialized;
}

CacheInitializer::~CacheInitializer()
{

}

bool CacheInitializer::tablesCreated() const
{
  Measure timer("Check tables exist");
  unsigned int count = _pimpl->con->executeint("select count(*) from sqlite_master where type='table';");
  timer.elapsed();
  return ( count == 26 );
}

void CacheInitializer::createTables()
{
  Measure timer("Create database tables");
  MIL << "Initializing cache schema..." << endl;
  sqlite3_transaction trans(*_pimpl->con);
  {
    string sql;
    const char *filename = "/usr/share/zypp/cache/schema.sql";
    std::ifstream stream(filename);
    string buffer;
    if ( stream )
    {
      stringstream str(sql);
      while ( getline( stream, buffer ) )
      {
        sql += (buffer+"\n");
      }
      //std::cout << sql << endl;
    }
    else
    {
      ZYPP_THROW(Exception(str::form("Can't open db schema %s", filename)));
    }

    //ERR << "Executing " << statements[i] << endl;
    MIL << "Schema size: " << sql.size() << endl;
    _pimpl->con->execute(sql.c_str());
  }
  trans.commit();
  timer.elapsed();
}

std::ostream & CacheInitializer::dumpOn( std::ostream & str ) const
{
  return str;
}

}
}

