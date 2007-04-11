#include "Tools.h"
#include "FakePool.h"

#include "zypp/base/Exception.h"
#include "zypp/base/InputStream.h"
#include "zypp/base/DefaultIntegral.h"
#include <zypp/base/Function.h>
#include <zypp/base/Iterator.h>

#include "zypp/ZYppFactory.h"
#include "zypp/ResPoolProxy.h"
#include <zypp/SourceManager.h>
#include <zypp/SourceFactory.h>

#include "zypp/ZYppCallbacks.h"
#include "zypp/NVRAD.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"
#include "zypp/Package.h"
#include "zypp/Pattern.h"
#include "zypp/Language.h"
#include "zypp/PackageKeyword.h"
#include "zypp/NameKindProxy.h"
#include "zypp/pool/GetResolvablesToInsDel.h"

using namespace std;
using namespace zypp;

///////////////////////////////////////////////////////////////////

template<class _Iterator>
    void addPool( _Iterator begin_r, _Iterator end_r )
{
  using zypp::debug::DataCollect;
  DataCollect dataCollect;
  dataCollect.collect( begin_r, end_r );
  getZYpp()->addResolvables( dataCollect.installed(), true );
  getZYpp()->addResolvables( dataCollect.available() );
  vdumpPoolStats( USR << "Pool:" << endl,
		  getZYpp()->pool().begin(),
		  getZYpp()->pool().end() ) << endl;
}

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  const char * data[] = {
    "@ package",
    "@ installed",
    "@ available",
    "- glibc 2.3.4 31 i686",
    "- glibc 2.3.5 28 i586",
    "- glibc-locale 2.3.5 28 i686",
    "@ requires",
    "glibc == 2.3.5",
    "@ fin"
  };
  addPool( data, data + ( sizeof(data) / sizeof(const char *) ) );

  ResPool pool( getZYpp()->pool() );

  vdumpPoolStats( USR << "Pool:" << endl,
		  getZYpp()->pool().begin(),
		  getZYpp()->pool().end() ) << endl;

 // solve();

  vdumpPoolStats( USR << "Pool:" << endl,
		  getZYpp()->pool().begin(),
		  getZYpp()->pool().end() ) << endl;

  INT << "===[END]============================================" << endl << endl;
  return 0;
}

