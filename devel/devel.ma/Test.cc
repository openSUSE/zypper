#include <iostream>
#include <fstream>
#include <sstream>

#include "zypp/base/LogControl.h"
#include "zypp/base/LogTools.h"
#include <zypp/base/Logger.h>

#include <zypp/source/PackageProvider.h>

using std::endl;
//using namespace std;
using namespace zypp;

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
namespace zypp
{

  using source::ManagedFile;
  using source::PackageProvider;

  Pathname testProvidePackage( Package::constPtr package )
  {
    if ( package->arch() != Arch_x86_64 )
      return Pathname();

    PackageProvider pkgProvider( package );

    try
      {
        ManagedFile r = pkgProvider.providePackage();
      }
    catch ( const Exception & excpt )
      {
        ERR << "Failed to provide Package " << package << endl;
        ZYPP_RETHROW( excpt );
      }

    INT << "got" << endl;
    return Pathname();
  }
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

#include "Tools.h"
#include "zypp/ZYppFactory.h"
#include "zypp/ZYppCallbacks.h"

namespace zypp
{
  class Dln : public callback::ReceiveReport<media::DownloadProgressReport>
  {
    virtual void reportbegin()
    { _SEC("REPORT") << "+++DownloadProgress" << endl; }
    virtual void reportend()
    { _SEC("REPORT") << "---DownloadProgress" << endl; }
  };

  class Sfp : public callback::ReceiveReport<source::DownloadFileReport>
  {
    virtual void reportbegin()
    { _SEC("REPORT") << "+++source::DownloadFile" << endl; }
    virtual void reportend()
    { _SEC("REPORT") << "---source::DownloadFile" << endl; }
  };


  /////////////////////////////////////////////////////////////////
  template<class _Tp>
    struct Encl
    {
      Encl( const _Tp & v )
      : _v( v )
      {}

      const _Tp & _v;
    };

  template<class _Tp>
    std::ostream & operator<<( std::ostream & str, const Encl<_Tp> & obj )
    { return str << ' ' << obj._v << ' '; }

  template<class _Tp>
    Encl<_Tp> encl( const _Tp & v )
    { return Encl<_Tp>( v ); }




  class SRP : public callback::ReceiveReport<source::DownloadResolvableReport>
  {
    virtual void reportbegin()
    { _SEC("REPORT") << "+++source::DownloadResolvable" << endl; }


    virtual void start( Resolvable::constPtr resolvable_ptr, Url url )
    { _SEC("REPORT") << encl(resolvable_ptr) << encl(url) << endl; }

    virtual void startDeltaDownload( const Pathname & filename, const ByteCount & downloadsize )
    { _SEC("REPORT") << encl(filename) << encl(downloadsize) << endl; }

    virtual bool progressDeltaDownload( int value )
    {  _SEC("REPORT") << encl(value) << endl; return true; }

    virtual void problemDeltaDownload( std::string description )
    { _SEC("REPORT") << encl(description) << endl; }

    // Apply delta rpm:
    // - local path of downloaded delta
    // - aplpy is not interruptable
    // - problems are just informal
    virtual void startDeltaApply( const Pathname & filename )
    { _SEC("REPORT") << encl(filename) << endl; }

    virtual void progressDeltaApply( int value )
    { _SEC("REPORT") << encl(value) << endl; }

    virtual void problemDeltaApply( std::string description )
    { _SEC("REPORT") << encl(description) << endl; }

    // Dowmload patch rpm:
    // - path below url reported on start()
    // - expected download size (0 if unknown)
    // - download is interruptable
    virtual void startPatchDownload( const Pathname & filename, const ByteCount & downloadsize )
    { _SEC("REPORT") << encl(filename) << encl(downloadsize) << endl; }

    virtual bool progressPatchDownload( int value )
    {  _SEC("REPORT") << encl(value) << endl; return true; }

    virtual void problemPatchDownload( std::string description )
    { _SEC("REPORT") << encl(description) << endl; }



    virtual bool progress(int value, Resolvable::constPtr resolvable_ptr)
    {  _SEC("REPORT") << encl(value) << endl; return true; }

    virtual Action problem( Resolvable::constPtr resolvable_ptr
                            , Error error
                            , std::string description
                            )
    {  _SEC("REPORT") <<  encl(error) << encl(description) << endl; return ABORT; }

    virtual void finish(Resolvable::constPtr resolvable_ptr
                         , Error error
                         , std::string reason
                         ) {}

    virtual void reportend()
    { _SEC("REPORT") << "---source::DownloadResolvable" << endl; }
  };
}

void test( const ResObject::constPtr & res )
{
  if ( ! isKind<Package>( res ) )
    return;

  if ( ! res->source() )
    return;

  SEC << "Test " << res << endl;

  try
    {
      MIL << zypp::testProvidePackage( asKind<Package>( res ) ) << endl;
    }
  catch ( Exception & expt )
    {
      ERR << expt << endl;
    }
}

void show( const Pathname & file )
{
  WAR << "show " << PathInfo( file ) << endl;
}

struct ValRelease
{
  ValRelease( int i )
  : _i( i )
  {}
  void operator()( const Pathname & file ) const
  { WAR << "ValRelease " << _i << " " << PathInfo( file ) << endl; }

  int _i;
};

void progressor( unsigned i )
{
  USR << i << "%" << endl;
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

  Dln dnl;
  Sfp sfp;
  SRP srp;
  dnl.connect();
  sfp.connect();
  srp.connect();

  std::for_each( pool.byNameBegin("glibc"), pool.byNameEnd("glibc"), test );

  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::instance().logNothing();
  return 0;
}

