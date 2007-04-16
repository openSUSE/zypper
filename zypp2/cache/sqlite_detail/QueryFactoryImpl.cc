/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <sqlite3.h>
#include "zypp2/cache/sqlite3x/sqlite3x.hpp"

#include "zypp/base/Logger.h"

#include "zypp2/cache/DatabaseTypes.h"
#include "zypp2/cache/CacheInitializer.h"
#include "zypp2/cache/QueryFactory.h"

#include "zypp2/cache/sqlite_detail/CacheSqlite.h"
#include "zypp2/cache/sqlite_detail/QueryFactoryImpl.h"

using namespace std;
using namespace zypp;
using namespace zypp::capability;
using namespace zypp::cache;
using namespace sqlite3x;

namespace zypp { namespace cache {

///////////////////////////////////////////////////////////////
// CACHE QUERY                                              //
//////////////////////////////////////////////////////////////

QueryFactory::Impl::Impl( const Pathname &pdbdir, sqlite3x::sqlite3_connection_ptr con )
{
  context.reset(new DatabaseContext);
  context->dbdir = pdbdir;
  context->con= con;
  initCommands();
}
    
  
QueryFactory::Impl::Impl( const Pathname &pdbdir )
{
  cache::CacheInitializer initializer(pdbdir, "zypp.db");
  if ( initializer.justInitialized() )
  {
    MIL << "database " << (pdbdir + "zypp.db") << " was just created" << endl;
  }
  
  context.reset(new DatabaseContext);
  context->dbdir = pdbdir;
  
  try
  {
    context->con->open( (pdbdir + "zypp.db").asString().c_str());
  }
  catch(exception &ex)
  {
    //ZYPP_CAUGHT(ex);
    ZYPP_THROW(Exception(ex.what()));
  }
 
  initCommands();
}

void QueryFactory::Impl::initCommands()
{
  try
  {
    // precompile statements
    context->select_versionedcap_cmd.reset( new sqlite3_command( *context->con, "select c.refers_kind, n.name, v.version, v.release, v.epoch, v.relation, c.dependency_type from names n, capabilities c, named_capabilities v where v.name_id=n.id and c.id=v.dependency_id and c.resolvable_id=:rid;"));
    context->select_namedcap_cmd.reset( new sqlite3_command( *context->con, "select c.refers_kind, n.name, c.dependency_type from names n, capabilities c, named_capabilities nc where nc.name_id=n.id and c.id=nc.dependency_id and c.resolvable_id=:rid;"));
    context->select_filecap_cmd.reset( new sqlite3_command( *context->con, "select c.refers_kind, dn.name, fn.name, c.dependency_type from file_names fn, dir_names dn, capabilities c, file_capabilities fc, files f  where f.id=fc.file_id and f.dir_name_id=dn.id and f.file_name_id=fn.id and c.id=fc.dependency_id and c.resolvable_id=:rid;"));
    
    // disable autocommit
    context->con->executenonquery("PRAGMA cache_size=8000;");
    context->con->executenonquery("BEGIN;");
  }
  catch(exception &ex)
  {
    //ZYPP_CAUGHT(ex);
    ZYPP_THROW(Exception(ex.what()));
  }
}

QueryFactory::Impl::~Impl()
{
  context->con->executenonquery("COMMIT;");
  context->con->executenonquery("PRAGMA cache_size=2000;");
}


} } // ns zypp::cache

