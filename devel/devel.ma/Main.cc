#include <iostream>
#include <iterator>
#include <functional>
#include <set>
#include <algorithm>
#include <zypp/base/Logger.h>
#include <zypp/Arch.h>
#include <zypp/Edition.h>
#include <zypp/Rel.h>

#include <boost/regex.hpp>


using namespace std;
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


  INT << "===[END]============================================" << endl;
  return 0;
}

