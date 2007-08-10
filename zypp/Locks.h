
#ifndef ZYPP_LOCKS_H
#define ZYPP_LOCKS_H

#include "zypp/ResPool.h"
#include "zypp/Pathname.h"

namespace zypp
{
  namespace locks
  {

    int readLocks(const ResPool & pool, const Pathname &file );
  }
}
    
#endif

