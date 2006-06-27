/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "zypp/base/Logger.h"
#include "zypp/cache/KnownSourcesCache.h"
#include "zypp/target/store/PersistentStorage.h"
#include "zypp/cache/sqlite3x/sqlite3x.hpp"

#define ZYPP_DB_FILE "/var/lib/zypp/zypp.db"

using namespace sqlite3x;
using namespace std;

//////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace cache
{ /////////////////////////////////////////////////////////////////

KnownSourcesCache::KnownSourcesCache( const Pathname &root_r )
{
  try
  {
    sqlite3_connection con(ZYPP_DB_FILE);
    
    {
      int count = con.executeint("select count(*) from sqlite_master where type='table' and name='sources';");
      if( count==0 )
      {
        con.executenonquery("create table sources (  id integer primary key autoincrement,  alias varchar,  url varchar,  description varchar,  enabled integer, autorefresh integer, type varchar, cachedir varchar, path varchar);");
        
        try
        {
          // import old sources
          storage::PersistentStorage store;
          store.init( root_r );
          source::SourceInfoList old_sources = store.storedSources();
          for ( source::SourceInfoList::const_iterator it = old_sources.begin(); it != old_sources.end(); it++ )
          {
            storeSource( *it );
          }
        }
        catch(std::exception &e)
        {
          ERR << "Exception Occured: " << e.what() << endl;
        } 
      }
    }
    
    con.close();
  }
  catch(exception &ex) {
    ERR << "Exception Occured: " << ex.what() << endl;
  } 
}

KnownSourcesCache::~KnownSourcesCache()
{
}

source::SourceInfoList KnownSourcesCache::knownSources() const
{
  return source::SourceInfoList();
}

void KnownSourcesCache::storeSource( const source::SourceInfo &info )
{
  try
  {
    sqlite3_connection con(ZYPP_DB_FILE);
    sqlite3_transaction trans(con);

    {
      sqlite3_command cmd(con, "insert into sources ( alias, url, description, enabled, autorefresh, type, cachedir, path) values ( ?, ?, ?, ? , ?, ?, ?, ?);");
      cmd.bind(1, info.alias());
      cmd.bind(2, info.url().asCompleteString());
      // FIXME 
      cmd.bind(4, info.enabled() ? 1 : 0 );
      cmd.bind(5, info.autorefresh() ? 1 : 0 );
      cmd.bind(6, info.type());
      cmd.bind(6, info.cacheDir().asString());
      cmd.bind(7, info.path().asString());
      
      cmd.executenonquery();
    }

    trans.commit(); // note: if trans goes out of scope (due to an exception or anything else) before calling commit(), it will automatically rollback()

    con.close();
  }
  catch(exception &ex) {
    ERR << "Exception Occured: " << ex.what() << endl;
  }
} 

std::ostream & KnownSourcesCache::dumpOn( std::ostream & str ) const
{
  return str;
}

}
}

