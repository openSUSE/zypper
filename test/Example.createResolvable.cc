#include <iostream>
#include <zypp/base/Logger.h>

#include <zypp/detail/PackageImpl.h>
#include <zypp/Package.h>
#include <zypp/CapFactory.h>

using namespace std;
using namespace zypp;

// refine parsing and add it to lib
//
struct CapSetInsert : public std::unary_function<const char *, void>
{
  CapSet &   _x;
  CapFactory _f;
  CapSetInsert( CapSet & x )
  : _x(x)
  {}
  void operator()( const char * v )
  { _x.insert( _f.parse( ResKind(), v ) ); }
};

inline CapSet parseDeps()
{
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

  CapSet ret;
  for_each( begin, end, CapSetInsert(ret) );
  return ret;
}

/******************************************************************
**
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
**
**      DESCRIPTION :
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  // parse basic values
  std::string _name( "foo" );
  Edition     _edition( "1.0", "42" );
  Arch        _arch( "i386" );

  // create implementation class
  detail::PackageImplPtr pkgImpl( new detail::PackageImpl(_name,_edition,_arch) );

  // finalize construction
  Dependencies _deps;
  _deps.setProvides( parseDeps() );
  pkgImpl->setDeps( _deps );
  // ... aditional data if...

  // create Package
  constPackagePtr foo( new Package( pkgImpl ) );
  DBG << foo << endl;
  DBG << *foo << endl;
  DBG << foo->deps() << endl;


  INT << "===[END]============================================" << endl;
  return 0;
}

