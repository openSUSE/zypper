#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <zypp/zyppng/base/EventDispatcher>
#include <zypp/zyppng/media/network/request.h>
#include <zypp/zyppng/media/network/networkrequestdispatcher.h>
#include <zypp/zyppng/media/network/networkrequesterror.h>
#include <zypp/TmpPath.h>
#include <zypp/base/String.h>
#include <zypp/Digest.h>
#include <zypp/PathInfo.h>

#include <iostream>
#include <thread>
#include <chrono>

#include "WebServer.h"


#define BOOST_TEST_REQ_ERR(REQ, EXPECERR) \
  do { \
  BOOST_REQUIRE( REQ->hasError() ); \
  BOOST_REQUIRE( REQ->error().isError() ); \
  BOOST_REQUIRE_EQUAL( REQ->error().type(), EXPECERR ); \
  } while(false)

#define BOOST_TEST_REQ_SUCCESS(REQ) \
  do { \
  BOOST_REQUIRE( !REQ->hasError() ); \
  BOOST_REQUIRE( !REQ->error().isError() ); \
  BOOST_REQUIRE_EQUAL( REQ->error().type(), zyppng::NetworkRequestError::NoError ); \
  } while(false)

namespace bdata = boost::unit_test::data;

const char * err404 = "Status: 404 Not Found\r\n"
                     "Date: Tue, 21 May 2019 08:30:59 GMT\r\n"
                     "Server: Apache/2.4.23 (Linux/SUSE)\r\n"
                     "X-Prefix: 93.192.0.0/10\r\n"
                     "X-AS: 3320\r\n"
                     "Vary: accept-language,accept-charset"
                     "Accept-Ranges: bytes"
                     "Transfer-Encoding: chunked"
                     "Content-Type: text/html; charset=utf-8"
                     "Content-Language: en\r\n"
                     "\r\n"
                     "Resource is no longer available!";

const char * err401 = "Status: 401 Unauthorized\r\n"
                     "Content-Type: text/html; charset=utf-8\r\n"
                     "WWW-Authenticate: Basic realm=\"User Visible Realm\", charset=\"UTF-8\" \r\n"
                     "\r\n"
                     "Sorry you are not authorized.";

bool withSSL[] = { true, false };

// convert a string in its byte representation
std::vector<unsigned char> convertHexStrToVector( const std::string &str )
{
  std::vector<unsigned char> bytes;
  for ( size_t i = 0; i < str.length(); i+=2 )
  {
#define c2h(c) (((c)>='0' && (c)<='9') ? ((c)-'0')              \
                : ((c)>='a' && (c)<='f') ? ((c)-('a'-10))       \
                : ((c)>='A' && (c)<='F') ? ((c)-('A'-10))       \
                : -1)
    int v = c2h(str[i]);
    if (v < 0)
      return {};
    bytes.push_back(v);
    v = c2h(str[i+1]);
    if (v < 0)
      return {};
    bytes.back() = (bytes.back() << 4) | v;
#undef c2h
  }
  return bytes;
}

BOOST_DATA_TEST_CASE(nwdispatcher_basic, bdata::make( withSSL ), withSSL)
{
  std::string dummyContent = "This is just some dummy content,\nto test downloading and signals.";

  auto ev = zyppng::EventDispatcher::createMain();

  WebServer web((zypp::Pathname(TESTS_SRC_DIR)/"data"/"dummywebroot").c_str(), 10001, withSSL );
  web.addRequestHandler("getData", WebServer::makeResponse("200 OK", dummyContent ) );
  web.start();

  BOOST_REQUIRE( !web.isStopped() );

  zyppng::TransferSettings set = web.transferSettings();
  zyppng::NetworkRequestDispatcher disp;
  disp.run();
  disp.sigQueueFinished().connect( [&ev]( const zyppng::NetworkRequestDispatcher& ){
    ev->quit();
  });

  bool gotStarted = false;
  bool gotFinished = false;
  bool gotProgress = false;
  off_t lastProgress = 0;
  off_t totalDL = 0;

  zypp::filesystem::TmpFile targetFile;
  zyppng::Url weburl (web.url());
  weburl.setPathName("/handler/getData");

  zyppng::NetworkRequest::Ptr reqData = std::make_shared<zyppng::NetworkRequest>( weburl, targetFile.path() );
  reqData->transferSettings() = set;
  reqData->sigStarted().connect( [ &gotStarted ]( zyppng::NetworkRequest& ){
    gotStarted = true;
  });
  reqData->sigFinished().connect( [ &gotFinished ]( zyppng::NetworkRequest&, const zyppng::NetworkRequestError & ){
    gotFinished = true;
  });
  reqData->sigProgress().connect( [ & ]( zyppng::NetworkRequest &, off_t dltotal, off_t dlnow, off_t, off_t ){
    gotProgress= true;
    lastProgress = dlnow;
    totalDL = dltotal;
  });

  disp.enqueue( reqData );
  ev->run();

  BOOST_TEST_REQ_SUCCESS( reqData );
  BOOST_REQUIRE( gotStarted );
  BOOST_REQUIRE( gotFinished );
  BOOST_REQUIRE( gotProgress );
  BOOST_REQUIRE_EQUAL( totalDL, dummyContent.length() );
  BOOST_REQUIRE_EQUAL( lastProgress, dummyContent.length() );
}

BOOST_DATA_TEST_CASE(nwdispatcher_http_errors, bdata::make( withSSL ), withSSL)
{
  auto makeErrorResponder = [] ( std::string err ) -> WebServer::RequestHandler  {
    return WebServer::makeResponse( err, "This is a error." );
  };

  auto ev = zyppng::EventDispatcher::createMain();
  WebServer web((zypp::Pathname(TESTS_SRC_DIR)/"data"/"dummywebroot").c_str(), 10001, withSSL );

  web.addRequestHandler("get404", WebServer::makeResponse( err404 ) );
  web.addRequestHandler("get401", WebServer::makeResponse( err401 ) );
  web.addRequestHandler("get502", makeErrorResponder( "502 Bad Gateway" ) );
  web.addRequestHandler("get503", makeErrorResponder( "503 Service Unavailable" ) );
  web.addRequestHandler("get504", makeErrorResponder( "504 Gateway Timeout" ) );
  web.addRequestHandler("get403", makeErrorResponder( "403 Forbidden" ) );
  web.addRequestHandler("get410", makeErrorResponder( "410 Gone" ) );
  web.addRequestHandler("get418", makeErrorResponder( "418 I'm a teapot" ) );
  web.addRequestHandler("delayMe", []( WebServer::Request &req ) {
  });
  web.start();

  zyppng::TransferSettings set = web.transferSettings();

  zyppng::NetworkRequestDispatcher disp;
  disp.sigQueueFinished().connect( [&ev]( const zyppng::NetworkRequestDispatcher& ){
    ev->quit();
  });

  zyppng::Url weburl (web.url());
  weburl.setPathName("/handler/get404");

  zypp::filesystem::TmpFile targetFile;
  zyppng::NetworkRequest::Ptr req404 = std::make_shared<zyppng::NetworkRequest>( weburl, targetFile.path() );
  req404->transferSettings() = set;
  disp.enqueue( req404 );

  weburl = zyppng::Url( "bad://127.0.0.1" );
  BOOST_REQUIRE( !disp.supportsProtocol(weburl) );
  zyppng::NetworkRequest::Ptr reqInvProto = std::make_shared<zyppng::NetworkRequest>( weburl, targetFile.path() );
  reqInvProto->transferSettings() = set;
  disp.enqueue( reqInvProto );

  weburl = zyppng::Url( web.url() );
  weburl.setPathName("/handler/get401");
  zyppng::NetworkRequest::Ptr reqUnauthorized = std::make_shared<zyppng::NetworkRequest>( weburl, targetFile.path() );
  reqUnauthorized->transferSettings() = set;
  disp.enqueue( reqUnauthorized );

  weburl = zyppng::Url( web.url() );
  weburl.setPathName("/handler/get401");
  zyppng::NetworkRequest::Ptr reqAuthFailed = std::make_shared<zyppng::NetworkRequest>( weburl, targetFile.path() );
  reqAuthFailed->transferSettings() = set;
  reqAuthFailed->transferSettings().setUsername("test");
  reqAuthFailed->transferSettings().setPassword("test");
  disp.enqueue( reqAuthFailed );

  weburl = zyppng::Url( web.url() );
  weburl.setPathName("/handler/get502");
  zyppng::NetworkRequest::Ptr req502 = std::make_shared<zyppng::NetworkRequest>( weburl, targetFile.path() );
  req502->transferSettings() = set;
  disp.enqueue( req502 );

  weburl = zyppng::Url( web.url() );
  weburl.setPathName("/handler/get503");
  zyppng::NetworkRequest::Ptr req503 = std::make_shared<zyppng::NetworkRequest>( weburl, targetFile.path() );
  req503->transferSettings() = set;
  disp.enqueue( req503 );

  weburl = zyppng::Url( web.url() );
  weburl.setPathName("/handler/get504");
  zyppng::NetworkRequest::Ptr req504 = std::make_shared<zyppng::NetworkRequest>( weburl, targetFile.path() );
  req504->transferSettings() = set;
  disp.enqueue( req504 );

  weburl = zyppng::Url( web.url() );
  weburl.setPathName("/handler/get403");
  zyppng::NetworkRequest::Ptr req403 = std::make_shared<zyppng::NetworkRequest>( weburl, targetFile.path() );
  req403->transferSettings() = set;
  disp.enqueue( req403 );

  weburl = zyppng::Url( web.url() );
  weburl.setPathName("/handler/get410");
  zyppng::NetworkRequest::Ptr req410 = std::make_shared<zyppng::NetworkRequest>( weburl, targetFile.path() );
  req410->transferSettings() = set;
  disp.enqueue( req410 );

  weburl = zyppng::Url( web.url() );
  weburl.setPathName("/handler/get418");
  zyppng::NetworkRequest::Ptr req418 = std::make_shared<zyppng::NetworkRequest>( weburl, targetFile.path() );
  req418->transferSettings() = set;
  disp.enqueue( req418 );

  disp.run();
  ev->run();

  BOOST_TEST_REQ_ERR( req404, zyppng::NetworkRequestError::NotFound );
  BOOST_TEST_REQ_ERR( reqInvProto, zyppng::NetworkRequestError::UnsupportedProtocol );
  BOOST_TEST_REQ_ERR( reqUnauthorized, zyppng::NetworkRequestError::Unauthorized );
  BOOST_TEST_REQ_ERR( reqAuthFailed, zyppng::NetworkRequestError::AuthFailed );
  BOOST_TEST_REQ_ERR( req502, zyppng::NetworkRequestError::TemporaryProblem );
  BOOST_TEST_REQ_ERR( req503, zyppng::NetworkRequestError::TemporaryProblem );
  BOOST_TEST_REQ_ERR( req504, zyppng::NetworkRequestError::Timeout );
  BOOST_TEST_REQ_ERR( req403, zyppng::NetworkRequestError::Forbidden );
  BOOST_TEST_REQ_ERR( req410, zyppng::NetworkRequestError::NotFound );
  BOOST_TEST_REQ_ERR( req418, zyppng::NetworkRequestError::ServerReturnedError );
}

BOOST_DATA_TEST_CASE(nwdispatcher_http_download, bdata::make( withSSL ), withSSL )
{
  auto ev = zyppng::EventDispatcher::createMain();
  zyppng::NetworkRequestDispatcher disp;
  disp.sigQueueFinished().connect( [&ev]( const zyppng::NetworkRequestDispatcher& ){
    ev->quit();
  });
  //start request dispatching, does not need to have requests enqueued
  disp.run();

  WebServer web((zypp::Pathname(TESTS_SRC_DIR)/"zypp/data/Fetcher/remote-site").c_str(), 10001, withSSL );
  web.start();

  zyppng::TransferSettings set = web.transferSettings();

  zyppng::Url weburl (web.url());
  weburl.setPathName("/complexdir/subdir1/subdir1-file1.txt");

  zypp::filesystem::TmpFile targetFile;
  zyppng::NetworkRequest::Ptr reqDLFile = std::make_shared<zyppng::NetworkRequest>( weburl, targetFile.path() );

  std::shared_ptr<zypp::Digest> dig = std::make_shared<zypp::Digest>();
  BOOST_REQUIRE_MESSAGE( dig->create( zypp::Digest::sha1() ), "Unable to create Digest " );

  reqDLFile->transferSettings() = set;
  reqDLFile->setDigest( dig );
  reqDLFile->setExpectedChecksum( convertHexStrToVector("f1d2d2f924e986ac86fdf7b36c94bcdf32beec15") );
  disp.enqueue( reqDLFile );
  ev->run();
  BOOST_TEST_REQ_SUCCESS( reqDLFile );

  //modify the checksum -> request should fail now
  reqDLFile->setExpectedChecksum( convertHexStrToVector("f1d2d2f924e986ac86fdf7b36c94bcdf32beec20") );
  disp.enqueue( reqDLFile );
  ev->run();
  BOOST_TEST_REQ_ERR( reqDLFile, zyppng::NetworkRequestError::InvalidChecksum );

  weburl = web.url();
  weburl.setPathName("/file-1.txt");
  reqDLFile = std::make_shared<zyppng::NetworkRequest>( weburl, targetFile.path() );
  reqDLFile->transferSettings() = set;
  reqDLFile->setUrl( weburl );
  reqDLFile->setRequestRange( 0, 7 );
  disp.enqueue( reqDLFile );
  ev->run();
  BOOST_TEST_REQ_SUCCESS( reqDLFile );
  {
    zypp::filesystem::PathInfo targetFileInfo( targetFile.path() );
    BOOST_REQUIRE( targetFileInfo.isExist() );
    BOOST_REQUIRE( targetFileInfo.isFile() );
    std::string fileSum = zypp::filesystem::md5sum( targetFile.path() );
    fileSum = zypp::str::trim( fileSum );
    BOOST_REQUIRE_EQUAL( std::string("16d2b386b2034b9488996466aaae0b57"), fileSum );
  }
}

BOOST_DATA_TEST_CASE(nwdispatcher_delay_download, bdata::make( withSSL ), withSSL )
{
  auto ev = zyppng::EventDispatcher::createMain();
  zyppng::NetworkRequestDispatcher disp;
  disp.sigQueueFinished().connect( [&ev]( const zyppng::NetworkRequestDispatcher& ){
    ev->quit();
  });

  disp.run();

  WebServer web((zypp::Pathname(TESTS_SRC_DIR)/"data"/"dummywebroot").c_str(), 10001, withSSL );

  web.addRequestHandler("stalled", []( WebServer::Request &r ){
    std::this_thread::sleep_for( std::chrono::milliseconds ( 2000 ) );
    r.rout << "Status: 200\r\n"
              "\r\n"
              "Hello";
  });

  web.start();

  zyppng::TransferSettings set = web.transferSettings();
  set.setTimeout( 1 );

  zypp::filesystem::TmpFile targetFile;

  zyppng::Url weburl (web.url());
  weburl.setPathName("/handler/stalled");

  zyppng::NetworkRequest::Ptr reqDLFile = std::make_shared<zyppng::NetworkRequest>( weburl, targetFile.path() );
  reqDLFile->transferSettings() = set;

  disp.enqueue( reqDLFile );
  ev->run();

  BOOST_TEST_REQ_ERR( reqDLFile, zyppng::NetworkRequestError::Timeout );
}

