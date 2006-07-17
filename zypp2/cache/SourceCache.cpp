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
#include "zypp2/cache/SourceCache.h"
#include "zypp/target/store/PersistentStorage.h"
#include "zypp2/cache/SourceCacheInitializer.h"
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
  

  
// alias 0 , type 1, desc 2, url 3, path 4, enabled 5, autorefresh 6, timestamp 7, checksum 8
  
SourceCache::SourceCache( const Pathname &root_r, const std::string alias )
{
  try
  {
    SourceCacheInitializer init( root_r, ZYPP_DB_FILE);
    _con.reset( new sqlite3_connection(ZYPP_DB_FILE) );
  }
  catch(exception &ex) {
    ERR << "Exception Occured: " << ex.what() << endl;
  } 
}

SourceCache::~SourceCache()
{
  _con->close();
}

void SourceCache::cachePackage( const data::Package package )
{
  MIL << "caching: " << package << std::endl;
}

void SourceCache::cachePattern( const data::Pattern pattern )
{

}

void SourceCache::cacheResolvable( const data::ResObject )
{

}

std::ostream & SourceCache::dumpOn( std::ostream & str ) const
{
  return str;
}

}
}

