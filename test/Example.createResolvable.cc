#include <iostream>
#include <list>
#include <string>
#include <zypp/base/Logger.h>

#include <zypp/detail/PackageImpl.h>
#include <zypp/Package.h>
#include <zypp/CapFactory.h>

using namespace std;
using namespace zypp;

namespace zypp
{
  template<>
    Capability CapFactory::parse<Package>( const std::string & strval_r ) const
    { throw; return parse( strval_r, ResTraits<Package>::_kind ); }
}

// refine parsing and add it to lib
//
template<typename _Res>
  struct CapSetInsert : public std::unary_function<const std::string &, void>
{
  CapSet &   _x;
  CapFactory _f;
  CapSetInsert( CapSet & x )
  : _x(x)
  {}
  void operator()( const std::string & v )
  { _x.insert( _f.parse<_Res>( v ) ); }
};

inline std::list<std::string> parseDeps()
{
  const char * init[] = {
    "/bin/sh",
    "rpmlib(PayloadFilesHavePrefix) <= 4.0-1",
    "rpmlib(CompressedFileNames) <= 3.0.4-1",
    "/bin/sh",
    "",
    "libc.so.6",
    "libc.so.6(GLIBC_2.0)",
    "libc.so.6(GLIBC_2.3.4)",
    "libhd.so.11",
    "libsysfs.so.1",
    "rpmlib(PayloadIsBzip2) <= 3.0.5-1",
  };
  const char ** begin = init;
  const char ** end   = init + ( sizeof(init) / sizeof(const char *) );

  std::list<std::string> ret( begin, end );
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

  // create the implementation class
  detail::PackageImplPtr pkgImpl( new detail::PackageImpl(_name,_edition,_arch) );
  DBG << *pkgImpl << endl;
  DBG << pkgImpl->deps() << endl;

  // finalize implementation class construction
  Dependencies _deps;

  std::list<std::string> depList( parseDeps() );
  CapSet prv;
  try
    {
      for_each( depList.begin(), depList.end(), CapSetInsert<Package>(prv) );
    }
  catch(...)
    {
      INT << _deps << endl;
    }
  _deps.setProvides( prv );

  // ...parse other deps
  pkgImpl->setDeps( _deps );

  // ... aditional data if...

  // create the Package
  constPackagePtr foo( new Package( pkgImpl ) );
  DBG << foo << endl;
  DBG << *foo << endl;
  DBG << foo->deps() << endl;


  INT << "===[END]============================================" << endl;
  return 0;
}

