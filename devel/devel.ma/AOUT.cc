#include <iostream>
#include <fstream>
#include <string>
#include <list>

#include <zypp/base/Logger.h>
#include <zypp/base/LogTools.h>
#include <zypp/PackageKeyword.h>

using std::endl;
using namespace zypp;

///////////////////////////////////////////////////////////////////
namespace parse
{ /////////////////////////////////////////////////////////////////


  /////////////////////////////////////////////////////////////////
} // namespace parse
///////////////////////////////////////////////////////////////////


/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  DBG << "===[START]==========================================" << endl;

  const char * _l[] = { "a", "b", "c", "a", "b", "c" };
  std::list<std::string> l( _l, _l+(sizeof(_l)/sizeof(*_l)) );
  DBG << l << endl;

  std::list<PackageKeyword> k( l.begin(), l.end() );
  DBG << k << endl;

  dumpRange( MIL, PackageKeyword::allBegin(), PackageKeyword::allEnd() );


  DBG << "===[END]============================================" << endl;
  zypp::base::LogControl::instance().logNothing();
  return 0;
}

