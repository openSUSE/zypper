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
#include "zypp/PathInfo.h"

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "locks"

#include "zypp/Locks.h"

using namespace std;
using namespace zypp;
using namespace zypp::str;

namespace zypp
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
  std::list<PoolQuery> toAdd;
  std::list<PoolQuery> toRemove;

  bool mergeList(callback::SendReport<SavingLocksReport>& report);
  
};

Locks::Locks() : _pimpl(new Impl){}

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
  std::list<PoolQuery>::iterator i = find(_pimpl->toRemove.begin(),
    _pimpl->toRemove.end(), query);
  if ( i != _pimpl->toRemove.end() )
  {
    DBG << "query removed from toRemove" << endl;
    _pimpl->toRemove.erase(i);
  }
  else
  {
    DBG << "query added as new" << endl;
    _pimpl->toAdd.push_back( query );
  }
}

void Locks::addLock(const ui::Selectable& selectable)
{
  PoolQuery q;
  q.addAttribute( sat::SolvAttr::name,selectable.name() );
  q.addKind( selectable.kind() );
  q.setMatchExact();
  q.setCaseSensitive(true);
  addLock( q );
}

void Locks::unlock( const PoolQuery& query )
{
  for_( it,query.begin(),query.end() )
  {
    PoolItem item(*it);
    item.status().setLock(false,ResStatus::USER);
  }
  
  std::list<PoolQuery>::iterator i = find(_pimpl->toAdd.begin(),
    _pimpl->toAdd.end(), query);
  if ( i != _pimpl->toAdd.end() )
  {
    DBG << "query removed from added" << endl;
    _pimpl->toAdd.erase(i);
  }
  else
  {
    DBG << "needed remove some old lock" << endl;
    _pimpl->toRemove.push_back( query );
  }
}

void Locks::unlock( const ui::Selectable& s )
{
  PoolQuery q;
  q.addAttribute( sat::SolvAttr::name,s.name() );
  q.addKind( s.kind() );
  q.setMatchExact();
  q.setCaseSensitive(true);
  q.requireAll();
  unlock(q);
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
class LocksCleanPredicate{
private:
  bool skip_rest;
  size_t searched;
  size_t all;
  callback::SendReport<CleanEmptyLocksReport> &report;

public:
  LocksCleanPredicate(size_t count, callback::SendReport<CleanEmptyLocksReport> &_report): skip_rest(false),searched(0),all(count), report(_report){}

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
  LocksCleanPredicate p(sum, report);

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

class LocksRemovePredicate
{
private:
  std::set<sat::Solvable>& solvs;
  const PoolQuery& query;
  callback::SendReport<SavingLocksReport>& report;
  bool aborted_;

  //1 for subset of set, 2 only intersect, 0 for not intersect
  int contains(const PoolQuery& q, std::set<sat::Solvable>& s)
  {
    bool intersect = false;
    for_( it,q.begin(),q.end() )
    {
      if ( s.find(*it)!=s.end() )
      {
        intersect = true;
      }
      else
      {
        if (intersect)
          return 2;
      }
    }
    return intersect ? 1 : 0;
  }

public:
  LocksRemovePredicate(std::set<sat::Solvable>& s, const PoolQuery& q,
      callback::SendReport<SavingLocksReport>& r)
      : solvs(s), query(q),report(r),aborted_(false) {}

  bool operator()(const PoolQuery& q)
  {
    if (aborted())
      return false;
    if( q==query )
    {//identical
      DBG << "identical queries" << endl;
      return true;
    }

    SavingLocksReport::ConflictState cs;
    switch( contains(q,solvs) )
    {
    case 0:
      return false;
    case 1:
      cs = SavingLocksReport::SAME_RESULTS;
      break;
    case 2:
      cs = SavingLocksReport::INTERSECT;
    default:
      return true;
    }
    switch (report->conflict(q,cs))
    {
    case SavingLocksReport::ABORT:
      aborted_ = true;
      DBG << "abort merging" << endl;
      return false;
    case SavingLocksReport::DELETE:
      DBG << "force delete" << endl;
      return true;
    case SavingLocksReport::IGNORE:
      DBG << "skip lock" << endl;
      return false;
    }
    WAR << "should not reached, some state is missing" << endl;
    return false;
  }

  bool aborted(){ return aborted_; }
};

bool Locks::Impl::mergeList(callback::SendReport<SavingLocksReport>& report)
{
  for_(it,toRemove.begin(),toRemove.end())
  {
    if (!report->progress())
      return false;
    std::set<sat::Solvable> s(it->begin(),it->end());
    locks.remove_if(LocksRemovePredicate(s,*it, report));
  }

  if (!report->progress())
    return false;
  locks.insert(locks.end(),toAdd.begin(),toAdd.end());
  return true;
}

void Locks::saveLocks( const Pathname& file )
{
  callback::SendReport<SavingLocksReport> report;
  report->start();
  if (!_pimpl->mergeList(report))
  {
    report->finish(SavingLocksReport::ABORTED);
    return;
  }
  DBG << "writed "<< _pimpl->locks.size() << "locks" << endl;
  writePoolQueriesToFile( file, _pimpl->locks.begin(), _pimpl->locks.end() );
  report->finish(SavingLocksReport::NO_ERROR);
}

} // ns zypp
