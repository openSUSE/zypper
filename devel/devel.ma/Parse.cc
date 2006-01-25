#include "zypp/base/Algorithm.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"

#include "zypp/ResPoolManager.h"

namespace zypp
{
  struct OnCapMatchCallback : public resfilter::OnCapMatchCallbackFunctor
  {
    bool operator()( ResObject::constPtr p, const Capability & match ) const
    {
      return true;
    }

    // func pointer
    // void *
  };

  int test( ResPool query )
  {
    Dep                 dep( Dep::PROVIDES );
    Capability          cap;
    OnCapMatchCallback  fnc;

    int ret
    = invokeOnEach( query.byCapabilityIndexBegin( cap.index(), dep ), // begin()
                    query.byCapabilityIndexEnd( cap.index(), dep ),   // end()
                    resfilter::callOnCapMatchIn( dep, cap, fnc ) );   // Action(ResObject::constPtr)
    return ret;
  }


}

int main()
{
  zypp::ResPoolManager pool;
  zypp::test( pool.accessor() );

  return 0;
}

