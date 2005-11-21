#include <iostream>
#include <iterator>
#include <functional>
#include <set>
#include <algorithm>
#include <zypp/base/Logger.h>
#include <zypp/Arch.h>

using namespace std;
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

  Arch a;
  DBG << a << endl;
  DBG << a.compatibleWith( Arch("i386") ) << endl;
  DBG << a.compatibleWith( Arch("foo") ) << endl;

  set<Arch> s;
  s.insert( Arch_i386 );
  s.insert( Arch_noarch );
  s.insert( Arch("FOO") );
  copy( s.begin(), s.end(), ostream_iterator<Arch>(SEC,"\n") );

  INT << "===[END]============================================" << endl;
  return 0;
}

