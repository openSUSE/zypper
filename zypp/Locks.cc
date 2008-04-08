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

#include "zypp/base/Regex.h"
#include "zypp/base/String.h"
#include "zypp/base/Logger.h"
#include "zypp/base/IOStream.h"
#include "zypp/PoolItem.h"
#include "zypp/PoolQueryUtil.tcc"
#include "zypp/ZYppCallbacks.h"

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
// assign Lock to installed pool item
//

struct AddLockToPool
{
  AddLockToPool( const ResPool &pool )
  : _pool(pool)
  , _count(0)
  {

  }

  bool operator()( const std::string & str_r )
  {
#warning MUST FIX LOCK IMPLEMENTATION
    // - make Capability's parse 'Name [Op edition]' available so it can be used here
    // - provide new, or extend Capability::Matches, functor to allow pattern (glob/rx) matching
    return false;
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

class Locks::Impl
{
public:
  std::list<PoolQuery> locks;
};

void Locks::saveLocks( const Pathname& file )
{
  writePoolQueriesToFile( file, _pimpl->locks.begin(), _pimpl->locks.end() );
}

void Locks::loadLocks( const Pathname& file )
{
  _pimpl->locks.clear();
  insert_iterator<std::list<PoolQuery> > ii( _pimpl->locks,
      _pimpl->locks.end() );
  readPoolQueriesFromFile( file, ii );
}

void Locks::addLock( const PoolQuery& query )
{
  //XXX real lock it!
  _pimpl->locks.push_back( query );
}

bool Locks::existEmptyLocks()
{
  for_( it, _pimpl->locks.begin(), _pimpl->locks.end() )
  {
    if( it->empty() )
      return true;
  }

  return false;
}

//handle locks during removing
class LocksRemovePredicate{
private:
  bool skip_rest;
  size_t searched;
  size_t all;
  callback::SendReport<CleanEmptyLocksReport> &report;

public:
  LocksRemovePredicate(size_t count, callback::SendReport<CleanEmptyLocksReport> &_report): skip_rest(false),searched(0),all(count), report(_report){}

  bool aborted(){ return skip_rest; }

  bool operator()(PoolQuery& q)
  {
    if( skip_rest )
      return false;
    searched++;
    if( !q.empty() )
      return false;

    if (!report->progress((100*searched)/all))
    {
      skip_rest = true;
      return false;
    }

    switch (report->execute(q))
    {
    case CleanEmptyLocksReport::ABORT:
      report->finish(CleanEmptyLocksReport::ABORTED);
      skip_rest = true;
      return false;
    case CleanEmptyLocksReport::DELETE:
      return true;
    case CleanEmptyLocksReport::IGNORE:
      return false;
    default:
      WAR << "Unknown returned value. Callback have more value then"
          << " this switch. Need correct handle all enum values." << std::endl;
    }

    return false;
  }

};

void Locks::removeEmptyLocks()
{
  callback::SendReport<CleanEmptyLocksReport> report;
  report->start();
  size_t sum = _pimpl->locks.size();
  LocksRemovePredicate p(sum, report);

  _pimpl->locks.remove_if(p);

  if( p.aborted() )
  {
    report->finish(CleanEmptyLocksReport::ABORTED);
  }
  else 
  {
    report->finish(CleanEmptyLocksReport::NO_ERROR);

  }
}

Locks::Locks(){}


} // ns locks
} // ns zypp
