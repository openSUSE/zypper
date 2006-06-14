#include <iostream>

#include "zypp/base/LogControl.h"
#include "zypp/base/LogTools.h"
#include <zypp/base/Logger.h>

#include "zypp/Source.h"
#include "zypp/Package.h"
#include "zypp/detail/ImplConnect.h"

using namespace std;
using namespace zypp;

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
namespace zypp
{
  Pathname testProvidePackage( Package::constPtr package )
  {
    // direct implementation access
    typedef detail::ResImplTraits<Package::Impl>::constPtr PackageImpl_constPtr;
    PackageImpl_constPtr implPtr( detail::ImplConnect::resimpl( package ) );

    SEC << package << endl;
    SEC << implPtr << endl;
    MIL << implPtr->deltaRpms() << endl;
    MIL << implPtr->patchRpms() << endl;

    return Pathname();
  }
}
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

#include "Tools.h"
#include "zypp/ZYppFactory.h"

void test( const ResObject::constPtr & res )
{
  if ( ! isKind<Package>( res ) )
    return;

  SEC << "Test " << res << endl;

  if ( ! res->source() )
    return;

  try
    {
      MIL << zypp::testProvidePackage( asKind<Package>( res ) ) << endl;
    }
  catch ( Exception & expt )
    {
      ERR << expt << endl;
    }
}

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  //zypp::base::LogControl::instance().logfile( "log.restrict" );
  INT << "===[START]==========================================" << endl;

  ResPool pool( getZYpp()->pool() );

  Source_Ref src( createSource( "dir:////Local/PATCHES" ) );
  getZYpp()->addResolvables( src.resolvables() );

  MIL << pool << endl;

  dumpRange( MIL, pool.byNameBegin("glibc"), pool.byNameEnd("glibc") ) << endl;

  std::for_each( pool.byNameBegin("glibc"), pool.byNameEnd("glibc"), test );

  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::instance().logNothing();
  return 0;
}

