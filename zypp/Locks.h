#ifndef ZYPP_LOCKS_H
#define ZYPP_LOCKS_H

#include "zypp/ResPool.h"
#include "zypp/Pathname.h"
#include "zypp/PoolQuery.h"
#include "zypp/ZConfig.h"
 
namespace zypp
{
  class Locks
  {
  public:
    typedef std::list<PoolQuery> LockList;
    typedef LockList::const_iterator const_iterator;
  public:
    class Impl;

    static Locks& instance();

    const_iterator begin() const;
    const_iterator end() const;
    LockList::size_type size() const;
    bool empty() const;

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
     * add lock by identifier (f.e. Selectable->ident()
     */
    void addLock( const IdString& ident_r );

    /**
     * add lock by name and kind
     */ 
    void addLock( const ResKind& kind_r, const IdString& name_r );
    void addLock( const ResKind& kind_r, const C_Str& name_r );

    /**
     * unlocks all solvables which is result of query.
     * Can call callback
     */
    void removeLock( const PoolQuery& query );

    /**
     * add lock by identifier (f.e. Selectable->ident()
     */
    void removeLock( const IdString& ident_r );

    /**
     * add lock by name and kind
     */ 
    void removeLock( const ResKind& kind_r, const IdString& name_r );
    void removeLock( const ResKind& kind_r, const C_Str & name_r );

    void readAndApply( const Pathname& file = ZConfig::instance().locksFile() );

    void read( const Pathname& file = ZConfig::instance().locksFile() );

    void apply() const;

    void save( const Pathname& file = ZConfig::instance().locksFile() );
    
    bool existEmpty() const;

    void removeEmpty();

  private:
    Locks();
    
    RW_pointer<Impl, rw_pointer::Scoped<Impl> > _pimpl;

  };
}
    
#endif
