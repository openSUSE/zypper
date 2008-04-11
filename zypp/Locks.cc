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
#include <boost/function_output_iterator.hpp>

#include "zypp/base/Regex.h"
#include "zypp/base/String.h"
#include "zypp/base/Logger.h"
#include "zypp/base/IOStream.h"
#include "zypp/PoolItem.h"
#include "zypp/PoolQueryUtil.tcc"
#include "zypp/ZYppCallbacks.h"
#include "zypp/sat/SolvAttr.h"

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

Locks& Locks::instance()
{
  static Locks _instance;
  return _instance;
}

class Locks::Impl
{
public:
  std::list<PoolQuery> locks;
};

Locks::Locks() : _pimpl(new Impl){}

void Locks::saveLocks( const Pathname& file )
{
  writePoolQueriesToFile( file, _pimpl->locks.begin(), _pimpl->locks.end() );
}

/**
 * iterator that takes lock, lock all solvables from query 
 * and send query to output iterator
 */
template <class OutputIterator>
struct LockingOutputIterator
{
  LockingOutputIterator(OutputIterator& out_)
    : out(out_)
    {}

  void operator()(const PoolQuery& query) const
  {
    for_( it,query.begin(),query.end() )
    {
      PoolItem item(*it);
      item.status().setLock(true,ResStatus::USER);
    }
    
    *out++ = query;
  }
  
  private:
  OutputIterator& out;
 };

void Locks::loadLocks( const Pathname& file )
{
  insert_iterator<std::list<PoolQuery> > ii( _pimpl->locks,
      _pimpl->locks.end() );
  LockingOutputIterator<insert_iterator<std::list<PoolQuery> > > lout(ii);
  readPoolQueriesFromFile( file, boost::make_function_output_iterator(lout) );
}

void Locks::addLock( const PoolQuery& query )
{
  for_( it,query.begin(),query.end() )
  {
    PoolItem item(*it);
    item.status().setLock(true,ResStatus::USER);
  }
  _pimpl->locks.push_back( query );
}

void Locks::addLock(const ui::Selectable& selectable)
{
  PoolQuery q;
  q.addAttribute( sat::SolvAttr::name,selectable.name() );
  q.addKind( selectable.kind() );
  q.setMatchExact();
  q.setCaseSensitive(true);
  q.requireAll();
  addLock( q );
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

} // ns locks
} // ns zypp
