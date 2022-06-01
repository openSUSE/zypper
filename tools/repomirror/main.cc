#define INCLUDE_TESTSETUP_WITHOUT_BOOST 1
#include "../../tests/lib/TestSetup.h"

#include <zypp-media/ng/Provide>
#include <zypp-media/ng/ProvideSpec>
#include <zypp-media/FileCheckException>
#include <zypp-core/zyppng/base/EventLoop>
#include <zypp-core/zyppng/base/Timer>
#include <zypp-core/zyppng/pipelines/Transform>
#include <zypp-core/zyppng/pipelines/Lift>
#include <zypp-core/zyppng/pipelines/Wait>
#include <boost/container/vector.hpp>
#include <solv/repo.h>
#include <solv/solvable.h>

#include <cstdlib>
#include <clocale>
#include <iostream>
#include <numeric>

#include "output.h"

class DlSkippedException : public zypp::Exception
{
  public:
    DlSkippedException() : zypp::Exception("Download was skipped" ) {}
};

class MyDLStatus : public zyppng::ProvideStatus
{
  public:
    MyDLStatus ( OutputView &out, zyppng::ProvideRef parent ) : ProvideStatus( parent ), _out(out) {}

    virtual void pulse ( ) {
      zyppng::ProvideStatus::pulse();

      const auto &currStats = stats();

      if ( currStats._expectedBytes == 0 )
        return;

      auto perc = (double)(currStats._finishedBytes + currStats._partialBytes) / (double)currStats._expectedBytes;
      std::string txt = zypp::str::Str() << "Downloading " << zypp::ByteCount((currStats._finishedBytes + currStats._partialBytes)) << "/" << currStats._expectedBytes << " (" << currStats._perSecond<<"/s)";
      _out.updateProgress( txt, perc );
    }

  private:
    OutputView &_out;
};

/*!
 * Downloads a solvable via the given handle.
 * Returns the AsyncResult
 */
zyppng::AsyncOpRef<zyppng::expected<zyppng::ProvideRes>> provideSolvable ( std::shared_ptr<OutputView> output, zyppng::ProvideMediaHandle handle, const zypp::sat::Solvable &s )
{
  using namespace zyppng::operators;

  auto prov = handle.parent();
  if ( !prov  )
    return zyppng::makeReadyResult( zyppng::expected<zyppng::ProvideRes>::error( std::make_exception_ptr( zypp::Exception("Handle without parent!"))) );

  zypp::PoolItem pi(s);
  if ( !pi->isKind<zypp::Package>() ) {
    //output->putMsgErr( zypp::str::Str() <<  "Skipping 1:  " << pi.asUserString() << "\n" );
    return zyppng::makeReadyResult( zyppng::expected<zyppng::ProvideRes>::error( std::make_exception_ptr(DlSkippedException())) );
  }

  auto oml = pi.lookupLocation();
  if ( oml.filename().empty() ) {
    output->putMsgErr( zypp::str::Str() <<  "Skipping:  " << pi.asUserString() << "\n" );
    return zyppng::makeReadyResult( zyppng::expected<zyppng::ProvideRes>::error( std::make_exception_ptr(DlSkippedException())) );
  }

  // enable for more output, but sloooooows the startup time down significantly
  //output->putMsgTxt( zypp::str::Str() << "Downloading " << pi.asUserString() << " size is: " << pi.downloadSize() << "\n" );

  return prov->provide( handle, oml.filename(), zyppng::ProvideFileSpec( oml ) )
  | [output]( zyppng::expected<zyppng::ProvideRes> &&res ) {
      if ( res ) {
        output->putMsgTxt( zypp::str::Str() << "File downloaded: " << res->file() << "\n" );
      } else {
        output->putMsgErr( zypp::str::Str() <<   "Failed to download file\n" );
        try
        {
          std::rethrow_exception(res.error());
        }
        catch(const zypp::Exception& e)
        {
          output->putMsgErr( zypp::str::Str() << e.asUserHistory() << '\n' );
        }
        catch(const std::exception& e)
        {
          output->putMsgErr( zypp::str::Str() << e.what() << '\n' );
        }
      }
      return res;
    };
}

/*!
 * Helper function to copy a ProvideRes underlying file to a target destination, keeping the ProvideRes alive until the copy operation has finished
 */
zyppng::AsyncOpRef<zyppng::expected<zypp::ManagedFile>> copyProvideResToFile ( std::shared_ptr<OutputView> output, zyppng::ProvideRef prov, zyppng::ProvideRes &&res, const zypp::Pathname &targetDir )
{
  using namespace zyppng::operators;

  auto fName = res.file();
  return prov->copyFile( fName, targetDir/fName.basename() )
    | [ resSave = std::move(res) ] ( auto &&result ) {
      // callback lambda to keep the ProvideRes reference around until the op is finished,
      // if the op fails the callback will be cleaned up and so the reference
      return result;
    };
}

/*!
 * Helper function to calculate a checksum on a ProvideRes result file, keeping the ProvideRes alive during the operation and returning it again after the test was successful,
 * here we could also ask the user if a invalid checksum should be accepted and ignore the result if the user wants to
 */
zyppng::AsyncOpRef<zyppng::expected<zyppng::ProvideRes>> checksumIfRequired ( std::shared_ptr<OutputView> output, zyppng::ProvideRef prov, zypp::sat::Solvable solvable, zyppng::ProvideRes &&res )
{
  using namespace zyppng::operators;

  auto fName = res.file();

  const auto oml = solvable.lookupLocation();
  if ( !oml.checksum().empty() ) {
    output->putMsgTxt( zypp::str::Str()<<"  calculating checksum for file: " << fName << "\n");

    return prov->checksumForFile( fName, oml.checksum().type() )
    | mbind([ output, expectedSum = oml.checksum(), saveRes = std::move(res) ]( auto &&checksumRes ) {
      output->putMsgTxt( zypp::str::Str()<<"  Got checksum for file: " << saveRes.file() << " is: "  << checksumRes << "\n" );
      try {
        zypp::CheckSum s( expectedSum.type(), checksumRes );
        if ( expectedSum == s )
          return zyppng::expected<zyppng::ProvideRes>::success(std::move(saveRes));
        else
          return zyppng::expected<zyppng::ProvideRes>::error( ZYPP_EXCPT_PTR( zypp::CheckSumCheckException( zypp::str::Str() << saveRes.file() << " has wrong checksum" ) ) );
      } catch ( const zypp::Exception &e ) {
        ZYPP_CAUGHT(e);
        return zyppng::expected<zyppng::ProvideRes>::error( ZYPP_EXCPT_PTR(e) );
      }

    });
  }

  return zyppng::makeReadyResult( zyppng::expected<zyppng::ProvideRes>::success(std::move(res)) );
}



int main ( int argc, char *argv[] )
{
  using namespace zyppng::operators;

  auto ev = zyppng::EventLoop::create();

  auto output = OutputView::create();
  if ( !output ) {
    std::cerr << "Failed to initialize Output view" << std::endl;
    return 1;
  }

  output->putMsgTxt("Loading System\n");
  output->putMsgErr("Errors go here\n");

#if 0
  {
    auto prov = zyppng::Provide::create();
    prov->setStatusTracker( std::make_shared<MyDLStatus>( *output, prov) );

    zypp::Url test("copy://");
    test.setPathName("/tmp/test/test1");

    auto rootOp = prov->provide( test, zyppng::ProvideFileSpec().setDestFilenameHint("/tmp/test/test2") )
    | [&]( zyppng::expected<zyppng::ProvideRes> &&res ) {
      if ( !res ) {
        output->putMsgErr( zypp::str::Str() <<   "Failed to copy file: ", false );
        try {
          std::rethrow_exception(res.error());

        } catch(const zypp::Exception& e) {
          output->putMsgErr( zypp::str::Str() << e.asUserHistory() << '\n' );

        } catch(const std::exception& e) {
          output->putMsgErr( zypp::str::Str() << e.what() << '\n' );

        } catch(...) {
          output->putMsgErr( zypp::str::Str() << "Unknown exception\n" );
        }
      } else {
        output->putMsgTxt( zypp::str::Str() <<  "Yay copy worked\n" );
      }
      return 0;
    };

    prov->start();
    if ( !rootOp->isReady() )
      ev->run();
    output->putMsgTxt( zypp::str::Str() <<  "Waiting, press Return to exit\n" );
    output->waitForKeys( { NCKEY_ENTER } );
    return 0;
  }
#endif

  TestSetup t;
  try {
    t.LoadSystemAt( "/" );
  }  catch ( const zypp::Exception &e ) {
    output.reset(); //disable curses
    std::cerr << "Failed to load repo info from system." << std::endl;
    std::cerr << "Got exception: " << e << std::endl;
    return 1;
  }

  zypp::RepoManager rManager( zypp::RepoManagerOptions("/") );
  auto pool = t.pool();
  auto satPool = t.satpool();

  output->putMsgTxt("Loading repositories\n");

  std::vector<zypp::Repository> myRepos;
  for ( const auto &r : t.satpool().repos() ) {
      if ( r.isSystemRepo() )
        continue;
      myRepos.push_back(r);
  }

  if ( !myRepos.size() ) {
    output->putMsgTxt( zypp::str::Str() <<  "No repos found, press any key to exit\n" );
    output->waitForKeys();
    return 0;
  }

  std::vector<int> reposToDl = output->promptMultiSelect( "Select the repos you want to download", "", myRepos );
  output->renderNow();

  if( !reposToDl.size() ) {
    output->putMsgTxt( zypp::str::Str() <<  "No repos selected, press any key to exit\n" );
    output->waitForKeys();
    return 0;
  }

  const auto &workerPath = zypp::Pathname ( ZYPP_BUILD_DIR ).dirname() / "tools" / "workers";
  auto prov = zyppng::Provide::create();
  prov->setStatusTracker( std::make_shared<MyDLStatus>( *output, prov) );
  prov->sigAuthRequired().connect( [&](const zypp::Url &reqUrl, const std::string &triedUsername, const std::map<std::string, std::string> &extraValues) -> std::optional<zypp::media::AuthData> {

    auto user = output->promptUser(
      zypp::str::Str() << "Auth request for URL: " << reqUrl << "\n"
                       << "Last tried username was: " << triedUsername << "\n",
                       "Username" );
    if ( !user )
      return {};

    zypp::media::AuthData d;
    d.setUrl( reqUrl );
    d.setUsername( *user );

    auto pass = output->promptUser("", "Password");
    if ( pass )
      d.setPassword( *pass );

    return d;
  });

  std::vector< zyppng::AsyncOpRef< std::vector<zyppng::expected<zypp::ManagedFile>>> > mop;

  // we need a way to remember the data for the full transaction, there is probably a better way for a real application
  std::unordered_map<int, std::unordered_map<int, std::vector<zypp::sat::Solvable>>> repoToMediaToSolvables;

  zypp::Pathname workPath = zypp::Pathname(".").realpath() / "allrpms";
  zypp::filesystem::assert_dir( workPath );

  for ( const auto repoToDl : reposToDl ) {
    const auto &r = myRepos[repoToDl];
    output->putMsgTxt( zypp::str::Str() << "Repo selected to download: " << r.asUserString() << "\n" );

    auto urlSet =  r.info().baseUrls();
    if ( urlSet.empty() ) {
      output->putMsgTxt( zypp::str::Str() <<  "Repo has no mirrors, ignoring!\n" );
      continue;
    }

    std::vector<zypp::Url> urlsWithPath;
    std::transform( urlSet.begin(), urlSet.end(), std::back_inserter(urlsWithPath), [&]( const zypp::Url &bUrl ){
      zypp::Url withPath(bUrl);
      withPath.setPathName( r.info().path() / bUrl.getPathName() );
      return withPath;
    });

    zypp::ByteCount bc;
    output->putMsgTxt( zypp::str::Str() <<  "Adding Solvables for repo: " << r << "\n" );
    std::unordered_map<int, std::vector<zypp::sat::Solvable>> &mediaToSolvables = repoToMediaToSolvables[repoToDl];
    std::for_each( satPool.solvablesBegin(), satPool.solvablesEnd(), [&]( const zypp::sat::Solvable &s ) {
      if ( s.repository() != r )
        return;
      bc += s.downloadSize();
      const auto mediaNr = s.lookupLocation().medianr();
      mediaToSolvables[mediaNr].push_back(s);
    });
    output->putMsgTxt( zypp::str::Str() <<  "Download size for repo: " << r << ":" << bc << "\n" );

    std::transform( mediaToSolvables.begin(), mediaToSolvables.end(), std::back_inserter(mop), [&]( const auto &s ) {

      auto spec = zyppng::ProvideMediaSpec( r.info().name() );
      zypp::Pathname mediafile = r.info().metadataPath();
      if ( !mediafile.empty() ) {
        mediafile += "/media.1/media";
        if ( zypp::PathInfo(mediafile).isExist() ) {
          spec.setMediaFile( mediafile );
          spec.setMedianr( s.first );
        }
      }

      return prov->attachMedia( urlsWithPath, spec )
        | [ &, &solvables = s.second ]( zyppng::expected<zyppng::Provide::MediaHandle> &&hdl ) {

            if ( !hdl ) {
              output->putMsgErr( zypp::str::Str() <<   "Failed to attach medium: ", false );
              try {
                std::rethrow_exception(hdl.error());

              } catch(const zypp::Exception& e) {
                output->putMsgErr( zypp::str::Str() << e.asUserHistory() << '\n' );

              } catch(const std::exception& e) {
                output->putMsgErr( zypp::str::Str() << e.what() << '\n' );

              } catch(...) {
                output->putMsgErr( zypp::str::Str() << "Unknown exception\n" );

              }
              return std::vector<zyppng::AsyncOpRef<zyppng::expected<zypp::ManagedFile>>>{ zyppng::makeReadyResult( zyppng::expected<zypp::ManagedFile>::error( hdl.error() ) ) };
            }

            return zyppng::transform( solvables, [ &, hdl=hdl.get() ]( const zypp::sat::Solvable &s ) {
              return s |       (std::bind( &provideSolvable,      output, hdl,  std::placeholders::_1 ))
                       | mbind (std::bind( &checksumIfRequired,   output, prov, sat::Solvable(s), std::placeholders::_1 ))
                       | mbind (std::bind( &copyProvideResToFile, output, prov, std::placeholders::_1, workPath ) ) ;
            });
          }
        | zyppng::waitFor<zyppng::expected<zypp::ManagedFile>>()
        | [ &r, &output ]( const auto &&res ){
          output->putMsgTxt( zypp::str::Str() << "Finished with rpo: " << r.info().name() << "\n" );
          return res;
        };
    });
  }

  std::vector<zyppng::expected<zypp::ManagedFile>> finalResults;

  auto rootOp = std::move( mop )
  | zyppng::waitFor< std::vector<zyppng::expected<zypp::ManagedFile>> >()
  | [&]( std::vector<std::vector<zyppng::expected<zypp::ManagedFile>>> &&allResults ) {
      // reduce to one vector of results
      for ( auto &innerVec : allResults ) {
        std::accumulate( std::make_move_iterator(innerVec.begin()), std::make_move_iterator(innerVec.end()), &finalResults, []( std::vector<zyppng::expected<zypp::ManagedFile>>* vec, auto &&bla ){
          vec->push_back(std::move(bla));
          return vec;
        });
      }
      ev->quit();
      return 0;
  };

  prov->start();
  if ( !rootOp->isReady() )
    ev->run();

  int succ = 0;
  int skip = 0;
  int err  = 0;
  for ( const auto &res : finalResults ) {
    if ( res )
      succ++;
    else {
      try {
        std::rethrow_exception( res.error() );
      } catch(const DlSkippedException& e) {
        skip++;
      } catch( const zypp::Exception &e ) {
        output->putMsgErr( zypp::str::Str()<<"Request failed with: " << e.asUserString() << "\n");
        err++;
      } catch( const std::exception &e ) {
        output->putMsgErr( zypp::str::Str()<<"Request failed with: " << e.what() << "\n");
        err++;
      } catch( ... ) {
        output->putMsgErr( zypp::str::Str()<<"Request failed with an unknown error.\n" );
        err++;
      }
    }
  }
  #if 0
  for ( const auto &outer : finalResults ) {

  }
  #endif

  output->putMsgTxt( zypp::str::Str() <<  "All done:\nSuccess:" << succ << "\nErrors: "<< err << "\nSkipped:"<<skip<<"\n", false );
  output->putMsgTxt( zypp::str::Str() <<  "Waiting, press Return to exit\n" );
  output->waitForKeys( { NCKEY_ENTER } );
  return 0;
}
