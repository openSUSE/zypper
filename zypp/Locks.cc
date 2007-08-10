/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <set>
#include <fstream>
#include <boost/function.hpp>

#include <zypp/base/Regex.h>
#include <zypp/base/String.h>
#include "zypp/base/Logger.h"
#include "zypp/base/IOStream.h"
#include "zypp/PoolItem.h"
#include "zypp/CapFactory.h"
#include "zypp/CapMatchHelper.h"
#include "zypp/capability/Capabilities.h"

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "locks"

#include "zypp/Locks.h"
#include "zypp/PathInfo.h"

using namespace std;
using namespace zypp;
using namespace zypp::str;

namespace zypp
{
namespace locks
{

//
// collect matching names
//
// called by regexp matching, see 'Match' below
//

struct NameMatchCollectorFunc
{
  set<string> matches;

  bool operator()( const PoolItem &item )
  {
    matches.insert( item.resolvable()->name() );
    return true;
  }
};


// taken from zypper
struct Match
{
  const regex * _regex;

  Match(const regex & regex ) :
    _regex(&regex)
  {}

  bool operator()(const zypp::PoolItem & pi) const
  {
    return
    // match resolvable name
    regex_match(pi.resolvable()->name(), *_regex);
  }
};


string
wildcards2regex(const string & str)
{
  string regexed;

  string all("*"); // regex to search for '*'
  string one("?"); // regex to search for '?'
  string r_all(".*"); // regex equivalent of '*'
  string r_one(".");  // regex equivalent of '?'

  // replace all "*" in input with ".*"
  regexed = str::gsub( str, all, r_all );
  MIL << "wildcards2regex: " << str << " -> " << regexed;

  // replace all "?" in input with "."
   regexed = str::gsub(regexed, one, r_one);
   MIL << " -> " << regexed << endl;

  return regexed;
}


//
// assign Lock to installed pool item
//

struct ItemLockerFunc
{
  ItemLockerFunc( const string lock_str )
    : _lock_str(lock_str)
  {}

  bool operator()( const CapAndItem &cai_r )
  {
    PoolItem_Ref item(cai_r.item);
    MIL << "Locking " << cai_r.item << "(matched by " << _lock_str << ")" << endl;
    item.status().setLock( true, ResStatus::USER);
    return true;
  }

  string _lock_str;
};

struct AddLockToPool
{
  AddLockToPool( const ResPool &pool )
  : _pool(pool)
  , _count(0)
  {
  
  }
  
  bool operator()( const std::string & str_r )
  {
    CapFactory cap_factory;
    
    std::string line( str::trim( str_r ) );
    
    if ( line.empty() || line[0] == '#')
      return true;
    
    MIL << "Applying locks from pattern '" << str_r << "'" << endl;
    
    // zypp does not provide wildcard or regex support in the Capability matching helpers
    // but it still parses the index if it contains wildcards.
    // so we decompose the capability, and keep the op and edition, while, the name
    // is converted to a regexp and matched against all possible names in the _pool
    // Then these names are combined with the original edition and relation and we
    // got a new capability for matching wildcard to use with the capability match
    // helpers

    Rel rel;
    Edition edition;
    string name;

    try
    {
      Capability capability = cap_factory.parse( ResTraits<zypp::Package>::kind, line );
      
      capability::NamedCap::constPtr named = capability::asKind<capability::NamedCap>(capability);
      if ( named )
      {
        rel = named->op();
        edition = named->edition();
        name = named->index();
      }
      else
      {
        ERR << "Not a named capability in: '" << line << "' skipping" << std::endl;
        return true;
      }
    }
    catch ( const Exception &e )
    {
      ERR << "Can't parse capability in: '" << line << "' (" << e.msg() << ") skipping" << std::endl;
      return true;
    }

    // Operator NONE is not allowed in Capability
    if (rel == Rel::NONE) rel = Rel::ANY;

    NameMatchCollectorFunc nameMatchFunc;

    // regex flags
    unsigned int flags = regex::normal;
    flags |= regex::icase;
    regex reg;

    // create regex object
    string regstr( wildcards2regex( name ) );
    MIL << "regstr '" << regstr << "'" << endl;
    try
    {
      reg.assign( regstr, flags );
    }
    catch (regex_error & e)
    {
      ERR << "locks: " << regstr << " is not a valid regular expression: \"" << e.msg() << "\"" << endl;
      ERR << "This is a bug, please file a bug report against libzypp" << endl;
      // ignore this lock and continue
      return true;;
    }

    invokeOnEach( _pool.begin(), _pool.end(), Match(reg), functor::functorRef<bool, const PoolItem &>(nameMatchFunc) );

    MIL << "Found " << nameMatchFunc.matches.size() << " matches." << endl;

    // now we have all the names matching

    // for each name matching try to match a capability

    ItemLockerFunc lockItemFunc( line );

    for ( set<string>::const_iterator it = nameMatchFunc.matches.begin(); it != nameMatchFunc.matches.end(); ++it )
    {
      string matched_name = *it;

      try
      {
        Capability capability = cap_factory.parse( ResTraits<zypp::Package>::kind, matched_name, rel, edition );
        MIL << "Locking capability " << capability << endl;
        forEachMatchIn( _pool, Dep::PROVIDES, capability, functor::functorRef<bool, const CapAndItem &>(lockItemFunc) );
      }
      catch ( const Exception &e )
      {
        ERR << "Invalid lock: " << e.msg() << std::endl;
      }
      ++_count;
    }
    return true;
  } // end operator()()
        
  ResPool _pool;
  int _count;
};

//
// read 'locks' table, evaluate 'glob' column, assign locks to pool
//
int
readLocks(const ResPool & pool, const Pathname &file )
{
  PathInfo lockrc( file );
  if ( lockrc.isFile() )
  {
    MIL << "Reading " << lockrc << endl;
    ifstream inp( file.c_str() );
    AddLockToPool addlock(pool);
    iostr::forEachLine( inp, addlock);
    MIL << addlock._count << " locks." << endl;
    return addlock._count;
  }
  return 0;
}

} // ns locks
} // ns zypp
