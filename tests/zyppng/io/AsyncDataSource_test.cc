#include <boost/test/unit_test.hpp>
#include <zypp-core/zyppng/base/EventLoop>
#include <zypp-core/zyppng/base/Timer>
#include <zypp-core/zyppng/io/AsyncDataSource>
#include <thread>
#include <string_view>
#include <iostream>
#include <glib-unix.h>


BOOST_AUTO_TEST_CASE ( pipe_read_close )
{
  std::string_view text ("Hello from thread");
  int pipeFds[2] { -1, -1 };
  BOOST_REQUIRE( g_unix_open_pipe( pipeFds, FD_CLOEXEC, nullptr ) );

  auto loop = zyppng::EventLoop::create();
  auto dataSource = zyppng::AsyncDataSource::create();

  // make sure we are not stuck
  auto timer = zyppng::Timer::create();
  timer->start( 1000 );
  timer->connectFunc( &zyppng::Timer::sigExpired, [&]( auto & ){
    loop->quit();
  });

  bool gotClosed = false;
  zypp::ByteArray readData;

  BOOST_REQUIRE( dataSource->open( pipeFds[0] ) );
  BOOST_REQUIRE( dataSource->canRead() );
  BOOST_REQUIRE( dataSource->readFdOpen() );

  dataSource->connectFunc( &zyppng::AsyncDataSource::sigReadyRead, [&](){
    std::cout <<"Got read"<<std::endl;
    zypp::ByteArray d = dataSource->readAll();
    readData.insert( readData.end(), d.begin(), d.end()  );
  } );

  dataSource->connectFunc( &zyppng::AsyncDataSource::sigReadFdClosed, [&]( auto ){
    gotClosed = true;
    loop->quit();
  });

  std::thread writer( []( int writeFd, std::string_view text ){
    ::write( writeFd, text.data(), text.length() );
    ::close( writeFd );
  }, pipeFds[1], text );

  loop->run();
  writer.join();

  BOOST_REQUIRE ( !dataSource->readFdOpen() );

  ::close( pipeFds[0] );
  BOOST_REQUIRE_EQUAL( std::string_view( readData.data(), readData.size() ), text );
  BOOST_REQUIRE ( gotClosed );
}

BOOST_AUTO_TEST_CASE ( pipe_write_close )
{
  std::string_view text ("Hello from main");
  int pipeFds[2] { -1, -1 };
  BOOST_REQUIRE( g_unix_open_pipe( pipeFds, FD_CLOEXEC, nullptr ) );

  auto loop = zyppng::EventLoop::create();
  auto dataSink = zyppng::AsyncDataSource::create();

  // make sure we are not stuck
  auto timer = zyppng::Timer::create();
  timer->start( 3000 );
  timer->connectFunc( &zyppng::Timer::sigExpired, [&]( auto & ){
    loop->quit();
  });

  BOOST_REQUIRE( dataSink->open( -1, pipeFds[1] ) );
  BOOST_REQUIRE( dataSink->canWrite() );

  std::size_t bytesWritten = 0;
  dataSink->connectFunc( &zyppng::AsyncDataSource::sigBytesWritten, [&]( std::size_t bytes ){
    bytesWritten += bytes;
    if ( bytesWritten == text.size() )
      loop->quit();

  });

  zypp::ByteArray readData;
  std::thread reader( [ &readData ]( int readFd, std::string_view text ) {

    auto loop = zyppng::EventLoop::create();
    auto dataSource = zyppng::AsyncDataSource::create();

    if ( !dataSource->open( readFd ) )
      return;

    // make sure we are not stuck
    auto timer = zyppng::Timer::create();
    timer->start( 1000 );
    timer->connectFunc( &zyppng::Timer::sigExpired, [&]( auto & ){
      loop->quit();
    });

    dataSource->connectFunc( &zyppng::AsyncDataSource::sigReadyRead, [&](){
      zypp::ByteArray d = dataSource->readAll();
      readData.insert( readData.end(), d.begin(), d.end()  );
      if ( readData.size() == text.length() )
        loop->quit();
    });

    loop->run();

    std::cout << "Thread did read: " << readData.data() << std::endl;

    ::close( readFd );

  }, pipeFds[0], text );

  BOOST_REQUIRE_EQUAL( dataSink->write( text.data(), text.length() ), text.length() );

  loop->run();
  reader.join();

  ::close( pipeFds[1] );
  BOOST_REQUIRE_EQUAL( std::string_view( readData.data(), readData.size() ), text );
}

BOOST_AUTO_TEST_CASE ( pipe_close )
{
  std::string_view text ("Hello from main");
  int pipeFds[2] { -1, -1 };
  BOOST_REQUIRE( g_unix_open_pipe( pipeFds, FD_CLOEXEC, nullptr ) );

  auto loop = zyppng::EventLoop::create();
  auto dataSink = zyppng::AsyncDataSource::create();

  // make sure we are not stuck
  auto timer = zyppng::Timer::create();
  timer->start( 3000 );
  timer->connectFunc( &zyppng::Timer::sigExpired, [&]( auto & ){
    loop->quit();
  });

  bool gotClosed = false;

  BOOST_REQUIRE( dataSink->open( -1, pipeFds[1] ) );
  BOOST_REQUIRE( dataSink->canWrite() );

  std::size_t bytesWritten = 0;
  dataSink->connectFunc( &zyppng::AsyncDataSource::sigBytesWritten, [&]( std::size_t bytes ){
    bytesWritten += bytes;
  });

  dataSink->sigWriteFdClosed().connect([&]( auto ){
    gotClosed = true;
    loop->quit();
  });

  // we close the pipe before the loop even has a chance writing to it
  ::close( pipeFds[0] );

  BOOST_REQUIRE_EQUAL( dataSink->write( text.data(), text.length() ), text.length() );

  loop->run();
  ::close( pipeFds[1] );

  BOOST_REQUIRE ( gotClosed );
}

