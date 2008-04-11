
#ifndef ZYPP_LOCKS_H
#define ZYPP_LOCKS_H

#include "zypp/ResPool.h"
#include "zypp/Pathname.h"
#include "zypp/PoolQuery.h"
#include "zypp/ZConfig.h"
#include "zypp/ui/Selectable.h"

namespace zypp
{
  namespace locks
  {
    class Locks
    {
    public:
      class Impl;

      static Locks& instance();

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
      void unlock( const PoolQuery& query );

      void unlock( const ui::Selectable& selectable );

      void loadLocks( const Pathname& file = ZConfig::instance().locksFile() );

      void saveLocks( const Pathname& file = ZConfig::instance().locksFile() );

      bool existEmptyLocks();

      void removeEmptyLocks();

    private:
      Locks();
      
      RW_pointer<Impl, rw_pointer::Scoped<Impl> > _pimpl;

    };
  }
}
    
#endif

