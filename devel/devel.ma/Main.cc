#include <iostream>

#include "zypp/base/Easy.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/InputStream.h"
#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/TmpPath.h"
#include "zypp/RepoManager.h"

using std::endl;
using namespace zypp;

///////////////////////////////////////////////////////////////////

static const Pathname sysRoot( getenv("SYSROOT") ? getenv("SYSROOT") : "/Local/ROOT" );

///////////////////////////////////////////////////////////////////

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  //TmpDir tmp_dir;
  //KeyRing keyring( tmp_dir.path() );

  {
    Measure x( "INIT TARGET" );
    {
      {
        //zypp::base::LogControl::TmpLineWriter shutUp;
        getZYpp()->initializeTarget( sysRoot );
      }
      //getZYpp()->target()->load();
    }
  }


  INT << "===[END]============================================" << endl << endl;
  return 0;
}
