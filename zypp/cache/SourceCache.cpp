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
#include "zypp/cache/SourceCache.h"
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

static int tribool_to_int( boost::tribool b )
{
  if (b)
    return 1;
  else if (!b)
    return 0;
  else
    return 2;
}  
  
static boost::tribool int_to_tribool( int i )
{
  if (i==1)
    return true;
  else if (i==0)
    return false;
  else
    return boost::indeterminate;
}
  
static std::string checksum_to_string( const CheckSum &checksum )
{
  return checksum.type() + ":" + checksum.checksum();
}  
  
static CheckSum string_to_checksum( const std::string &checksum )
{
  std::vector<std::string> words;
  if ( str::split( checksum, std::back_inserter(words), ":" ) != 2 )
    return CheckSum();
  
  return CheckSum( words[0], words[19]);
}
  
#define SOURCES_TABLE_SCHEMA "create table sources ( alias varchar primary key, type varchar, description varchar,  url varchar, path varchar,  enabled integer, autorefresh integer, timestamp varchar, checksum varchar);"   
  
// alias 0 , type 1, desc 2, url 3, path 4, enabled 5, autorefresh 6, timestamp 7, checksum 8
  
SourceCache::SourceCache( const Pathname &root_r )
{
  try
  {
    sqlite3_connection con(ZYPP_DB_FILE);
    
    {
      int count = con.executeint("select count(*) from sqlite_master where type='table' and name='sources';");
      if( count==0 )
      {
        con.executenonquery(SOURCES_TABLE_SCHEMA);
        
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

SourceCache::~SourceCache()
{
}

source::SourceInfoList SourceCache::knownSources() const
{
  source::SourceInfoList sources;
  
  try {
    sqlite3_connection con(ZYPP_DB_FILE);

    {
      sqlite3_command cmd(con, "select * from sources;");
      sqlite3_reader reader = cmd.executereader();

      while(reader.read())
      {
        // alias 0 , type 1, desc 2, url 3, path 4, enabled 5, autorefresh 6, timestamp 7, checksum 8
        source::SourceInfo info;
        info.setAlias(reader.getstring(0));
        info.setType(reader.getstring(1));
        info.setDescription(reader.getstring(2));
        info.setUrl(reader.getstring(3));
        info.setPath(reader.getstring(4));
        info.setEnabled( int_to_tribool(reader.getint(5)) );
        info.setAutorefresh( int_to_tribool( reader.getint(6) ));
        //info.setTimestamp(Date(reader.getstring(7)));
        info.setChecksum(string_to_checksum(reader.getstring(8)));
        sources.push_back(info);
      }
    }
    con.close();
  }
  catch(exception &ex) {
    ERR << "Exception Occured: " << ex.what() << endl;
  }
  return sources;
}

void SourceCache::storeSource( const source::SourceInfo &info )
{
  try
  {
    sqlite3_connection con(ZYPP_DB_FILE);
    sqlite3_transaction trans(con);

    {
      // alias 0 , type 1, desc 2, url 3, path 4, enabled 5, autorefresh 6, timestamp 7, checksum 8
        
      sqlite3_command cmd(con, "insert into sources ( alias, type, description, url, path, enabled, autorefresh, timestamp, checksum ) values ( ?, ?, ?, ? , ?, ?, ?, ?, ?);");
      cmd.bind(0, info.alias());
      cmd.bind(1, info.type());
      cmd.bind(2, info.description());
      cmd.bind(3, info.url().asCompleteString());
      cmd.bind(4, info.path().asString());
      cmd.bind(5, tribool_to_int(info.enabled()) );
      cmd.bind(6, tribool_to_int(info.autorefresh()) );
      cmd.bind(7, info.timestamp().asString());
      cmd.bind(8, checksum_to_string(info.checksum()) );
      
      cmd.executenonquery();
    }

    trans.commit(); // note: if trans goes out of scope (due to an exception or anything else) before calling commit(), it will automatically rollback()

    con.close();
  }
  catch(exception &ex) {
    ERR << "Exception Occured: " << ex.what() << endl;
  }
} 

std::ostream & SourceCache::dumpOn( std::ostream & str ) const
{
  return str;
}

}
}

