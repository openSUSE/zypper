
#ifndef ZYPP_TESTSUITE_TESTUTILS
#define ZYPP_TESTSUITE_TESTUTILS

#include "zypp/Dependencies.h"
#include "zypp/Capability.h"
#include "zypp/ResStore.h"

namespace zypp
{
  namespace testsuite
  {
    namespace utils
    {
      void dump( const ResStore &store, bool descr = false, bool deps = false );  
      void dump( const std::list<ResObject::Ptr> &list, bool descr = false, bool deps = false );  
    }  
  }
}

#endif

