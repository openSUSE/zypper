#ifndef ZYPP_LOCKS_H
#define ZYPP_LOCKS_H

#include "zypp/ResPool.h"
#include "zypp/Pathname.h"
#include "zypp/PoolQuery.h"
#include "zypp/ZConfig.h"
#include "zypp/ui/Selectable.h"
 
namespace zypp
{
  class Locks
  {
  public:
    typedef std::list<PoolQuery> LockList;
    typedef LockList::const_iterator iterator;
  public:
    class Impl;

    static Locks& instance();

    iterator begin();
    iterator end();
    LockList::size_type size();
    bool empty();

    /**
     * TODO add:
     * applied{Begin,End,Size,Empty}
     * toBeAdded{Begin,End,Size,Empty}
     * toBeRemoved{Begin,End,Size,Empty}
     * bool dirty();
     */

    /**
     * locks all solvables which is result of query
     */
    void addLock( const PoolQuery& query );

    /**
     * locks selectable
     */
    void addLock( const ui::Selectable& selectable );

    /**
     * unlocks all solvables which is result of query.
     * Can call callback
     */
    void removeLock( const PoolQuery& query );

    void removeLock( const ui::Selectable& selectable );

    void readAndApply( const Pathname& file = ZConfig::instance().locksFile() );

    void read( const Pathname& file = ZConfig::instance().locksFile() );

    void apply();

    void save( const Pathname& file = ZConfig::instance().locksFile() );
    
    bool existEmpty();

    void removeEmpty();

  private:
    Locks();
    
    RW_pointer<Impl, rw_pointer::Scoped<Impl> > _pimpl;

  };
}
    
#endif
