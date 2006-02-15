#include <iostream>
#include <list>
#include <string>
#include <zypp/base/Logger.h>

#include <zypp/Package.h>
#include <zypp/detail/PackageImpl.h>
#include <zypp/CapFactory.h>
#include <zypp/CapSet.h>

using namespace std;
using namespace zypp;

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
    { _x.insert( _f.parse( ResTraits<_Res>::kind, v ) ); }
  };

inline std::list<std::string> parseDeps()
{
  const char * init[] = {
    "xextra:/usr/X11R6/bin/Xvfb"
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

  // Collect basic Resolvable data
  NVRAD dataCollect;

  // parse basic values
  dataCollect.name    = "foo";
  dataCollect.edition = Edition("1.0","42");
  dataCollect.arch    = Arch_i386;

  // parse dependencies
  std::list<std::string> depList( parseDeps() );
  try
    {
      for_each( depList.begin(), depList.end(),
                CapSetInsert<Package>(dataCollect[Dep::PROVIDES]) );
    }
  catch(...)
    {
      INT << dataCollect[Dep::PROVIDES] << endl;
    }
  // ...parse other deps

  // create the object
  detail::ResImplTraits<detail::PackageImpl>::Ptr pkgImpl;
  Package::Ptr pkg( detail::makeResolvableAndImpl( dataCollect, pkgImpl ) );
  DBG << *pkg << endl;
  DBG << pkg->deps() << endl;


  // finalize implementation class construction
  // ... aditional data ...

  DBG << pkg << endl;
  DBG << *pkg << endl;
  DBG << pkg->deps() << endl;


  INT << "===[END]============================================" << endl;
  return 0;
}

