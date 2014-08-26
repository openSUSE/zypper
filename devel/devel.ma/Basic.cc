#include "Tools.h"

#include <iostream>

#include <zypp/base/LogControl.h>
#include <zypp/base/LogTools.h>
#include <zypp/base/String.h>
#include <zypp/base/SerialNumber.h>
#include <zypp/ExternalProgram.h>
#include <zypp/PathInfo.h>
#include <zypp/TmpPath.h>
#include <zypp/ResPoolProxy.h>
#include <zypp/repo/PackageProvider.h>
#include "zypp/media/MediaManager.h"
#include "zypp/MediaSetAccess.h"
#include "zypp/Fetcher.h"

static const Pathname sysRoot( "/" );

using namespace std;
using namespace zypp;
using namespace zypp::ui;


#include <zypp/ZYppCallbacks.h>

    struct DownloadProgressReceive :  public callback::ReceiveReport<media::DownloadProgressReport>
    {
      DownloadProgressReceive()
      { connect(); }
#if 0
        enum Action {
          ABORT,  // abort and return error
          RETRY,	// retry
          IGNORE	// ignore the failure
        };

        enum Error {
          NO_ERROR,
          NOT_FOUND, 	// the requested Url was not found
          IO,		// IO error
          ACCESS_DENIED, // user authent. failed while accessing restricted file
          ERROR // other error
        };
#endif
        virtual void start( const Url & file, Pathname localfile )
	{
	  USR << "DP +++ " << file  << endl;
	  lp = 0;
	}

        virtual bool progress(int value, const Url &file, double dbps_avg = -1, double dbps_current = -1)
        {
	  if ( abs(value-lp) >= 20 || value == 100 && lp != 100  )
	  {
	    USR << "DP " << file << " " << value << "%" << endl;
	    lp = value;
	  }
	  return true;

	}

        virtual Action problem( const Url &file , Error error , const std::string &description )
	{
	  USR << "DP !!! " << file << " (" << error << ")" << endl;
	  return ABORT;

	}

        virtual void finish( const Url &file , Error error , const std::string &reason )
	{
	  USR << "DP --- " << file << " (" << error << ")" << endl;
	}

	int lp;
    };

    ////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////

    struct DownloadResolvableReceive :  public callback::ReceiveReport<repo::DownloadResolvableReport>
    {
      DownloadResolvableReceive()
      { connect(); }
#if 0
      enum Action {
        ABORT,  // abort and return error
        RETRY,	// retry
        IGNORE, // ignore this resolvable but continue
      };

      enum Error {
        NO_ERROR,
        NOT_FOUND, 	// the requested Url was not found
        IO,		// IO error
        INVALID		// the downloaded file is invalid
      };
#endif
      virtual void start( Resolvable::constPtr resolvable_ptr, const Url &url )
      {
	USR << "+++ " << resolvable_ptr << endl;
      }


      // Dowmload delta rpm:
      // - path below url reported on start()
      // - expected download size (0 if unknown)
      // - download is interruptable
      // - problems are just informal
      virtual void startDeltaDownload( const Pathname & /*filename*/, const ByteCount & /*downloadsize*/ )
      {
	USR << __PRETTY_FUNCTION__ << endl;
      }

      virtual bool progressDeltaDownload( int /*value*/ )
      {
	USR << __PRETTY_FUNCTION__ << endl;
	return true;
      }

      virtual void problemDeltaDownload( const std::string &/*description*/ )
      {
	USR << __PRETTY_FUNCTION__ << endl;
      }

      virtual void finishDeltaDownload()
      {
	USR << __PRETTY_FUNCTION__ << endl;
      }

      // Apply delta rpm:
      // - local path of downloaded delta
      // - aplpy is not interruptable
      // - problems are just informal
      virtual void startDeltaApply( const Pathname & /*filename*/ )
      {
	USR << __PRETTY_FUNCTION__ << endl;
      }

      virtual void progressDeltaApply( int /*value*/ )
      {
	USR << __PRETTY_FUNCTION__ << endl;
      }

      virtual void problemDeltaApply( const std::string &/*description*/ )
      {
	USR << __PRETTY_FUNCTION__ << endl;
      }

      virtual void finishDeltaApply()
      {
	USR << __PRETTY_FUNCTION__ << endl;
      }

      // return false if the download should be aborted right now
      virtual bool progress(int value, Resolvable::constPtr resolvable_ptr)
      {
	if ( 1 || abs(value-lp) >= 20 || value == 100 && lp != 100  )
	{
	  USR << resolvable_ptr << " " << value << "%" << endl;
	  lp = value;
	}
	return true;
      }

      virtual Action problem( Resolvable::constPtr resolvable_ptr , Error error , const std::string &/*description*/ )
      {
	USR << "!!! " << resolvable_ptr << " (" << error << ")" << endl;
	return ABORT;
      }

      virtual void finish(Resolvable::constPtr resolvable_ptr , Error error , const std::string &/*reason*/ )
      {
	USR << "--- " << resolvable_ptr << " (" << error << ")" << endl;
      }

      int lp;
    };


bool queryInstalledEditionHelper( const std::string & name_r,
                                  const Edition &     ed_r,
                                  const Arch &        arch_r )
{
  if ( ed_r == Edition::noedition )
    return true;
  if ( name_r == "kernel-default" && ed_r == Edition("2.6.22.5-10") )
    return true;
  if ( name_r == "update-test-affects-package-manager" && ed_r == Edition("1.1-6") )
    return true;

  return false;
}

ManagedFile repoProvidePackage( const PoolItem & pi )
{
  ResPool _pool( getZYpp()->pool() );
  repo::RepoMediaAccess _access;

  // Redirect PackageProvider queries for installed editions
  // (in case of patch/delta rpm processing) to rpmDb.
  repo::PackageProviderPolicy packageProviderPolicy;
  packageProviderPolicy.queryInstalledCB( queryInstalledEditionHelper );

  Package::constPtr p = asKind<Package>( pi.resolvable() );

  // Build a repository list for repos
  // contributing to the pool
  repo::DeltaCandidates deltas;//( repo::makeDeltaCandidates( _pool.knownRepositoriesBegin(), _pool.knownRepositoriesEnd() ) );

  repo::PackageProvider pkgProvider( _access, p, deltas, packageProviderPolicy );

  return pkgProvider.providePackage();
}

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  --argc;
  ++argv;
  zypp::base::LogControl::instance().logToStdErr();
  INT << "===[START]==========================================" << endl;
  ::unsetenv( "ZYPP_CONF" );
  ZConfig::instance();

  DownloadProgressReceive _dpr;
  DownloadResolvableReceive _drr;

  TestSetup::LoadSystemAt( sysRoot );
  ///////////////////////////////////////////////////////////////////
  ResPool   pool( ResPool::instance() );
  sat::Pool satpool( sat::Pool::instance() );
  ///////////////////////////////////////////////////////////////////
  dumpRange( USR, satpool.reposBegin(), satpool.reposEnd() ) << endl;
  USR << "pool: " << pool << endl;

  if ( 0 ) {
    PoolItem pi( getPi<Package>( "CDT", "amarok", Edition(), Arch_empty ) );
    SEC << pi << endl;
    ManagedFile f( repoProvidePackage( pi ) );
    SEC << f << endl;
  }
  {
    Url url("cd:///?devices=/dev/sr0");
    Pathname path(url.getPathName());
    url.setPathName ("/");
    MediaSetAccess access(url);
    Pathname local = access.provideFile(path);
    SEC << local << endl;
  }
  if ( 0 ) {
    Url url("http://download.opensuse.org/debug/distribution/11.4/repo/oss/content.asc");
    url.setPathName ("/");
    MediaSetAccess access(url);

    zypp::Fetcher fch;
    fch.reset();
    fch.setOptions(zypp::Fetcher::AutoAddIndexes);

    // path - add "/" to the beginning if it's missing there
    std::string media_path("/debug/distribution/11.4/repo/oss/content.ascx");
    zypp::OnMediaLocation mloc(media_path, 1);
    mloc.setOptional(true);

    zypp::filesystem::TmpDir tmpdir( zypp::filesystem::TmpDir::defaultLocation() );
    fch.addIndex(mloc);
    fch.start(tmpdir.path(), access);
  }



//   f.resetDispose();
//   ExternalProgram("find /tmp/var") >> DBG;
//   DBG << endl;

  INT << "===[END]============================================" << endl;
  zypp::base::LogControl::instance().logNothing();
  return 0;
}
