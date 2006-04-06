#include <ctime>

#include <iostream>
#include <list>
#include <map>
#include <set>

#include "Measure.h"
#include "Printing.h"
#include "Tools.h"

#include <zypp/base/Logger.h>
#include <zypp/base/LogControl.h>
#include <zypp/base/String.h>
#include <zypp/base/Exception.h>
#include <zypp/base/PtrTypes.h>
#include <zypp/base/Iterator.h>
#include <zypp/base/Algorithm.h>
#include <zypp/base/Functional.h>
#include <zypp/base/ProvideNumericId.h>
#include <zypp/base/Debug.h>

#include "zypp/NVRAD.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"
#include "zypp/CapFactory.h"
#include "zypp/Package.h"
#include "zypp/Language.h"
#include "zypp/VendorAttr.h"

#include <zypp/SourceManager.h>
#include <zypp/SourceFactory.h>
#include <zypp/source/susetags/SuseTagsImpl.h>

#include "zypp/ZYppFactory.h"
#include "zypp/ResPoolProxy.h"
#include "zypp/ResPoolProxy.h"
#include <zypp/target/rpm/RpmDb.h>

using namespace std;
using namespace zypp;
using namespace zypp::base;

///////////////////////////////////////////////////////////////////
static const Url      instSrc( "dir:/Local/SLES10" );
///////////////////////////////////////////////////////////////////

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  Measure x;
  Source_Ref src( SourceFactory().createFrom( Url("dir:/Local/SLES10"),
                                              "/",
                                              Date::now().asSeconds() ) );
  src.resolvables();
  MIL << src.resolvables() << endl;
  MIL << CapFactory() << endl;

  INT << "===[END]============================================" << endl << endl;
  return 0;
}

