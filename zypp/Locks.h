
#ifndef ZYPP_LOCKS_H
#define ZYPP_LOCKS_H

#include "zypp/ResPool.h"
#include "zypp/Pathname.h"
#include "zypp/PoolQuery.h"
#include "zypp/ZConfig.h"

namespace zypp
{
  namespace locks
  {

    int readLocks(const ResPool & pool, const Pathname &file );

    class Locks
    {
    public:
      class Impl;

      static Locks instance;

      /**
       * locks all solvables which is result of query
       */
      void addLock( const PoolQuery& query );

      /**
       * unlocks all solvables which is result of query.
       * Can call callback
       */
      void unlock( const PoolQuery& query );

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

