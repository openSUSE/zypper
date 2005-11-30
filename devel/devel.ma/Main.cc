#include <iostream>
#include <iterator>
#include <functional>
#include <set>
#include <algorithm>
#include <zypp/base/Logger.h>
#include <zypp/base/String.h>
#include <zypp/base/ReferenceCounted.h>
#include <zypp/Arch.h>
#include <zypp/Edition.h>
#include <zypp/Rel.h>
#include <zypp/Capability.h>

using namespace std;

#define TAG INT << __PRETTY_FUNCTION__ << endl

template<class _C>
  void outc( const _C & cont, ostream & str )
  {
    copy( cont.begin(), cont.end(),
          ostream_iterator<typename _C::value_type>(str,"\n") );
  }

namespace zypp
{
}

using namespace zypp;

/******************************************************************
**
**
**	FUNCTION NAME : main
**	FUNCTION TYPE : int
**
**	DESCRIPTION :
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  try
    {
      try
        {
          ZYPP_THROW( "Something bad happened." );
        }
      catch ( Exception & excpt )
        {
          ZYPP_RETHROW( excpt );
        }

    }
  catch ( Exception & excpt )
    {
      ZYPP_CAUGHT( excpt );
    }

  INT << "===[END]============================================" << endl;
  return 0;
}

