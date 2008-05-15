#ifndef ZYPP_LOCKS_H
#define ZYPP_LOCKS_H

#include "zypp/ResPool.h"
#include "zypp/Pathname.h"
#include "zypp/PoolQuery.h"
#include "zypp/ZConfig.h"
 
namespace zypp
{
  /** \name Locks */
  //@{
  /**
   * Singleton class which manipulate with locks file and apply locks on pool.
   * for user information about locksfile and its format see 
   * <a>http://en.opensuse.org/Libzypp/Locksfile</a>
   */
  class Locks
  {
  public:
    typedef std::list<PoolQuery> LockList;
    typedef LockList::const_iterator const_iterator;
    typedef LockList::size_type size_type;
  public:
    class Impl;

    /**
     * Gets instance of this class.
     * Singleton method.
     */
    static Locks& instance();

    const_iterator begin() const;
    const_iterator end() const;
    LockList::size_type size() const;
    bool empty() const;

    /**
     * TODO add:
     * toBeAdded{Begin,End,Size,Empty}
     * toBeRemoved{Begin,End,Size,Empty}
     */

    /**
     * locks result of query and add this lock as toAdd
     */
    void addLock( const PoolQuery& query );

    /**
     * add lock by identifier (e.g. Selectable->ident()
     * and add this lock as toAdd
     */
    void addLock( const IdString& ident_r );

    /**
     * add lock by name and kind and
     * add this lock as toAdd
     */ 
    void addLock( const ResKind& kind_r, const IdString& name_r );

    /**
     * add lock by name and kind and
     * add this lock as toAdd
     */ 
    void addLock( const ResKind& kind_r, const C_Str& name_r );

    /**
     * unlocks by result of query and add to toRemove.
     *
     * If unlock non-saved lock (so he is in toAdd list) then both is deleted 
     * and nathing happen during save
     */
    void removeLock( const PoolQuery& query );

    /**
     * remove lock by identifier (e.g. Selectable->ident()
     *
     * If unlock non-saved lock (so he is in toAdd list) then both is deleted 
     * and nathing happen during save
     */
    void removeLock( const IdString& ident_r );

    /**
     * remove lock by name and kind
     *
     * If unlock non-saved lock (so he is in toAdd list) then both is deleted 
     * and nathing happen during save
     */
    void removeLock( const ResKind& kind_r, const IdString& name_r );
    void removeLock( const ResKind& kind_r, const C_Str & name_r );

    /**
     * Optimalized version of read and apply.
     * \see read
     * \see apply
     */
    void readAndApply( const Pathname& file = ZConfig::instance().locksFile() );

    /**
     * Read locks from file to list of stable locks (locks which is not changed
     * during session)
     */
    void read( const Pathname& file = ZConfig::instance().locksFile() );

    /**
     * Applies locks in stable list (locks which is not changed during session).
     */
    void apply() const;

    /**
     * Merges toAdd and ToRemove list to stable list and
     * save that stable list to file.
     * \see SavingLocksReport
     */
    void save( const Pathname& file = ZConfig::instance().locksFile() );

    /**
     * Merges toAdd and ToRemove list to stable list.
     * \note Can call callback if problem during merging occure
     * \see SavingLocksReport
     */
    void merge();
    
    /**
     * Gets true if some lock doesn't lock any object in pool
     * This can happen e.g. if package is removed or
     * due to user bad definition of lock
     */
    bool existEmpty() const;

    /**
     * Call callback for each empty lock.
     * \see existEmpty
     * \see CleanEmptyLocksReport
     * \note you must call \a save to write cleaned locks to file
     */
    void removeEmpty();

    /**
     * Delete all query duplicate in loaded locks.
     * \note you must call \a save to write cleaned locks to file
     */
    void removeDuplicates();
    //@}
  private:
    Locks();
    
    RW_pointer<Impl, rw_pointer::Scoped<Impl> > _pimpl;

  };
}
    
#endif
