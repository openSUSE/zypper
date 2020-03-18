#include <zypp/zyppng/base/EventDispatcher>
#include <zypp/zyppng/media/network/downloader.h>
#include <zypp/zyppng/media/network/networkrequesterror.h>
#include <zypp/zyppng/media/network/networkrequestdispatcher.h>
#include <zypp/zyppng/media/network/request.h>
#include <zypp/media/CredentialManager.h>
#include <zypp/TmpPath.h>
#include <zypp/PathInfo.h>
#include <zypp/ZConfig.h>
#include <iostream>
#include <fstream>
#include <random>
#include "WebServer.h"
#include "TestTools.h"

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#define BOOST_TEST_REQ_SUCCESS(REQ) \
  do { \
      BOOST_REQUIRE_MESSAGE( REQ->state() == zyppng::Download::Success, zypp::str::Format(" %1% != zyppng::Download::Success (%2%)") % REQ->state() % REQ->errorString() ); \
      BOOST_REQUIRE_EQUAL( REQ->lastRequestError().type(), zyppng::NetworkRequestError::NoError ); \
      BOOST_REQUIRE( REQ->errorString().empty() ); \
  } while(false)

#define BOOST_TEST_REQ_FAILED(REQ) \
  do { \
      BOOST_REQUIRE_EQUAL( REQ->state(), zyppng::Download::Failed ); \
      BOOST_REQUIRE_NE( REQ->lastRequestError().type(), zyppng::NetworkRequestError::NoError ); \
      BOOST_REQUIRE( !REQ->errorString().empty() ); \
  } while(false)

namespace bdata = boost::unit_test::data;

bool withSSL[] = {true, false};

BOOST_DATA_TEST_CASE( dltest_basic, bdata::make( withSSL ), withSSL)
{
  auto ev = zyppng::EventDispatcher::createMain();

  zyppng::Downloader downloader;

  //make sure the data here is big enough to cross the threshold of 256 bytes so we get a progress signal emitted and not only the alive signal.
  std::string dummyContent = "This is just some dummy content,\nto test downloading and signals.\n"
                             "This is just some dummy content,\nto test downloading and signals.\n"
                             "This is just some dummy content,\nto test downloading and signals.\n"
                             "This is just some dummy content,\nto test downloading and signals.\n";

  WebServer web((zypp::Pathname(TESTS_SRC_DIR)/"data"/"dummywebroot").c_str(), 10001, withSSL );
  web.addRequestHandler("getData", WebServer::makeResponse("200", dummyContent ) );
  BOOST_REQUIRE( web.start() );

  zypp::filesystem::TmpFile targetFile;
  zyppng::Url weburl (web.url());
  weburl.setPathName("/handler/getData");
  zyppng::TransferSettings set = web.transferSettings();

  bool gotStarted = false;
  bool gotFinished = false;
  bool gotProgress = false;
  bool gotAlive = false;
  off_t lastProgress = 0;
  off_t totalDL = 0;

  std::vector<zyppng::Download::State> allStates;


  zyppng::Download::Ptr dl = downloader.downloadFile(  weburl, targetFile.path(), dummyContent.length() );
  dl->settings() = set;

  dl->sigFinished().connect([&]( zyppng::Download & ){
    gotFinished = true;
    ev->quit();
  });

  dl->sigStarted().connect([&]( zyppng::Download & ){
    gotStarted = true;
  });

  dl->sigAlive().connect([&]( zyppng::Download &, off_t ){
    gotAlive = true;
  });

  dl->sigStateChanged().connect([&]( zyppng::Download &, zyppng::Download::State state ){
    allStates.push_back( state );
  });

  dl->sigProgress().connect([&]( zyppng::Download &, off_t dltotal, off_t dlnow ){
    gotProgress = true;
    lastProgress = dlnow;
    totalDL = dltotal;
  });
  dl->start();
  ev->run();

  BOOST_TEST_REQ_SUCCESS( dl );
  BOOST_REQUIRE( gotStarted );
  BOOST_REQUIRE( gotFinished );
  BOOST_REQUIRE( gotProgress );
  BOOST_REQUIRE( gotAlive );
  BOOST_REQUIRE_EQUAL( totalDL, dummyContent.length() );
  BOOST_REQUIRE_EQUAL( lastProgress, dummyContent.length() );
  BOOST_REQUIRE ( allStates == std::vector<zyppng::Download::State>({zyppng::Download::Initializing,zyppng::Download::Running, zyppng::Download::Success}) );
}


struct MirrorSet
{
  std::string name; //<dataset name, used only in debug output if the test fails
  std::string handlerPath; //< the webhandler path used to query the resource
  std::vector< std::pair<int, std::string> > mirrors; //all mirrors injected into the metalink file
  int expectedFileDownloads; //< how many downloads are direct file downloads
  int expectedHandlerDownloads; //< how many started downloads are handler requests
  std::vector<zyppng::Download::State> expectedStates;
  bool expectSuccess; //< should the download work out?
  zypp::ByteCount chunkSize = zypp::ByteCount( 256, zypp::ByteCount::K );

  std::ostream & operator<<( std::ostream & str ) const {
    str << "MirrorSet{ " << name << " }";
    return str;
  }
};

namespace boost{ namespace test_tools{ namespace tt_detail{
template<>
struct print_log_value< MirrorSet > {
    void    operator()( std::ostream& str, MirrorSet const& set) {
      set.operator<<( str );
    }
};
}}}

std::vector< MirrorSet > generateMirr ()
{
  std::vector< MirrorSet > res;

  //all mirrors good:
  res.push_back( MirrorSet() );
  res.back().name = "All good mirrors";
  res.back().expectSuccess = true;
  res.back().expectedHandlerDownloads  = 1;
  res.back().expectedFileDownloads  = 9;
  res.back().expectedStates = {zyppng::Download::Initializing, zyppng::Download::RunningMulti, zyppng::Download::Success};
  for ( int i = 100 ; i >= 10; i -= 10 )
    res.back().mirrors.push_back( std::make_pair( i, "/test.txt") );

  //all mirrors good big chunk
  res.push_back( MirrorSet() );
  res.back().name = "All good mirrors, 1024 chunk size";
  res.back().expectSuccess = true;
  res.back().expectedHandlerDownloads  = 1;
  res.back().expectedFileDownloads  = 3;
  res.back().chunkSize = zypp::ByteCount( 1024, zypp::ByteCount::K );
  res.back().expectedStates = {zyppng::Download::Initializing, zyppng::Download::RunningMulti, zyppng::Download::Success};
  for ( int i = 100 ; i >= 10; i -= 10 )
    res.back().mirrors.push_back( std::make_pair( i, "/test.txt") );

  //no mirrors:
  res.push_back( MirrorSet() );
  res.back().name = "Empty mirrors";
  res.back().expectSuccess = true;
  res.back().expectedHandlerDownloads  = 10;
  res.back().expectedFileDownloads  = 0;
  res.back().expectedStates = {zyppng::Download::Initializing, zyppng::Download::RunningMulti, zyppng::Download::Success};

  //only broken mirrors:
  res.push_back( MirrorSet() );
  res.back().name = "All broken mirrors";
  res.back().expectSuccess = true;
  res.back().expectedHandlerDownloads  = 2; //has to fall back to url handler download
  res.back().expectedFileDownloads  = 10; //should try all mirrors and fail
  res.back().expectedStates = {zyppng::Download::Initializing, zyppng::Download::RunningMulti, zyppng::Download::Running, zyppng::Download::Success};
  for ( int i = 100 ; i >= 10; i -= 10 )
    res.back().mirrors.push_back( std::make_pair( i, "/doesnotexist.txt") );

  //some broken mirrors:
  res.push_back( MirrorSet() );
  res.back().name = "Some broken mirrors less URLs than blocks";
  res.back().expectSuccess = true;
  res.back().expectedHandlerDownloads = 1;
  res.back().expectedFileDownloads = 9 + 3; // 3 should fail due to broken mirrors
  res.back().expectedStates = {zyppng::Download::Initializing, zyppng::Download::RunningMulti, zyppng::Download::Success};
  for ( int i = 10 ; i >= 5; i-- ) {
    if ( i % 2 ) {
      res.back().mirrors.push_back( std::make_pair( i*10, "/doesnotexist.txt") );
    } else {
      res.back().mirrors.push_back( std::make_pair( i*10, "/test.txt") );
    }
  }

  //some broken mirrors with more URLs than blocks:
  res.push_back( MirrorSet() );
  res.back().name = "Some broken mirrors more URLs than blocks";
  res.back().expectSuccess = true;
  res.back().expectedHandlerDownloads = 1;
  //its not really possible to know how many times the downloads will fail, there are
  //5 broken mirrors in the set, but if a working mirror is done before the last broken
  //URL is picked from the dataset not all broken URLs will be used
  res.back().expectedFileDownloads  = -1;
  res.back().expectedStates = {zyppng::Download::Initializing, zyppng::Download::RunningMulti, zyppng::Download::Success};
  for ( int i = 10 ; i >= 1; i-- ) {
    if ( i % 2 ) {
      res.back().mirrors.push_back( std::make_pair( i*10, "/doesnotexist.txt") );
    } else {
      res.back().mirrors.push_back( std::make_pair( i*10, "/test.txt") );
    }
  }

  //mirrors where some return a invalid block
  res.push_back( MirrorSet() );
  res.back().name = "Some mirrors return broken blocks";
  res.back().expectSuccess = true;
  res.back().expectedHandlerDownloads  = 1;
  //its not really possible to know how many times the downloads will fail, there are
  //5 broken mirrors in the set, but if a working mirror is done before the last broken
  //URL is picked from the dataset not all broken URLs will be used
  res.back().expectedFileDownloads  = -1;
  res.back().expectedStates = {zyppng::Download::Initializing, zyppng::Download::RunningMulti, zyppng::Download::Success};
  for ( int i = 10 ; i >= 1; i-- ) {
    if ( i % 2 ) {
      res.back().mirrors.push_back( std::make_pair( i*10, "/handler/random") );
    } else {
      res.back().mirrors.push_back( std::make_pair( i*10, "/test.txt") );
    }
  }
  return res;
}


//create one URL line for a metalink template file
std::string makeUrl ( int pref, const zyppng::Url &url )
{
  return ( zypp::str::Format( "<url preference=\"%1%\" location=\"de\" type=\"%2%\">%3%</url>" ) % pref % url.getScheme() % url );
};


static bool requestWantsMetaLink ( const WebServer::Request &req )
{
  auto it = req.params.find( "HTTP_ACCEPT" );
  if ( it != req.params.end() ) {
    if ( (*it).second.find("application/metalink+xml")  != std::string::npos ||
         (*it).second.find("application/metalink4+xml") != std::string::npos ) {
      return true;
    }
  }
  return false;
}

//creates a request handler for the Mock WebServer that returns the metalink data
//specified in \a data if the request has the metalink accept handler
WebServer::RequestHandler makeMetaFileHandler ( const std::string *data )
{
  return [ data ]( WebServer::Request &req ){
    if ( requestWantsMetaLink( req ) ) {
      req.rout << WebServer::makeResponseString( "200", { "Content-Type: application/metalink+xml; charset=utf-8\r\n" }, *data );
      return;
    }
    req.rout << "Location: /test.txt\r\n\r\n";
    return;
  };
};

//creates a request handler for the Mock WebServer that returns a junk block of
//data for a range request, otherwise relocates the request
WebServer::RequestHandler makeJunkBlockHandler ( )
{
  return [ ]( WebServer::Request &req ){
    auto it = req.params.find( "HTTP_RANGE" );
    if ( it != req.params.end() && zypp::str::startsWith( it->second, "bytes=" ) ) {
        //bytes=786432-1048575
        std::string range = it->second.substr( 6 ); //remove bytes=
        size_t dash = range.find_first_of( "-" );
        off_t start = -1;
        off_t end = -1;
        if ( dash != std::string::npos ) {
          start = std::stoll( range.substr( 0, dash ) );
          end   = std::stoll( range.substr( dash+1 ) );
        }

        if ( start != -1 && end != -1 ) {
          std::string block;
          for ( off_t curr = start; curr <= end; curr++ ) {
            block += 'a';
          }
          req.rout << "Status: 206 Partial Content\r\n"
                   << "Accept-Ranges: bytes\r\n"
                   << "Content-Length: "<< block.length() <<"\r\n"
                   << "Content-Range: bytes "<<start<<"-"<<end<<"/"<<block.length()<<"\r\n"
                   <<"\r\n"
                   << block;
          return;
        }
    }
    req.rout << "Location: /test.txt\r\n\r\n";
    return;
  };
};

int maxConcurrentDLs[] = { 1, 2, 4, 8, 10, 15 };

BOOST_DATA_TEST_CASE( test1, bdata::make( generateMirr() ) * bdata::make( withSSL ) * bdata::make( maxConcurrentDLs )  , elem, withSSL, maxDLs )
{
  //each URL in the metalink file has a preference , a schema and of course the URL, we need to adapt those to our test setup
  //so we generate the file on the fly from a template in the test data
  std::string metaTempl = TestTools::readFile ( zypp::Pathname(TESTS_SRC_DIR)/"/zyppng/data/downloader/test.txt.meta" );
  BOOST_REQUIRE( !metaTempl.empty() );

  auto ev = zyppng::EventDispatcher::createMain();

  WebServer web((zypp::Pathname(TESTS_SRC_DIR)/"/zyppng/data/downloader").c_str(), 10001, withSSL );
  BOOST_REQUIRE( web.start() );

  zypp::filesystem::TmpFile targetFile;
  zyppng::Downloader downloader;
  downloader.requestDispatcher()->setMaximumConcurrentConnections( maxDLs );

  //first metalink download, generate a fully valid one
  zyppng::Url weburl (web.url());
  weburl.setPathName("/handler/test.txt");

  std::string urls;
  if ( elem.mirrors.size() ) {
    for ( const auto &mirr : elem.mirrors ) {
      zyppng::Url mirrUrl (web.url());
      mirrUrl.setPathName( mirr.second );
      urls += makeUrl( mirr.first, mirrUrl ) + "\n";
    }
  }

  std::string metaFile = zypp::str::Format( metaTempl ) % urls;
  web.addRequestHandler("test.txt", makeMetaFileHandler( &metaFile ) );
  web.addRequestHandler("random", makeJunkBlockHandler( ) );

  int expectedDownloads = elem.expectedHandlerDownloads + elem.expectedFileDownloads;
  int startedDownloads = 0;
  int finishedDownloads = 0;
  bool downloadHadError = false;
  bool gotProgress = false;
  bool gotAlive = false;
  bool gotMultiDLState = false;
  std::vector<zyppng::Download::State> allStates;
  off_t gotTotal = 0;
  off_t lastProgress = 0;

  int countHandlerReq = 0; //the requests made to the handler slot
  int countFileReq = 0;    //the requests made to the file directly, e.g. a mirror read from the metalink file

  auto dl = downloader.downloadFile( weburl, targetFile );
  dl->settings() = web.transferSettings();
  dl->setPreferredChunkSize( elem.chunkSize );

  dl->dispatcher().sigDownloadStarted().connect( [&]( zyppng::NetworkRequestDispatcher &, zyppng::NetworkRequest &req){
    startedDownloads++;
    if ( req.url() == weburl )
      countHandlerReq++;
    else
      countFileReq++;
  });

  dl->dispatcher().sigDownloadFinished().connect( [&]( zyppng::NetworkRequestDispatcher &, zyppng::NetworkRequest &req ){
    finishedDownloads++;

    if ( !downloadHadError )
      downloadHadError = req.hasError();
  });

  dl->sigFinished().connect([&]( zyppng::Download & ){
    ev->quit();
  });

  dl->sigAlive().connect([&]( zyppng::Download &, off_t dlnow ){
    gotAlive = true;
    lastProgress = dlnow;
  });

  dl->sigProgress().connect([&]( zyppng::Download &, off_t dltotal, off_t dlnow ){
    gotProgress = true;
    gotTotal = dltotal;
    lastProgress = dlnow;
  });

  dl->sigStateChanged().connect([&]( zyppng::Download &, zyppng::Download::State state ){
    if ( state == zyppng::Download::RunningMulti )
      gotMultiDLState = true;
  });

  dl->sigStateChanged().connect([&]( zyppng::Download &, zyppng::Download::State state ){
    allStates.push_back( state );
  });

  dl->start();
  ev->run();

  //std::cout << dl->errorString() << std::endl;

  if ( elem.expectSuccess )
    BOOST_TEST_REQ_SUCCESS( dl );
  else
    BOOST_TEST_REQ_FAILED ( dl );

  if ( elem.expectedHandlerDownloads > -1 && elem.expectedFileDownloads > -1 )
    BOOST_REQUIRE_EQUAL( startedDownloads, expectedDownloads );

  BOOST_REQUIRE_EQUAL( startedDownloads, finishedDownloads );
  BOOST_REQUIRE( gotAlive );
  BOOST_REQUIRE( gotProgress );
  BOOST_REQUIRE( gotMultiDLState );
  BOOST_REQUIRE_EQUAL( lastProgress, 2148018 );
  BOOST_REQUIRE_EQUAL( lastProgress, gotTotal );

  if ( elem.expectedHandlerDownloads > -1 )
    BOOST_REQUIRE_EQUAL( countHandlerReq, elem.expectedHandlerDownloads );

  if ( elem.expectedFileDownloads > -1 )
    BOOST_REQUIRE_EQUAL( countFileReq, elem.expectedFileDownloads );

  if ( !elem.expectedStates.empty() )
    BOOST_REQUIRE( elem.expectedStates == allStates );
}

//tests:
// - broken cert
// - correct expected filesize
// - invalid filesize
// - password handling and propagation


//creates a request handler that requires a authentication to work
WebServer::RequestHandler createAuthHandler ( )
{
  return [ ]( WebServer::Request &req ){
    //Basic dGVzdDp0ZXN0
    auto it = req.params.find( "HTTP_AUTHORIZATION" );
    bool authorized = false;
    if ( it != req.params.end() )
      authorized = ( it->second == "Basic dGVzdDp0ZXN0" );

    if ( !authorized ) {
      req.rout << "Status: 401 Unauthorized\r\n"
                  "Content-Type: text/html; charset=utf-8\r\n"
                  "WWW-Authenticate: Basic realm=\"User Visible Realm\", charset=\"UTF-8\" \r\n"
                  "\r\n"
                  "Sorry you are not authorized.";
      return;
    }

    if ( requestWantsMetaLink( req ) ) {
      it = req.params.find( "HTTPS" );
      if ( it != req.params.end() && it->second == "on" ) {
        req.rout << "Status: 307 Temporary Redirect\r\n"
                 << "Cache-Control: no-cache\r\n"
                 << "Location: /auth-https.meta\r\n\r\n";
      } else {
        req.rout << "Status: 307 Temporary Redirect\r\n"
                 << "Cache-Control: no-cache\r\n"
                 << "Location: /auth-http.meta\r\n\r\n";
      }
      return;
    }
    req.rout << "Status: 307 Temporary Redirect\r\n"
                "Location: /test.txt\r\n\r\n";
    return;
  };
};

BOOST_DATA_TEST_CASE( dltest_auth, bdata::make( withSSL ), withSSL )
{
  //don't write or read creds from real settings dir
  zypp::filesystem::TmpDir repoManagerRoot;
  zypp::ZConfig::instance().setRepoManagerRoot( repoManagerRoot.path() );

  auto ev = zyppng::EventDispatcher::createMain();

  zyppng::Downloader downloader;

  WebServer web((zypp::Pathname(TESTS_SRC_DIR)/"/zyppng/data/downloader").c_str(), 10001, withSSL );
  BOOST_REQUIRE( web.start() );

  zypp::filesystem::TmpFile targetFile;
  zyppng::Url weburl (web.url());
  weburl.setPathName("/handler/test.txt");
  zyppng::TransferSettings set = web.transferSettings();

  web.addRequestHandler( "test.txt", createAuthHandler() );
  web.addRequestHandler( "quit", [ &ev ]( WebServer::Request & ){ ev->quit();} );

  {
    auto dl = downloader.downloadFile( weburl, targetFile.path() );
    dl->settings() = set;

    dl->sigFinished( ).connect([ &ev ]( zyppng::Download & ){
      ev->quit();
    });

    dl->start();
    ev->run();
    BOOST_TEST_REQ_FAILED( dl );
    BOOST_REQUIRE_EQUAL( dl->lastRequestError().type(), zyppng::NetworkRequestError::Unauthorized );
  }

  {
    auto dl = downloader.downloadFile( weburl, targetFile.path() );
    dl->settings() = set;

    int gotAuthRequest = 0;

    dl->sigFinished( ).connect([ &ev ]( zyppng::Download & ){
      ev->quit();
    });

    dl->sigAuthRequired().connect( [&]( zyppng::Download &, zyppng::NetworkAuthData &auth, const std::string &availAuth ){
      gotAuthRequest++;
      if ( gotAuthRequest >= 2 )
        return;
      auth.setUsername("wrong");
      auth.setPassword("credentials");
    });

    dl->start();
    ev->run();
    BOOST_TEST_REQ_FAILED( dl );
    BOOST_REQUIRE_EQUAL( gotAuthRequest, 2 );
    BOOST_REQUIRE_EQUAL( dl->lastRequestError().type(), zyppng::NetworkRequestError::AuthFailed );
  }

  {
    int gotAuthRequest = 0;
    std::vector<zyppng::Download::State> allStates;
    auto dl = downloader.downloadFile( weburl, targetFile.path() );
    dl->settings() = set;

    dl->sigFinished( ).connect([ &ev ]( zyppng::Download & ){
      ev->quit();
    });

    dl->sigAuthRequired().connect( [&]( zyppng::Download &, zyppng::NetworkAuthData &auth, const std::string &availAuth ){
      gotAuthRequest++;
      auth.setUsername("test");
      auth.setPassword("test");
    });

    dl->sigStateChanged().connect([&]( zyppng::Download &, zyppng::Download::State state ){
      allStates.push_back( state );
    });


    dl->start();
    ev->run();
    BOOST_TEST_REQ_SUCCESS( dl );
    BOOST_REQUIRE_EQUAL( gotAuthRequest, 1 );
    BOOST_REQUIRE ( allStates == std::vector<zyppng::Download::State>({zyppng::Download::Initializing, zyppng::Download::RunningMulti, zyppng::Download::Success}) );
  }

  {
    //the creds should be in the credential manager now, we should not need to specify them again in the slot
    bool gotAuthRequest = false;
    auto dl = downloader.downloadFile( weburl, targetFile.path() );
    dl->settings() = set;

    dl->sigFinished( ).connect([ &ev ]( zyppng::Download & ){
      ev->quit();
    });

    dl->sigAuthRequired().connect( [&]( zyppng::Download &, zyppng::NetworkAuthData &auth, const std::string &availAuth ){
      gotAuthRequest = true;
    });

    dl->start();
    ev->run();
    BOOST_TEST_REQ_SUCCESS( dl );
    BOOST_REQUIRE( !gotAuthRequest );
  }
}

/**
 * Test for bsc#1174011 auth=basic ignored in some cases
 *
 * If the URL specifes ?auth=basic libzypp should proactively send credentials we have available in the cred store
 */
BOOST_DATA_TEST_CASE( dltest_auth_basic, bdata::make( withSSL ), withSSL )
{
  //don't write or read creds from real settings dir
  zypp::filesystem::TmpDir repoManagerRoot;
  zypp::ZConfig::instance().setRepoManagerRoot( repoManagerRoot.path() );

  auto ev = zyppng::EventDispatcher::createMain();

  zyppng::Downloader downloader;

  WebServer web((zypp::Pathname(TESTS_SRC_DIR)/"/zyppng/data/downloader").c_str(), 10001, withSSL );
  BOOST_REQUIRE( web.start() );

  zypp::filesystem::TmpFile targetFile;
  zyppng::Url weburl (web.url());
  weburl.setPathName("/handler/test.txt");
  weburl.setQueryParam("auth", "basic");
  weburl.setUsername("test");

  // make sure the creds are already available
  zypp::media::CredentialManager cm( repoManagerRoot.path() );
  zypp::media::AuthData data ("test", "test");
  data.setUrl( weburl );
  cm.addCred( data );
  cm.save();

  zyppng::TransferSettings set = web.transferSettings();

  web.addRequestHandler( "test.txt", createAuthHandler() );
  web.addRequestHandler( "quit", [ &ev ]( WebServer::Request & ){ ev->quit();} );

  {
    // simply check by request count if the test was successfull:
    // if the proactive code adding the credentials to the first request is not executed we will
    // have more than 1 request.
    int reqCount = 0;
    auto dispatcher = downloader.requestDispatcher();
    dispatcher->sigDownloadStarted().connect([&]( zyppng::NetworkRequestDispatcher &, zyppng::NetworkRequest & ){
      reqCount++;
    });


    auto dl = downloader.downloadFile( weburl, targetFile.path() );
    dl->setMultiPartHandlingEnabled( false );

    dl->settings() = set;

    dl->sigFinished( ).connect([ &ev ]( zyppng::Download & ){
      ev->quit();
    });

    dl->start();
    ev->run();
    BOOST_TEST_REQ_SUCCESS( dl );
    BOOST_REQUIRE_EQUAL( reqCount, 1 );
  }
}
