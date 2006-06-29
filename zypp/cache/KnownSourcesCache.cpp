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

#define ZYPP_DB_FILE "/var/lib/zypp/zypp.db"

using namespace sqlite3x;
using namespace std;

//////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace cache
{ /////////////////////////////////////////////////////////////////

KnownSourcesCache::KnownSourcesCache( const Pathname &root_r ) : _root(root_r)
{
  try
  {
    _con.reset( new sqlite3_connection(ZYPP_DB_FILE) );
  }
  catch(exception &ex)
  {
    ERR << "Exception Occured: " << ex.what() << endl;
  }

  try
  {
      if( ! tablesCreated() )
      {
        try
        {
          importOldSources();
        }
        catch(std::exception &e)
        {
          ERR << "Exception Occured: " << e.what() << endl;
        } 
      }
  }
  catch(exception &ex)
  {
    ERR << "Exception Occured: " << ex.what() << endl;
  }
}

KnownSourcesCache::~KnownSourcesCache()
{
  _con->close();
}

bool KnownSourcesCache::tablesCreated() const
{
	unsigned int count = _con->executeint("select count(*) from sqlite_master where type='table' and name='sources';");
	return ( count > 0 );
}

void KnownSourcesCache::createTables()
{
	_con->executenonquery("create table sources (  id integer primary key autoincrement,  alias varchar,  url varchar,  description varchar,  enabled integer, autorefresh integer, type varchar, cachedir varchar, path varchar);");
}

void KnownSourcesCache::importOldSources()
{
  // import old sources
  storage::PersistentStorage store;
  store.init( _root );
  source::SourceInfoList old_sources = store.storedSources();
  for ( source::SourceInfoList::const_iterator it = old_sources.begin(); it != old_sources.end(); it++ )
  {
    storeSource( *it );
  }
}

source::SourceInfoList KnownSourcesCache::knownSources() const
{
  source::SourceInfoList sources;
  try
  {
      sqlite3_command cmd( *_con , "select * from sources;");
      sqlite3_reader reader = cmd.executereader();

      while(reader.read())
      {
        source::SourceInfo info;
        info.setAlias(reader.getstring(1));
        info.setUrl(reader.getstring(2));
        info.setEnabled( (reader.getint(4) == 1 ) ? true : false );
        info.setAutorefresh( (reader.getint(5) == 1 ) ? true : false );
        info.setType(reader.getstring(6));
        info.setCacheDir(reader.getstring(7));
        info.setPath(reader.getstring(8));
        sources.push_back(info);
      }
  }
  catch(exception &ex)
  {
    ERR << "Exception Occured: " << ex.what() << endl;
  }
  return sources;
}

void KnownSourcesCache::storeSource( const source::SourceInfo &info )
{
  try
  {
      sqlite3_command cmd( *_con, "insert into sources ( alias, url, description, enabled, autorefresh, type, cachedir, path) values ( ?, ?, ?, ? , ?, ?, ?, ?);");
      cmd.bind(1, info.alias());
      cmd.bind(2, info.url().asCompleteString());
      // FIXME no description
      cmd.bind(4, info.enabled() ? 1 : 0 );
      cmd.bind(5, info.autorefresh() ? 1 : 0 );
      cmd.bind(6, info.type());
      cmd.bind(7, info.cacheDir().asString());
      cmd.bind(8, info.path().asString());
      
      cmd.executenonquery();
  }
  catch(exception &ex)
  {
    ERR << "Exception Occured: " << ex.what() << endl;
  }
} 

std::ostream & KnownSourcesCache::dumpOn( std::ostream & str ) const
{
  return str;
}

}
}

