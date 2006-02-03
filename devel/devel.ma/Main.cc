#include <ctime>

#include <iostream>
#include <list>
#include <map>
#include <set>

#include <zypp/base/Logger.h>
#include <zypp/base/String.h>
#include <zypp/base/Exception.h>
#include <zypp/base/PtrTypes.h>
#include <zypp/base/Iterator.h>
#include <zypp/base/Algorithm.h>

#include <zypp/PathInfo.h>
#include <zypp/SourceFactory.h>
#include <zypp/source/Builtin.h>
#include <zypp/source/susetags/SuseTagsImpl.h>

#include <zypp/Resolvable.h>
#include <zypp/Package.h>
#include <zypp/detail/PackageImpl.h>
#include <zypp/Selection.h>
#include <zypp/detail/SelectionImpl.h>
#include <zypp/Patch.h>
#include <zypp/detail/PatchImpl.h>

#include <zypp/CapFactory.h>

#include <zypp/ResFilters.h>
#include <zypp/ResStatus.h>
#include <zypp/ResPoolManager.h>

#include <zypp/ZYppFactory.h>
#include <zypp/Callback.h>

using namespace std;
using namespace zypp;
using namespace zypp::functor;
using namespace zypp::resfilter;

///////////////////////////////////////////////////////////////////
namespace zypp
{

}
///////////////////////////////////////////////////////////////////

namespace zypp
{
  struct Foo : public callback::ReportBase
  {
    virtual void ping( int i )
    {}
    virtual int pong()
    { return -1; }
  };

  struct FooRec : public callback::ReceiveReport<Foo>
  {
    FooRec() : _i( -1 ) {}
    virtual void ping( int i )
    { _i = i; }
    virtual int pong()
    { return _i; }
    int _i;
  };

  struct FooRec2 : public callback::ReceiveReport<Foo>
  {
    FooRec2() : _i( -1 ) {}
    virtual void ping( int i )
    { _i = i; }
    virtual int pong()
    { return 2*_i; }
    int _i;
  };
}
///////////////////////////////////////////////////////////////////


using namespace zypp;

void ping()
{
  callback::SendReport<Foo> r;
  r->ping( 13 );
  int res = r->pong();
  MIL << "PingPong: 13 -> " << res << endl;
}

namespace zypp { namespace callback
{


}}

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;


  FooRec  r;
  FooRec2 r2;
  r2.connect();
  ping();
  {
    callback::TempConnect<Foo> temp( r );
    ping();
  }

  ping();

  INT << "===[END]============================================" << endl << endl;
  return 0;
}

