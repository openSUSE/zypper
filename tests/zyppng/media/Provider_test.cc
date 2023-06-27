#include <zypp-media/ng/Provide>
#include <zypp-media/ng/ProvideSpec>
#include <zypp-media/MediaException>
#include <zypp-media/auth/AuthData>
#include <zypp-media/auth/CredentialManager>
#include <zypp-core/OnMediaLocation>
#include <zypp-core/zyppng/base/EventLoop>
#include <zypp-core/Pathname.h>
#include <zypp-core/Url.h>
#include <zypp-core/base/UserRequestException>
#include <zypp/ZConfig.h>
#include <zypp/TmpPath.h>
#include <zypp-core/zyppng/pipelines/Wait>
#include <zypp-core/zyppng/rpc/zerocopystreams.h>
#include <zypp-core/zyppng/base/private/linuxhelpers_p.h>

#include <zypp-proto/test/tvm.pb.h>
#include <iostream>
#include <fstream>

#include "WebServer.h"
#include "TestTools.h"

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#ifndef TESTS_BUILD_DIR
#error "TESTS_BUILD_DIR not defined"
#endif

#define ZYPP_REQUIRE_THROW( statement, exception ) \
  try { BOOST_REQUIRE_THROW(  statement, exception  ); } catch( ... ) { BOOST_FAIL( #statement" throws unexpected exception"); }

namespace bdata = boost::unit_test::data;

BOOST_AUTO_TEST_CASE( http_prov )
{
  using namespace zyppng::operators;

  auto ev = zyppng::EventLoop::create ();

  const auto &workerPath = zypp::Pathname ( TESTS_BUILD_DIR ).dirname() / "tools" / "workers";
  const auto &webRoot    = zypp::Pathname ( TESTS_SRC_DIR ) / "zyppng" / "data" / "downloader";

  zypp::filesystem::TmpDir provideRoot;

  auto prov = zyppng::Provide::create ( provideRoot );
  prov->setWorkerPath ( workerPath );
  prov->start();

  WebServer web( webRoot.c_str(), 10001, false );
  BOOST_REQUIRE( web.start() );

  auto fileUrl = web.url();
  fileUrl.setPathName( "/media.1/media" );
  auto op = prov->provide( fileUrl, zyppng::ProvideFileSpec() );

  std::exception_ptr err;
  std::optional<zyppng::ProvideRes> resOpt;
  op->onReady([&]( zyppng::expected<zyppng::ProvideRes> &&res ){
    ev->quit();
    if ( !res )
      err = res.error();
    else
      resOpt = *res;
  });

  BOOST_REQUIRE( !op->isReady() );

  if ( !op->isReady() )
    ev->run();

  BOOST_REQUIRE( !err );
  BOOST_REQUIRE( resOpt.has_value() );
  zypp::PathInfo pi( resOpt->file() );
  BOOST_REQUIRE( pi.isExist() );
  BOOST_REQUIRE( pi.isFile() );

  std::ifstream in( resOpt->file().asString(), std::ios::binary );
  auto sum = zypp::CheckSum::md5( in );
  BOOST_REQUIRE_EQUAL( sum, std::string("63b4a45ec881d90b83c2e6af7bcbfa78") );
}

BOOST_AUTO_TEST_CASE( http_attach )
{
  using namespace zyppng::operators;

  auto ev = zyppng::EventLoop::create ();

  const auto &workerPath = zypp::Pathname ( TESTS_BUILD_DIR ).dirname() / "tools" / "workers";
  const auto &webRoot    = zypp::Pathname (TESTS_SRC_DIR)/"/zyppng/data/downloader";
  zypp::filesystem::TmpDir provideRoot;

  auto prov = zyppng::Provide::create ( provideRoot );
  prov->setWorkerPath ( workerPath );
  prov->start();

  WebServer web( webRoot.c_str(), 10001, false );
  BOOST_REQUIRE( web.start() );

  auto op = prov->attachMedia( web.url(), zyppng::ProvideMediaSpec( "OnlineMedia" )
                                               .setMediaFile( webRoot / "media.1" / "media" )
                                               .setMedianr(1) );

  op->onReady([&]( zyppng::expected<zyppng::Provide::MediaHandle> &&res ){
    ev->quit();
    BOOST_REQUIRE( res.is_valid() );
    BOOST_REQUIRE( !res->handle().empty() );
  });

  BOOST_REQUIRE( !op->isReady() );

  if ( !op->isReady() )
    ev->run();
}

BOOST_AUTO_TEST_CASE( http_attach_prov )
{
  using namespace zyppng::operators;

  auto ev = zyppng::EventLoop::create ();

  const auto &workerPath = zypp::Pathname ( TESTS_BUILD_DIR ).dirname() / "tools" / "workers";
  const auto &webRoot    = zypp::Pathname ( TESTS_SRC_DIR ) / "zyppng" / "data" / "downloader";

  zypp::filesystem::TmpDir provideRoot;

  auto prov = zyppng::Provide::create ( provideRoot );
  prov->setWorkerPath ( workerPath );
  prov->start();

  WebServer web( webRoot.c_str(), 10001, false );
  BOOST_REQUIRE( web.start() );

  zyppng::Provide::MediaHandle media;

  auto op = prov->attachMedia( web.url(), zyppng::ProvideMediaSpec( "OnlineMedia" )
                                               .setMediaFile( webRoot / "media.1" / "media" )
                                               .setMedianr(1) )
            | mbind ( [&]( zyppng::Provide::MediaHandle &&res ){
              media = std::move(res);
              return prov->provide( media, "/test.txt", zyppng::ProvideFileSpec() );
            });

  std::optional<zyppng::ProvideRes> fileRes;
  op->onReady([&]( zyppng::expected<zyppng::ProvideRes> &&res ){
    if ( res )
      fileRes = std::move(*res);
    ev->quit();
  });

  BOOST_REQUIRE( !op->isReady() );

  if ( !op->isReady() )
    ev->run();

  BOOST_REQUIRE( fileRes.has_value() );
  zypp::PathInfo pi ( fileRes->file() );
  BOOST_REQUIRE( pi.isExist() && pi.isFile() );

  std::ifstream in( fileRes->file().asString(), std::ios::binary );
  auto sum = zypp::CheckSum::md5( in );
  BOOST_REQUIRE_EQUAL( sum, std::string("7e562d52c100b68e9d6a561fa8519575") );
}

BOOST_AUTO_TEST_CASE( http_attach_prov_404 )
{
  using namespace zyppng::operators;

  auto ev = zyppng::EventLoop::create ();

  const auto &workerPath = zypp::Pathname ( TESTS_BUILD_DIR ).dirname() / "tools" / "workers";
  const auto &webRoot    = zypp::Pathname ( TESTS_SRC_DIR ) / "zyppng" / "data" / "downloader";

  zypp::filesystem::TmpDir provideRoot;

  auto prov = zyppng::Provide::create ( provideRoot );
  prov->setWorkerPath ( workerPath );
  prov->start();

  WebServer web( webRoot.c_str(), 10001, false );
  BOOST_REQUIRE( web.start() );

  zyppng::Provide::MediaHandle media;

  auto op = prov->attachMedia( web.url(), zyppng::ProvideMediaSpec( "OnlineMedia" )
                                               .setMediaFile( webRoot / "media.1" / "media" )
                                               .setMedianr(1) )
            | mbind ( [&]( zyppng::Provide::MediaHandle &&res ){
              media = std::move(res);
              return prov->provide( media, "/doesnotexist", zyppng::ProvideFileSpec() );
            });

  std::optional<zyppng::ProvideRes> fileRes;
  op->onReady([&]( zyppng::expected<zyppng::ProvideRes> &&res ){
    ev->quit();

    BOOST_REQUIRE(!res);
    if ( !res ) {
      ZYPP_REQUIRE_THROW( std::rethrow_exception( res.error() ), zypp::media::MediaFileNotFoundException );
    }

  });

  BOOST_REQUIRE( !op->isReady() );

  if ( !op->isReady() )
    ev->run();
}


// special case where we can detect that something is a directory because we already provided a file from the
// directory that's required as a file.
BOOST_AUTO_TEST_CASE( http_attach_prov_notafile )
{
  using namespace zyppng::operators;

  auto ev = zyppng::EventLoop::create ();

  const auto &workerPath = zypp::Pathname ( TESTS_BUILD_DIR ).dirname() / "tools" / "workers";
  const auto &webRoot    = zypp::Pathname ( TESTS_SRC_DIR ) / "zyppng" / "data" / "downloader";

  zypp::filesystem::TmpDir provideRoot;

  auto prov = zyppng::Provide::create ( provideRoot );
  prov->setWorkerPath ( workerPath );
  prov->start();

  WebServer web( webRoot.c_str(), 10001, false );
  BOOST_REQUIRE( web.start() );

  zyppng::Provide::MediaHandle media;
  std::exception_ptr err;

  auto op = prov->attachMedia( web.url(), zyppng::ProvideMediaSpec( "OnlineMedia" )
                                               .setMediaFile( webRoot / "media.1" / "media" )
                                               .setMedianr(1) )
            | mbind ( [&]( zyppng::Provide::MediaHandle &&res ){
              media = std::move(res);
              return prov->provide( media, "/media.1", zyppng::ProvideFileSpec() );
            });

  op->onReady([&]( zyppng::expected<zyppng::ProvideRes> &&res ){
    ev->quit();
    if ( !res )
      err = res.error();
  });

  BOOST_REQUIRE( !op->isReady() );

  if ( !op->isReady() )
    ev->run();

  BOOST_REQUIRE( err );
  ZYPP_REQUIRE_THROW( std::rethrow_exception( err ), zypp::media::MediaNotAFileException );
}

//creates a request handler that requires a authentication to work
WebServer::RequestHandler createAuthHandler ( const zypp::Pathname &webroot = "/" )
{
  return [ webroot ]( WebServer::Request &req ){
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

    //remove "/handler/" prefix
    std::string key = req.params.at("SCRIPT_NAME").substr( std::string_view("/handler").length() );

    req.rout << "Status: 307 Temporary Redirect\r\n"
                "Location: "<< webroot / key <<"\r\n\r\n";
    return;
  };
};

BOOST_AUTO_TEST_CASE( http_prov_auth )
{
  using namespace zyppng::operators;

  auto ev = zyppng::EventLoop::create ();

  const auto &workerPath = zypp::Pathname ( TESTS_BUILD_DIR ).dirname() / "tools" / "workers";
  const auto &webRoot    = zypp::Pathname ( TESTS_SRC_DIR ) / "zyppng" / "data" / "downloader";

  zypp::filesystem::TmpDir provideRoot;

  auto prov = zyppng::Provide::create ( provideRoot );
  prov->setWorkerPath ( workerPath );
  prov->start();

  //don't write or read creds from real settings dir
  zypp::filesystem::TmpDir globCredPath;
  zypp::filesystem::TmpDir userCredPath;
  zypp::media::CredManagerOptions opts;
  opts.globalCredFilePath = globCredPath.path() / "credentials.cat";
  opts.userCredFilePath   = userCredPath.path() / "credentials.cat";
  prov->setCredManagerOptions( opts );

  bool gotSigAuthRequired = false;
  prov->sigAuthRequired().connect( [&]( const zypp::Url &reqUrl, const std::string &triedUsername, const std::map<std::string, std::string> &extraValues ) -> std::optional<zypp::media::AuthData> {
    gotSigAuthRequired = true;
    zypp::media::AuthData auth( reqUrl );
    auth.setUsername( "test" );
    auth.setPassword( "test" );
    auth.setLastDatabaseUpdate( time( nullptr ) );
    return auth;
  });

  WebServer web( webRoot.c_str(), 10001, false );
  BOOST_REQUIRE( web.start() );

  web.setDefaultHandler( createAuthHandler() );

  auto fileUrl = web.url();
  fileUrl.setPathName( "/handler/test.txt" );
  auto op = prov->provide( fileUrl, zyppng::ProvideFileSpec() );

  std::exception_ptr err;
  std::optional<zyppng::ProvideRes> resOpt;
  op->onReady([&]( zyppng::expected<zyppng::ProvideRes> &&res ){
    ev->quit();
    if ( !res )
      err = res.error();
    else
      resOpt = *res;
  });

  BOOST_REQUIRE( !op->isReady() );

  if ( !op->isReady() )
    ev->run();

  //BOOST_REQUIRE( gotSigAuthRequired );
  BOOST_REQUIRE( !err );
  BOOST_REQUIRE( resOpt.has_value() );
  zypp::PathInfo pi( resOpt->file() );
  BOOST_REQUIRE( pi.isExist() );
  BOOST_REQUIRE( pi.isFile() );

  std::ifstream in( resOpt->file().asString(), std::ios::binary );
  auto sum = zypp::CheckSum::md5( in );
  BOOST_REQUIRE_EQUAL( sum, std::string("7e562d52c100b68e9d6a561fa8519575") );
}

BOOST_AUTO_TEST_CASE( http_prov_auth_nouserresponse )
{
  using namespace zyppng::operators;

  //don't write or read creds from real settings dir
  zypp::filesystem::TmpDir repoManagerRoot;
  zypp::ZConfig::instance().setRepoManagerRoot( repoManagerRoot.path() );

  auto ev = zyppng::EventLoop::create ();

  const auto &workerPath = zypp::Pathname ( TESTS_BUILD_DIR ).dirname() / "tools" / "workers";
  const auto &webRoot    = zypp::Pathname ( TESTS_SRC_DIR ) / "zyppng" / "data" / "downloader";

  zypp::filesystem::TmpDir provideRoot;

  auto prov = zyppng::Provide::create ( provideRoot );
  prov->setWorkerPath ( workerPath );
  prov->start();

  //don't write or read creds from real settings dir
  zypp::filesystem::TmpDir globCredPath;
  zypp::filesystem::TmpDir userCredPath;
  zypp::media::CredManagerOptions opts;
  opts.globalCredFilePath = globCredPath.path() / "credentials.cat";
  opts.userCredFilePath   = userCredPath.path() / "credentials.cat";
  prov->setCredManagerOptions( opts );

  bool gotSigAuthRequired = false;
  prov->sigAuthRequired().connect( [&]( const zypp::Url &, const std::string &, const std::map<std::string, std::string> & ) -> std::optional<zypp::media::AuthData> {
    gotSigAuthRequired = true;
    return {};
  });

  WebServer web( webRoot.c_str(), 10001, false );
  BOOST_REQUIRE( web.start() );

  web.setDefaultHandler( createAuthHandler() );

  auto fileUrl = web.url();
  fileUrl.setPathName( "/handler/test.txt" );
  auto op = prov->provide( fileUrl, zyppng::ProvideFileSpec() );

  std::exception_ptr err;
  std::optional<zyppng::ProvideRes> resOpt;
  op->onReady([&]( zyppng::expected<zyppng::ProvideRes> &&res ){
    ev->quit();
    if ( !res )
      err = res.error();
    else
      resOpt = *res;
  });

  BOOST_REQUIRE( !op->isReady() );

  if ( !op->isReady() )
    ev->run();

  BOOST_REQUIRE( gotSigAuthRequired );
  BOOST_REQUIRE( err );
  BOOST_REQUIRE( !resOpt.has_value() );
  ZYPP_REQUIRE_THROW( std::rethrow_exception( err ), zypp::media::MediaUnauthorizedException );
}

BOOST_AUTO_TEST_CASE( http_prov_auth_wrongpw )
{
  using namespace zyppng::operators;

  //don't write or read creds from real settings dir
  zypp::filesystem::TmpDir repoManagerRoot;
  zypp::ZConfig::instance().setRepoManagerRoot( repoManagerRoot.path() );

  auto ev = zyppng::EventLoop::create ();

  const auto &workerPath = zypp::Pathname ( TESTS_BUILD_DIR ).dirname() / "tools" / "workers";
  const auto &webRoot    = zypp::Pathname ( TESTS_SRC_DIR ) / "zyppng" / "data" / "downloader";

  zypp::filesystem::TmpDir provideRoot;

  auto prov = zyppng::Provide::create ( provideRoot );
  prov->setWorkerPath ( workerPath );
  prov->start();

  //don't write or read creds from real settings dir
  zypp::filesystem::TmpDir globCredPath;
  zypp::filesystem::TmpDir userCredPath;
  zypp::media::CredManagerOptions opts;
  opts.globalCredFilePath = globCredPath.path() / "credentials.cat";
  opts.userCredFilePath   = userCredPath.path() / "credentials.cat";
  prov->setCredManagerOptions( opts );

  int cntAuthRequired = 0;
  prov->sigAuthRequired().connect( [&]( const zypp::Url &reqUrl, const std::string &, const std::map<std::string, std::string> & ) -> std::optional<zypp::media::AuthData> {
    cntAuthRequired++;
    zypp::media::AuthData auth( reqUrl );
    auth.setLastDatabaseUpdate( time( nullptr ) );
    if ( cntAuthRequired >= 2 ) {
      auth.setUsername( "test" );
      auth.setPassword( "test" );
    } else {
      auth.setUsername( "test" );
      auth.setPassword( "wrong" );
    }

    return auth;
  });

  WebServer web( webRoot.c_str(), 10001, false );
  BOOST_REQUIRE( web.start() );

  web.setDefaultHandler( createAuthHandler() );

  auto fileUrl = web.url();
  fileUrl.setPathName( "/handler/test.txt" );
  auto op = prov->provide( fileUrl, zyppng::ProvideFileSpec() );

  std::exception_ptr err;
  std::optional<zyppng::ProvideRes> resOpt;
  op->onReady([&]( zyppng::expected<zyppng::ProvideRes> &&res ){
    ev->quit();
    if ( !res )
      err = res.error();
    else
      resOpt = *res;
  });

  BOOST_REQUIRE( !op->isReady() );

  if ( !op->isReady() )
    ev->run();

  BOOST_REQUIRE_EQUAL( cntAuthRequired, 2 );
  BOOST_REQUIRE( !err );
  BOOST_REQUIRE( resOpt.has_value() );
  zypp::PathInfo pi( resOpt->file() );
  BOOST_REQUIRE( pi.isExist() );
  BOOST_REQUIRE( pi.isFile() );

  std::ifstream in( resOpt->file().asString(), std::ios::binary );
  auto sum = zypp::CheckSum::md5( in );
  BOOST_REQUIRE_EQUAL( sum, std::string("7e562d52c100b68e9d6a561fa8519575") );
}

BOOST_AUTO_TEST_CASE( http_attach_prov_auth )
{
  using namespace zyppng::operators;

  auto ev = zyppng::EventLoop::create ();

  const auto &workerPath = zypp::Pathname ( TESTS_BUILD_DIR ).dirname() / "tools" / "workers";
  const auto &webRoot    = zypp::Pathname ( TESTS_SRC_DIR ) / "zyppng" / "data" / "downloader";

  zypp::filesystem::TmpDir provideRoot;

  auto prov = zyppng::Provide::create ( provideRoot );
  prov->setWorkerPath ( workerPath );

  //don't write or read creds from real settings dir
  zypp::filesystem::TmpDir globCredPath;
  zypp::filesystem::TmpDir userCredPath;
  zypp::media::CredManagerOptions opts;
  opts.globalCredFilePath = globCredPath.path() / "credentials.cat";
  opts.userCredFilePath   = userCredPath.path() / "credentials.cat";
  prov->setCredManagerOptions( opts );

  int cntAuthRequired = 0;
  prov->sigAuthRequired().connect( [&]( const zypp::Url &reqUrl, const std::string &, const std::map<std::string, std::string> & ) -> std::optional<zypp::media::AuthData> {
    cntAuthRequired++;
    zypp::media::AuthData auth( reqUrl );
    auth.setLastDatabaseUpdate( time( nullptr ) );
    auth.setUsername( "test" );
    auth.setPassword( "test" );
    return auth;
  });

  prov->start();

  WebServer web( webRoot.c_str(), 10001, false );
  web.setDefaultHandler( createAuthHandler() );
  BOOST_REQUIRE( web.start() );

  zyppng::Provide::MediaHandle media;

  auto baseUrl = web.url();
  baseUrl.setPathName("/handler");

  auto op = prov->attachMedia( baseUrl, zyppng::ProvideMediaSpec( "OnlineMedia" )
                                               .setMediaFile( webRoot / "media.1" / "media" )
                                               .setMedianr(1) )
            | mbind ( [&]( zyppng::Provide::MediaHandle &&res ){
              media = std::move(res);
              return prov->provide( media, "/test.txt", zyppng::ProvideFileSpec() );
            });

  std::optional<zyppng::ProvideRes> fileRes;
  op->onReady([&]( zyppng::expected<zyppng::ProvideRes> &&res ){
    if ( res )
      fileRes = std::move(*res);
    ev->quit();
  });

  BOOST_REQUIRE( !op->isReady() );

  if ( !op->isReady() )
    ev->run();

  BOOST_REQUIRE_EQUAL( cntAuthRequired, 1 ); // after the first auth request all others should work
  BOOST_REQUIRE( fileRes.has_value() );
  zypp::PathInfo pi ( fileRes->file() );
  BOOST_REQUIRE( pi.isExist() && pi.isFile() );

  std::ifstream in( fileRes->file().asString(), std::ios::binary );
  auto sum = zypp::CheckSum::md5( in );
  BOOST_REQUIRE_EQUAL( sum, std::string("7e562d52c100b68e9d6a561fa8519575") );
}

static void writeTVMConfig ( const zypp::Pathname &file, const zypp::proto::test::TVMSettings &set )
{
  const auto &fname = file/"tvm.conf";
  int fd = open( fname.asString().data(), O_WRONLY | O_CREAT | O_TRUNC, 0666 );
  if ( fd < 0 ) {
    ERR << "Failed to open/create file " << fname << " error: "<<zyppng::strerr_cxx(errno)<< " " << errno << std::endl;
    return;
  }
  zyppng::FileOutputStream out( fd );
  out.SetCloseOnDelete( true );
  if ( !set.SerializeToZeroCopyStream( &out ) )
    ERR << "Failed to serialize settings" << std::endl;
  MIL << "TVM config serialized to: " << fname << std::endl;
}

static auto makeDVDProv ( zyppng::ProvideRef &prov, const zypp::filesystem::Pathname &devRoot, int mediaNr, const std::string &fName )
{
  using namespace zyppng::operators;

  return prov->attachMedia( zypp::Url("tvm:/"), zyppng::ProvideMediaSpec("CD Test Set")
                                                .setMediaFile( devRoot / (std::string("cd") + zypp::str::numstring(mediaNr)) / (std::string("media.") + zypp::str::numstring(mediaNr))  / "media" )
                                                .setMedianr(mediaNr))
        | [&prov, mediaNr, fName]( zyppng::expected<zyppng::Provide::MediaHandle> &&res ){
            if ( res ) {
              std::cout << "Attached " << mediaNr << " as: " << res->handle() << std::endl;
              return prov->provide ( *res, fName , zyppng::ProvideFileSpec() ) | [ attachId = *res, &prov ]( auto &&res ) {
                if ( !res ) {
                  try {
                    std::rethrow_exception(res.error());
                  }  catch ( const zypp::Exception &e ) {
                    std::cout << "Provide failed with " << e <<std::endl;
                  }

                } else
                  std::cout << "File provided to : " << res->file () <<std::endl;

                std::cout << "DETACHING " << attachId.handle() << std::endl;
                return res;
              };
            } else {
              std::cout << "Failed to attach media" << std::endl;
              return zyppng::makeReadyResult( zyppng::expected<zyppng::ProvideRes>::error(res.error()) );
            }
          } | [](  zyppng::expected<zyppng::ProvideRes> &&res ) {
          if ( !res ) {
            try {
              std::rethrow_exception(res.error());
            }  catch ( const zypp::Exception &e ) {
              std::cout << "Provide failed with " << e <<std::endl;
              return zyppng::expected<void>::error(res.error());
            } catch ( ... ) {
              std::cout << "Provide failed with exception " << std::endl;
              return zyppng::expected<void>::error(res.error());
            }
          } else {
            return zyppng::expected<void>::success();
          }
      };
  }

BOOST_AUTO_TEST_CASE( tvm_basic )
{
  using namespace zyppng::operators;

  auto ev = zyppng::EventLoop::create ();

  const auto &workerPath = zypp::Pathname ( TESTS_BUILD_DIR ).dirname() / "tools" / "workers";
  const auto &devRoot    = zypp::Pathname ( TESTS_SRC_DIR ) / "zyppng" / "data" / "provide";

  zypp::filesystem::TmpDir provideRoot;

  auto prov = zyppng::Provide::create ( provideRoot );
  prov->setWorkerPath ( workerPath );

  zypp::proto::test::TVMSettings devSet;
  auto dev = devSet.add_devices();
  dev->set_name("/fakedev/tvm/slot1");
  dev->set_insertedpath( (devRoot/"cd1").asString() );
  dev = devSet.add_devices();
  dev->set_name("/fakedev/tvm/slot2");
  dev->set_insertedpath( (devRoot/"cd2").asString() );
  dev = devSet.add_devices();
  dev->set_name("/fakedev/tvm/slot3");
  dev->set_insertedpath( (devRoot/"cd3").asString() );
  writeTVMConfig( provideRoot, devSet );

  prov->start();

  std::vector< zyppng::AsyncOpRef<zyppng::expected<void>>> ops;

  ops.push_back( makeDVDProv( prov, devRoot, 1, "/file1") );
  ops.push_back( makeDVDProv( prov, devRoot, 2, "/file2") );
  ops.push_back( makeDVDProv( prov, devRoot, 3, "/file3") );

  auto r = std::move(ops) | zyppng::waitFor<zyppng::expected<void>>();
  r->sigReady().connect([&](){
    ev->quit();
  });

  BOOST_REQUIRE( !r->isReady() );

  if ( !r->isReady() )
    ev->run();

  for ( const auto &res : r->get() ) {
    BOOST_REQUIRE(res);
  }
}

BOOST_AUTO_TEST_CASE( tvm_medchange )
{
  using namespace zyppng::operators;

  auto ev = zyppng::EventLoop::create ();

  const auto &workerPath = zypp::Pathname ( TESTS_BUILD_DIR ).dirname() / "tools" / "workers";
  const auto &devRoot    = zypp::Pathname ( TESTS_SRC_DIR ) / "zyppng" / "data" / "provide";

  zypp::filesystem::TmpDir provideRoot;

  auto prov = zyppng::Provide::create ( provideRoot );
  prov->setWorkerPath ( workerPath );

  zypp::proto::test::TVMSettings devSet;
  auto dev = devSet.add_devices();
  dev->set_name("/fakedev/tvm/slot1");
  dev->set_insertedpath( (devRoot/"cd1").asString() );
  dev = devSet.add_devices();
  dev->set_name("/fakedev/tvm/slot2");
  dev = devSet.add_devices();
  dev->set_name("/fakedev/tvm/slot3");
  writeTVMConfig( provideRoot, devSet );

  prov->start();

  bool gotMediaChange = false;
  std::vector<std::string> freeDevices;
  int32_t mediaNrAsked = -1;
  std::string labelAsked;
  prov->sigMediaChangeRequested().connect([&]( const std::string &ref, const std::string &label, const int32_t mediaNr, const std::vector<std::string> &devices, const std::optional<std::string> &desc ){
    gotMediaChange = true;
    freeDevices = devices;
    labelAsked = label;
    mediaNrAsked = mediaNr;
    devSet.mutable_devices(1)->set_insertedpath( (devRoot/"cd2").asString() );
    writeTVMConfig( provideRoot, devSet );
    return zyppng::Provide::RETRY;
  });

  std::vector< zyppng::AsyncOpRef<zyppng::expected<void>>> ops;

  ops.push_back( makeDVDProv( prov, devRoot, 1, "/file1") );
  ops.push_back( makeDVDProv( prov, devRoot, 2, "/file2") );

  auto r = std::move(ops) | zyppng::waitFor<zyppng::expected<void>>();
  r->sigReady().connect([&](){
    ev->quit();
  });

  BOOST_REQUIRE( !r->isReady() );

  if ( !r->isReady() )
    ev->run();

  BOOST_REQUIRE( gotMediaChange );
  BOOST_REQUIRE_EQUAL( labelAsked, std::string("CD Test Set") );
  BOOST_REQUIRE_EQUAL( mediaNrAsked, 2 );
  BOOST_REQUIRE( std::find( freeDevices.begin(), freeDevices.end(), "/fakedev/tvm/slot2" ) != freeDevices.end() );
  BOOST_REQUIRE( std::find( freeDevices.begin(), freeDevices.end(), "/fakedev/tvm/slot3" ) != freeDevices.end() );
  for ( const auto &res : r->get() ) {
    BOOST_REQUIRE(res);
  }
}

zyppng::Provide::Action cancelOps[] = { zyppng::Provide::ABORT, zyppng::Provide::SKIP };

BOOST_DATA_TEST_CASE( tvm_medchange_abort, bdata::make( cancelOps ), cancelOp )
{
  using namespace zyppng::operators;

  auto ev = zyppng::EventLoop::create ();

  const auto &workerPath = zypp::Pathname ( TESTS_BUILD_DIR ).dirname() / "tools" / "workers";
  const auto &devRoot    = zypp::Pathname ( TESTS_SRC_DIR ) / "zyppng" / "data" / "provide";

  zypp::filesystem::TmpDir provideRoot;

  auto prov = zyppng::Provide::create ( provideRoot );
  prov->setWorkerPath ( workerPath );

  zypp::proto::test::TVMSettings devSet;
  auto dev = devSet.add_devices();
  dev->set_name("/fakedev/tvm/slot1");
  dev->set_insertedpath( (devRoot/"cd1").asString() );
  dev = devSet.add_devices();
  dev->set_name("/fakedev/tvm/slot2");
  dev = devSet.add_devices();
  dev->set_name("/fakedev/tvm/slot3");
  writeTVMConfig( provideRoot, devSet );

  prov->start();

  bool gotMediaChange = false;
  std::vector<std::string> freeDevices;
  int32_t mediaNrAsked = -1;
  std::string labelAsked;
  prov->sigMediaChangeRequested().connect([&]( const std::string &ref, const std::string &label, const int32_t mediaNr, const std::vector<std::string> &devices, const std::optional<std::string> &desc ){
    gotMediaChange = true;
    freeDevices = devices;
    labelAsked = label;
    mediaNrAsked = mediaNr;
    return cancelOp;
  });

  auto op1 = makeDVDProv( prov, devRoot, 1, "/file1");
  auto op2 = makeDVDProv( prov, devRoot, 2, "/file2");

  const auto &readyCB = [&](){
    if ( op1->isReady() && op2->isReady() )
      ev->quit();
  };

  op1->sigReady().connect( readyCB );
  op2->sigReady().connect( readyCB );

  BOOST_REQUIRE( !op1->isReady() );
  BOOST_REQUIRE( !op2->isReady() );


  if ( !op1->isReady() && !op2->isReady() )
    ev->run();

  BOOST_REQUIRE( gotMediaChange );
  BOOST_REQUIRE_EQUAL( labelAsked, std::string("CD Test Set") );
  BOOST_REQUIRE_EQUAL( mediaNrAsked, 2 );
  BOOST_REQUIRE( std::find( freeDevices.begin(), freeDevices.end(), "/fakedev/tvm/slot2" ) != freeDevices.end() );
  BOOST_REQUIRE( std::find( freeDevices.begin(), freeDevices.end(), "/fakedev/tvm/slot3" ) != freeDevices.end() );
  BOOST_REQUIRE( op1->get().is_valid() );
  BOOST_REQUIRE( !op2->get().is_valid() );
  if ( cancelOp == zyppng::Provide::ABORT ) {
    ZYPP_REQUIRE_THROW( std::rethrow_exception( op2->get().error() ), zypp::AbortRequestException );
  } else {
    ZYPP_REQUIRE_THROW( std::rethrow_exception( op2->get().error() ), zypp::SkipRequestException );
  }
}

BOOST_AUTO_TEST_CASE( tvm_jammed )
{
  using namespace zyppng::operators;

  auto ev = zyppng::EventLoop::create ();

  const auto &workerPath = zypp::Pathname ( TESTS_BUILD_DIR ).dirname() / "tools" / "workers";
  const auto &devRoot    = zypp::Pathname ( TESTS_SRC_DIR ) / "zyppng" / "data" / "provide";

  zypp::filesystem::TmpDir provideRoot;

  auto prov = zyppng::Provide::create ( provideRoot );
  prov->setWorkerPath ( workerPath );

  bool gotMediaChange = false;
  prov->sigMediaChangeRequested().connect([&]( const std::string &ref, const std::string &label, const int32_t mediaNr, const std::vector<std::string> &devices, const std::optional<std::string> &desc ){
    gotMediaChange = true;
    return zyppng::Provide::ABORT;
  });

  zypp::proto::test::TVMSettings devSet;
  auto dev = devSet.add_devices();
  dev->set_name("/fakedev/tvm/slot1");
  dev->set_insertedpath( (devRoot/"cd1").asString() );
  writeTVMConfig( provideRoot, devSet );

  prov->start();

  auto op1 = prov->attachMedia( zypp::Url("tvm:/"), zyppng::ProvideMediaSpec("CD Test Set")
                                                .setMediaFile( devRoot / (std::string("cd") + zypp::str::numstring(1)) / (std::string("media.") + zypp::str::numstring(1))  / "media" )
                                                .setMedianr(1));
  auto op2 = prov->attachMedia( zypp::Url("tvm:/"), zyppng::ProvideMediaSpec("CD Test Set")
                                                .setMediaFile( devRoot / (std::string("cd") + zypp::str::numstring(2)) / (std::string("media.") + zypp::str::numstring(2))  / "media" )
                                                .setMedianr(2));



  const auto &readyCB = [&](){
    if ( op1->isReady() && op2->isReady() )
      ev->quit();
  };

  op1->sigReady().connect( readyCB );
  op2->sigReady().connect( readyCB );


  BOOST_REQUIRE( !op1->isReady() );
  BOOST_REQUIRE( !op2->isReady() );


  if ( !op1->isReady() && !op2->isReady() )
    ev->run();

  //we have only one drive which is in use, so we can  not be asked for media change
  BOOST_REQUIRE( !gotMediaChange );
  BOOST_REQUIRE( op1->get().is_valid() );
  BOOST_REQUIRE( !op2->get().is_valid() );
  ZYPP_REQUIRE_THROW( std::rethrow_exception( op2->get().error() ), zypp::media::MediaJammedException );
}

// test if we can release a medium and immediately use the resulting free space for new requests
BOOST_AUTO_TEST_CASE( tvm_jammed_release )
{
  using namespace zyppng::operators;

  auto ev = zyppng::EventLoop::create ();

  const auto &workerPath = zypp::Pathname ( TESTS_BUILD_DIR ).dirname() / "tools" / "workers";
  const auto &devRoot    = zypp::Pathname ( TESTS_SRC_DIR ) / "zyppng" / "data" / "provide";

  zypp::filesystem::TmpDir provideRoot;

  auto prov = zyppng::Provide::create ( provideRoot );
  prov->setWorkerPath ( workerPath );

  zypp::proto::test::TVMSettings devSet;

  bool mediaChangeAllowed = false;
  bool gotInvalidMediaChange = false;
  prov->sigMediaChangeRequested().connect([&]( const std::string &ref, const std::string &label, const int32_t mediaNr, const std::vector<std::string> &devices, const std::optional<std::string> &desc ){
    if ( ! mediaChangeAllowed ) {
      gotInvalidMediaChange = true;
      return zyppng::Provide::ABORT;
    }

    devSet.mutable_devices(0)->set_insertedpath( (devRoot/"cd2").asString() );
    writeTVMConfig( provideRoot, devSet );
    return zyppng::Provide::RETRY;
  });

  auto dev = devSet.add_devices();
  dev->set_name("/fakedev/tvm/slot1");
  dev->set_insertedpath( (devRoot/"cd1").asString() );
  writeTVMConfig( provideRoot, devSet );

  prov->start();

  auto op1 = prov->attachMedia( zypp::Url("tvm:/"), zyppng::ProvideMediaSpec("CD Test Set")
                                                .setMediaFile( devRoot / (std::string("cd") + zypp::str::numstring(1)) / (std::string("media.") + zypp::str::numstring(1))  / "media" )
                                                .setMedianr(1));

  zyppng::AsyncOpRef<zyppng::expected<zyppng::Provide::MediaHandle>> op2;
  zyppng::AsyncOpRef<zyppng::expected<zyppng::Provide::MediaHandle>> op3;

  bool attach1Success = false;
  bool attach3Success = false;
  std::exception_ptr op2Error;
  op1->sigReady().connect( [&]() {
    if ( !op1->get() ) {
      attach1Success = false;
      ev->quit();
      return;
    }

    attach1Success = true;
    op2 = prov->attachMedia( zypp::Url("tvm:/"), zyppng::ProvideMediaSpec("CD Test Set")
                                                .setMediaFile( devRoot / (std::string("cd") + zypp::str::numstring(2)) / (std::string("media.") + zypp::str::numstring(2))  / "media" )
                                                .setMedianr(2));

    op2->sigReady().connect([&](){
      if ( !op2->get().is_valid() ) {
        op2Error = op2->get().error();

        op1.reset(); // kill the first media handle
        mediaChangeAllowed = true;

        op3 = prov->attachMedia( zypp::Url("tvm:/"), zyppng::ProvideMediaSpec("CD Test Set")
                                                .setMediaFile( devRoot / (std::string("cd") + zypp::str::numstring(2)) / (std::string("media.") + zypp::str::numstring(2))  / "media" )
                                                .setMedianr(2));
        op3->sigReady().connect( [&]() {
          ev->quit();
          if ( !op3->get() )
            attach3Success = false;
          else
            attach3Success = true;
        });
        return;
      }
      ev->quit();
    });
  });

  BOOST_REQUIRE( !op1->isReady() );
  BOOST_REQUIRE( !op2 || !op2->isReady() );
  BOOST_REQUIRE( !op3 || !op3->isReady() );

  if ( !op1->isReady() )
    ev->run();

  //we have only one drive which is in use, so we can  not be asked for media change
  BOOST_REQUIRE( !gotInvalidMediaChange );
  BOOST_REQUIRE( attach1Success );
  BOOST_REQUIRE( op2Error != nullptr );
  ZYPP_REQUIRE_THROW( std::rethrow_exception( op2Error ), zypp::media::MediaJammedException );
  BOOST_REQUIRE( attach3Success );
  BOOST_REQUIRE( !op2->get().is_valid() );
}


#if 0
BOOST_AUTO_TEST_CASE( dltest_basic )
{
  using namespace zyppng::operators;

  auto ev = zyppng::EventLoop::create ();
  auto prov = zyppng::Provide::create ( "/tmp/provTest" );
  prov->setWorkerPath ( "/home/zbenjamin/build/build-libzypp-Desktop_GCC-Debug/tools/workers" );
  prov->start();

  std::string baseURL = "http://download.opensuse.org/distribution/leap/15.0/repo/oss";
  std::vector<std::string> downloads {
    "11822f1421ae50fb1a07f72220b79000", "/x86_64/0ad-0.0.22-lp150.2.10.x86_64.rpm",
    "b0aaaca4c3763792a495de293c8431c5", "/x86_64/alsa-1.1.5-lp150.4.3.x86_64.rpm",
    "a6eb92351c03bcf603a09a2e8eddcead", "/x86_64/amarok-2.9.0-lp150.2.1.x86_64.rpm",
    "a2fd84f6d0530abbfe6d5a3da3940d70", "/x86_64/aspell-0.60.6.1-lp150.1.15.x86_64.rpm",
    "29b5eab4a9620f158331106df4603866", "/x86_64/atk-devel-2.26.1-lp150.2.4.x86_64.rpm",
    "a795874986018674c37af85a62c8f28a", "/x86_64/bing-1.0.5-lp150.2.1.x86_64.rpm",
    "ce09cb1af156203c89312f9faffce219", "/x86_64/cdrtools-3.02~a09-lp150.2.11.x86_64.rpm",
    "3f57113bd0dea2b5d748b7a343a7fb31", "/x86_64/cfengine-3.11.0-lp150.2.3.x86_64.rpm",
    "582cae086f67382e71bf02ff0db554cd", "/x86_64/cgit-1.1-lp150.1.5.x86_64.rpm",
    "f10cfc37a20c13d59266241ecc30b152", "/x86_64/ck-devel-0.6.0-lp150.1.3.x86_64.rpm",
    "4a19708dc8d58129f8832d934c47888b", "/x86_64/cloud-init-18.2-lp150.1.1.x86_64.rpm",
    "7b535587d9bfd8b88edf57b6df5c4d99", "/x86_64/collectd-plugin-pinba-5.7.2-lp150.1.4.x86_64.rpm",
    "d86c1d65039645a1895f458f38d9d9e7", "/x86_64/compface-1.5.2-lp150.1.4.x86_64.rpm",
    "33a0e878c92b5b298cd6aaec44c0aa46", "/x86_64/compositeproto-devel-0.4.2-lp150.1.6.x86_64.rpm",
    "646c6cc180caf27f56bb9ec5b4d50f5b", "/x86_64/corosync-testagents-2.4.4-lp150.3.1.x86_64.rpm",
    "10685e733abf77e7439e33471b23612c", "/x86_64/cpupower-bench-4.15-lp150.1.4.x86_64.rpm"
  };

  prov->sigMediaChangeRequested().connect([&]( const std::string &ref, const std::string &label, const int32_t mediaNr, const std::vector<std::string> &devices, const std::optional<std::string> &desc ){

    std::cout << "Please insert medium: " << mediaNr << " of media set " << label << " into one of the free devices: " << std::endl;
    if ( desc )
      std::cout << "Media desc: " << *desc << std::endl;

    std::vector<std::string> choices;
    for ( uint i = 0 ; i < devices.size (); i++ ) {
      std::cout << "(" << i << ") Free device: " << devices.at(i) << std::endl;
      choices.push_back( zypp::str::numstring(i));
    }

    while ( true ) {
      std::string choice;
      std::cout << "Select the device you want to use, or a for abort" << std::endl;
      std::getline( std::cin, choice );
      if ( choice == "a" )
        return zyppng::Provide::ABORT;

      auto i = std::find( choices.begin(), choices.end(), choice );
      if ( i == choices.end() ) {
        std::cout << "Invalid answer, please select a device from the list" << std::endl;
        continue;
      }

      auto sel = std::distance( choices.begin(), i );
      prov->ejectDevice( ref, devices.at(sel) );
      break;
    }

    std::string dummy;
    std::cout << "Please insert the medium and press a key to continue" << std::endl;
    std::getline( std::cin, dummy );

    return zyppng::Provide::RETRY;
  });

  prov->sigAuthRequired().connect( [&]( const zypp::Url &reqUrl, const std::string &triedUsername, const std::map<std::string, std::string> &extraValues ) -> std::optional<zypp::media::AuthData> {
      std::cout << reqUrl << " requires authentication, previously tried username was: " << triedUsername << std::endl;
      std::cout << "Please enter the username: " << std::endl;
      std::string username;
      std::getline ( std::cin, username );
      std::cout << "Please enter the password: " << std::endl;
      std::string pass;
      std::getline ( std::cin, pass );
      zypp::media::AuthData d( username, pass );
      return d;
  });

  std::vector< zyppng::AsyncOpRef<zyppng::expected<void>>> ops;
#if 0
  ops.push_back( prov->attachMedia ( baseURL, zyppng::ProvideMediaSpec( "Label1" ).setMediaFile( "/tmp/provTest/media" ).setMedianr(1) )
    | [&]( zyppng::expected<std::string> &&res ){
        if ( res ) {
          std::cout << "Attached as: " << *res << std::endl;

          std::vector<zyppng::AsyncOpRef<zyppng::expected<zyppng::ProvideRes>>> provs;
          for ( std::size_t i = 0; i < downloads.size(); i+=2 ) {
            const auto &pName = zypp::Pathname(downloads[i+1]);
            std::cout << "Asking to provide file: " << pName << std::endl;
            provs.push_back( prov->provide ( *res, pName, zyppng::ProvideFileSpec() ) | []( auto &&res ) {
              if ( !res ) {
                try {
                  std::rethrow_exception(res.error());
                }  catch ( const zypp::Exception &e ) {
                  std::cout << "Provide failed with " << e <<std::endl;
                }

              } else
                std::cout << "File provided to : " << res->file () <<std::endl;
              return res;
            });
          }
          return std::move(provs) | zyppng::waitFor<zyppng::expected<zyppng::ProvideRes>>();
        }

        std::cout << "Failed to attach media" << std::endl;
        return zyppng::makeReadyResult( std::vector<zyppng::expected<zyppng::ProvideRes>> { zyppng::expected<zyppng::ProvideRes>::error(res.error()) } );
      } | []( std::vector<zyppng::expected<zyppng::ProvideRes>> &&results ){
         for ( const auto &r : results ) {
           if ( !r )
             return zyppng::expected<void>::error( r.error() );
         }
         return zyppng::expected<void>::success();
    });
#endif

  const auto &makeDVDProv = [&]( int mediaNr, const std::string &fName ){
    return prov->attachMedia( zypp::Url("dvd:/"), zyppng::ProvideMediaSpec("CD Test Set")
                                               .setMediaFile("/tmp/provTest/mediacd")
                                               .setMedianr(mediaNr)
                                               .addCustomHeaderValue ("device", "/dev/sr1")
                                               .addCustomHeaderValue ("device", "/dev/sr2"))
      | [&]( zyppng::expected<zyppng::Provide::MediaHandle> &&res ){
          if ( res ) {
            std::cout << "Attached as: " << res->handle() << std::endl;
            return prov->provide ( *res, fName , zyppng::ProvideFileSpec() ) | [ attachId = *res, &prov ]( auto &&res ) {
              if ( !res ) {
                try {
                  std::rethrow_exception(res.error());
                }  catch ( const zypp::Exception &e ) {
                  std::cout << "Provide failed with " << e <<std::endl;
                }

              } else
                std::cout << "File provided to : " << res->file () <<std::endl;

              std::cout << "DETACHING " << attachId.handle() << std::endl;
              return res;
            };
          } else {
            std::cout << "Failed to attach media" << std::endl;
            return zyppng::makeReadyResult( zyppng::expected<zyppng::ProvideRes>::error(res.error()) );
          }
        } | [](  zyppng::expected<zyppng::ProvideRes> &&res ) {
        if ( !res ) {
          try {
            std::rethrow_exception(res.error());
          }  catch ( const zypp::Exception &e ) {
            std::cout << "Provide failed with " << e <<std::endl;
            return zyppng::expected<void>::error(res.error());
          } catch ( ... ) {
            std::cout << "Provide failed with exception " << std::endl;
            return zyppng::expected<void>::error(res.error());
          }
        } else {
          return zyppng::expected<void>::success();
        }
    };
  };

  ops.push_back( makeDVDProv(1, "/file1") );
  ops.push_back( makeDVDProv(1, "/file1") );
  ops.push_back( makeDVDProv(1, "/file1") );

  auto r = std::move(ops) | zyppng::waitFor<zyppng::expected<void>>();
  r->onReady( [&]( std::vector<zyppng::expected<void>> &&results ){
    for ( const auto &res : results ) {
      if ( res ) {
        std::cout << "Request finished successfull" << std::endl;
      } else {
        try {
          std::rethrow_exception (res.error());
        }  catch ( const zypp::Exception &e ) {
          std::cout << "Request failed with " << e <<std::endl;
        } catch ( ... ) {
          std::cout << "Request failed with exception " << std::endl;
        }
      }
    }
    std::cout << "All done "<< results.size() << " YAY" << std::endl;
    ev->quit();
  });

  ev->run ();
  return;
}
#endif
