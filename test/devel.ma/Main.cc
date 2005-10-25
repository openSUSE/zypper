#include <iostream>
#include <iterator>
#include <functional>
#include <algorithm>
#include <set>
#include <zypp/base/Logger.h>
#include <zypp/Capability.h>
#include <zypp/CapFactory.h>
#include <zypp/CapSet.h>
#include "main.h"

#define TAG INT << __PRETTY_FUNCTION__ << std::endl

namespace zypp
{
}

using namespace std;
using namespace zypp;
#include <zypp/ResKind.h>
/******************************************************************
**
**
**	FUNCTION NAME : main
**	FUNCTION TYPE : int
**
**	DESCRIPTION :
*/
struct CapSetInsert : public std::unary_function<const char *, void> {
    CapSet &   _x;
    CapFactory _f;
    CapSetInsert( CapSet & x ) : _x(x) {}
    void operator()( const char * v )
    { _x.insert( _f.parse( ResKind(), v ) ); }
};

int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  const char * init[] = {
    "/bin/sh",
    "rpmlib(PayloadFilesHavePrefix) <= 4.0-1",
    "rpmlib(CompressedFileNames) <= 3.0.4-1",
    "/bin/sh",
    "libc.so.6",
    "libc.so.6(GLIBC_2.0)",
    "libc.so.6(GLIBC_2.3.4)",
    "libhd.so.11",
    "libsysfs.so.1",
    "rpmlib(PayloadIsBzip2) <= 3.0.5-1",
  };
  const char ** begin = init;
  const char ** end   = init + ( sizeof(init) / sizeof(const char *) );

  CapSet x;
  CapSetInsert a( x );

  for_each( begin, end, CapSetInsert(x) );

  copy( x.begin(), x.end(), ostream_iterator<Capability>(SEC,"\n") );

  INT << "===[END]============================================" << endl;
  return 0;
}

