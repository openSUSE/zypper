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
#include "zypp/cache/CacheInitializer.h"
#include "zypp/target/store/PersistentStorage.h"
#include "zypp/cache/Utils.h"
#include "zypp/PathInfo.h"
#include "sqlite-schema.h"

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
  : root(root_r), just_initialized(false),
    just_reinitialized(false)
  {
  }
  //typedef std::map<media::MediaNr, media::MediaAccessId> MediaMap
  shared_ptr<sqlite3x::sqlite3_connection> con;
  Pathname root;
  bool just_initialized;
  bool just_reinitialized;
};

CacheInitializer::CacheInitializer( const Pathname &root_r, const Pathname &db_file )
  : _pimpl( new Impl( root_r ) )
{
  try
  {
     assert_dir( _pimpl->root );
    _pimpl->con.reset( new sqlite3_connection( ( _pimpl->root + db_file).asString().c_str()) );
    _pimpl->con->executenonquery("begin;");
    if( ! tablesCreated() )
    {
      createTables();
      _pimpl->just_initialized = true;
      //_pimpl->con->close();
      MIL << "Repository cache initialized" << std::endl;
    }
    else
    {
      int version = _pimpl->con->executeint("select version from db_info;");
      if ( version != ZYPP_CACHE_SCHEMA_VERSION )
      {
        WAR << "cache schema version is different from ZYpp cache version" << endl;
        // FIXME should this code ask first?
        sqlite3_command tables_cmd( *_pimpl->con, "select name from sqlite_master where type='table';");
        sqlite3_reader reader = tables_cmd.executereader();
        list<string> tables;
        while ( reader.read() )
        {
          string tablename = reader.getstring(0);
          if ( tablename != "sqlite_sequence" )
            tables.push_back(tablename);
        }
        reader.close();
        
        for ( list<string>::const_iterator it = tables.begin();
              it != tables.end();
              ++it )
        {
          MIL << "Removing table " << *it << endl;
          _pimpl->con->execute("drop table " + (*it) + ";");
        }
        
        createTables();
        _pimpl->just_reinitialized = true;
        MIL << "Repository cache re-initialized" << std::endl;
        return;
      }
      
      MIL << "Repository cache already initialized" << std::endl;
    }
  }
  catch( const exception &ex )
  {
    ZYPP_RETHROW(Exception(ex.what()));
    //ERR << "Exception Occured: " << ex.what() << endl;
  }
  _pimpl->con->executenonquery("commit;");
  _pimpl->con->close();
}

bool CacheInitializer::justInitialized() const
{
  return _pimpl->just_initialized;
}

bool CacheInitializer::justReinitialized() const
{
  return _pimpl->just_reinitialized;
}

CacheInitializer::~CacheInitializer()
{

}

bool CacheInitializer::tablesCreated() const
{
  Measure timer("Check tables exist");
  unsigned int count = _pimpl->con->executeint("select count(*) from sqlite_master where type='table';");
  timer.elapsed();
  return ( count > 0 );
}

void CacheInitializer::createTables()
{
  //Measure timer("Create database tables");
  MIL << "Initializing cache schema..." << endl;
  //sqlite3_transaction trans(*_pimpl->con);
  //{
    string sql( schemaData, _schemaData_size);
    //ERR << "Executing " << statements[i] << endl;
    MIL << "Schema size: " << sql.size() << endl;
    _pimpl->con->execute(sql.c_str());
    sqlite3_command version_cmd( *_pimpl->con, "insert into db_info (version) values(:version);");
    version_cmd.bind(":version", ZYPP_CACHE_SCHEMA_VERSION);
    version_cmd.executenonquery();
  //}
  //trans.commit();
  //timer.elapsed();
}

std::ostream & CacheInitializer::dumpOn( std::ostream & str ) const
{
  return str;
}

}
}

