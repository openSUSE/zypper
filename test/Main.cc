#include <iostream>
#include <string>
#include <zypp/base/Logger.h>
#include <zypp/detail/PackageImpl.h>
#include <zypp/Package.h>

using namespace std;
using namespace zypp;
using namespace base;

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

  ResName    _name( "foo" );
  ResEdition _edition( "1.0", "42" );
  ResArch    _arch( "i386" );


  detail::PackageImplPtr pi( new detail::PackageImpl(_name,_edition,_arch) );
  DBG << pi << endl;
  DBG << *pi << endl;
  constPackagePtr foo( new Package( pi ) );
  DBG << foo << endl;
  DBG << *foo << endl;


  detail::constPackageImplPtr c( pi );
  detail::PackageImplPtr cc( const_pointer_cast<detail::PackageImpl>(c) );


  INT << "===[END]============================================" << endl;
  return 0;
}
