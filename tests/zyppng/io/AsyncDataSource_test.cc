#include <boost/test/unit_test.hpp>
#include <zypp-core/zyppng/base/EventLoop>
#include <zypp-core/zyppng/base/EventDispatcher>
#include <zypp-core/zyppng/base/Timer>
#include <zypp-core/zyppng/io/AsyncDataSource>
#include <thread>
#include <string_view>
#include <iostream>
#include <glib-unix.h>
#include <fstream>

#include <mutex>
#include <condition_variable>


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

  BOOST_REQUIRE( dataSource->openFds( { pipeFds[0] } ) );
  BOOST_REQUIRE( dataSource->canRead() );
  BOOST_REQUIRE( dataSource->readFdOpen() );

  const auto &readAllData = [&](){
    zypp::ByteArray d = dataSource->readAll();
    readData.insert( readData.end(), d.begin(), d.end()  );
  };

  dataSource->connectFunc( &zyppng::AsyncDataSource::sigReadyRead, readAllData );
  dataSource->connectFunc( &zyppng::AsyncDataSource::sigReadFdClosed, [&]( auto, auto ){
    gotClosed = true;
    readAllData();
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

// basically the same as above, but we break the receiving socket by closing the read end
// during a readAll call.
BOOST_AUTO_TEST_CASE ( pipe_read_close2 )
{
  std::string_view text ("Hello again");
  int pipeFds[2] { -1, -1 };
  BOOST_REQUIRE( g_unix_open_pipe( pipeFds, FD_CLOEXEC, nullptr ) );

  std::mutex lock;
  std::condition_variable cv;
  bool written = false;

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

  BOOST_REQUIRE( dataSource->openFds( { pipeFds[0] } ) );
  BOOST_REQUIRE( dataSource->canRead() );

  const auto &readAllData = [&](){
    zypp::ByteArray d = dataSource->readAll();
    readData.insert( readData.end(), d.begin(), d.end()  );
  };

  dataSource->connectFunc( &zyppng::AsyncDataSource::sigReadyRead, [&]() {
    std::unique_lock l(lock);
    if ( !written ){
      cv.wait(l, [&](){ return written; });
    }

    // close the pipe here, breaking the receiving socket
    ::close( pipeFds[1] );
    readAllData();
  } );
  dataSource->connectFunc( &zyppng::AsyncDataSource::sigReadFdClosed, [&]( auto, auto ){
    gotClosed = true;
    readAllData();
    loop->quit();

  });

  std::thread writer( [&lock, &written, &cv]( int writeFd, std::string_view text ){
    {
      std::unique_lock l(lock);
      ::write( writeFd, text.data(), text.length() );
      written = true;
    }
    cv.notify_all();
  }, pipeFds[1], text );

  loop->run();
  writer.join();

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

  BOOST_REQUIRE( dataSink->openFds( {}, pipeFds[1] ) );
  BOOST_REQUIRE( dataSink->canWrite() );

  int64_t bytesWritten = 0;
  dataSink->connectFunc( &zyppng::AsyncDataSource::sigBytesWritten, [&]( int64_t bytes ){
    bytesWritten += bytes;
    if ( std::size_t(bytesWritten) == text.size() )
      loop->quit();

  });

  zypp::ByteArray readData;
  std::thread reader( [ &readData ]( int readFd, std::string_view text ) {

    auto loop = zyppng::EventLoop::create();
    auto dataSource = zyppng::AsyncDataSource::create();

    if ( !dataSource->openFds( { readFd } ) )
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
  auto dataSink  = zyppng::AsyncDataSource::create();

  // make sure we are not stuck
  auto timer = zyppng::Timer::create();
  timer->start( 3000 );
  timer->connectFunc( &zyppng::Timer::sigExpired, [&]( auto & ){
    loop->quit();
  });

  bool gotClosed = false;

  BOOST_REQUIRE( dataSink->openFds( {}, pipeFds[1] ) );
  BOOST_REQUIRE( dataSink->canWrite() );

  int64_t bytesWritten = 0;
  dataSink->connectFunc( &zyppng::AsyncDataSource::sigBytesWritten, [&]( int64_t bytes ){
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

BOOST_AUTO_TEST_CASE ( multichannel )
{
  std::string_view text[] = { "Hello to channel1", "Hello to channel2 with more bytes" };
  int pipeFds[2] { -1, -1 };
  BOOST_REQUIRE( g_unix_open_pipe( pipeFds, FD_CLOEXEC, nullptr ) );

  int pipe2Fds[2] { -1, -1 };
  BOOST_REQUIRE( g_unix_open_pipe( pipe2Fds, FD_CLOEXEC, nullptr ) );

  auto loop = zyppng::EventLoop::create();
  auto dataSink = zyppng::AsyncDataSource::create();
  auto dataSink2 = zyppng::AsyncDataSource::create();

  // make sure we are not stuck
  auto timer = zyppng::Timer::create();
  timer->start( 10000 );

  bool timeout = false;
  timer->connectFunc( &zyppng::Timer::sigExpired, [&]( auto & ){
    timeout = true;
    loop->quit();
  });

  BOOST_REQUIRE( dataSink->openFds( {}, pipeFds[1] ) );
  BOOST_REQUIRE( dataSink->canWrite() );

  BOOST_REQUIRE( dataSink2->openFds( {}, pipe2Fds[1] ) );
  BOOST_REQUIRE( dataSink2->canWrite() );

  int64_t bytesWritten = 0;
  dataSink->connectFunc( &zyppng::AsyncDataSource::sigBytesWritten, [&]( int64_t bytes ){
    bytesWritten += bytes;
  });

  int64_t bytesWritten2 = 0;
  dataSink2->connectFunc( &zyppng::AsyncDataSource::sigBytesWritten, [&]( int64_t bytes ){
    bytesWritten2 += bytes;
  });

  bool dataSink1ABW = false;
  dataSink->connectFunc( &zyppng::AsyncDataSource::sigAllBytesWritten, [&]( ){
    dataSink1ABW = true;
  });

  bool dataSink2ABW = false;
  dataSink2->connectFunc( &zyppng::AsyncDataSource::sigAllBytesWritten, [&]( ){
    dataSink2ABW = true;
  });

  auto dataReceiver = zyppng::AsyncDataSource::create();
  BOOST_REQUIRE( dataReceiver->openFds( { pipeFds[0], pipe2Fds[0] }, -1 ) );
  BOOST_REQUIRE( dataReceiver->canRead() );
  BOOST_REQUIRE_EQUAL( dataReceiver->readChannelCount(), 2 );
  BOOST_REQUIRE_EQUAL( dataReceiver->currentReadChannel(), 0 );

  zypp::ByteArray dataRecv[2];

  dataReceiver->connectFunc( &zyppng::AsyncDataSource::sigChannelReadyRead, [&]( auto channel ) {
    dataReceiver->setReadChannel( channel );
    BOOST_REQUIRE_EQUAL( dataReceiver->currentReadChannel(), channel );
    zypp::ByteArray d = dataReceiver->readAll();
    dataRecv[channel].insert( dataRecv[channel].end(), d.begin(), d.end()  );
    if ( dataRecv[0].size() > 0 && dataRecv[0].size() == text[0].size()
      && dataRecv[1].size() > 0 && dataRecv[1].size() == text[1].size() )
      loop->quit();
  });

  bool bytesPushed = false;
  loop->eventDispatcher()->invokeOnIdle( [ & ](){
    dataSink->write( text[0].data(), text[0].size() );
    dataSink2->write( text[1].data(), text[1].size() );
    bytesPushed = true;
    return false;
  });

  loop->run();

  BOOST_REQUIRE( !timeout );
  BOOST_REQUIRE( dataSink1ABW );
  BOOST_REQUIRE( dataSink2ABW );
  BOOST_REQUIRE_EQUAL( bytesWritten, text[0].size() );
  BOOST_REQUIRE_EQUAL( bytesWritten2, text[1].size() );
  BOOST_REQUIRE_EQUAL( text[0], std::string_view( dataRecv[0].data(), dataRecv[0].size() ) );
  BOOST_REQUIRE_EQUAL( text[1], std::string_view( dataRecv[1].data(), dataRecv[1].size() ) );
}

BOOST_AUTO_TEST_CASE ( readl )
{
  std::string_view text[] = { "Hello\n", "World\n", "123456" };
  int pipeFds[2] { -1, -1 };
  BOOST_REQUIRE( g_unix_open_pipe( pipeFds, FD_CLOEXEC, nullptr ) );

  auto loop = zyppng::EventLoop::create();
  auto dataSource = zyppng::AsyncDataSource::create();

  // make sure we are not stuck
  auto timer = zyppng::Timer::create();
  timer->start( 3000 );

  bool timeout = false;
  timer->connectFunc( &zyppng::Timer::sigExpired, [&]( auto & ){
    loop->quit();
    timeout = true;
  });

  BOOST_REQUIRE( dataSource->openFds( { pipeFds[0] } ) );
  BOOST_REQUIRE( dataSource->canRead() );

  dataSource->setReadChannel( 0 );
  dataSource->sigReadyRead ().connect([&](){
    // now write more data without returning to the ev loop, and check if we get it all
    ::write( pipeFds[1], text[1].data(), text[1].size() );
    ::write( pipeFds[1], text[2].data(), text[2].size() );

    auto ba = dataSource->readLine();
    BOOST_REQUIRE_EQUAL( text[0], std::string_view( ba.data(), ba.size() ) );
    ba = dataSource->readLine( text[1].size () + 1 ); // trigger code path with fixed size
    BOOST_REQUIRE_EQUAL( text[1], std::string_view( ba.data(), ba.size() ) );
    ba = dataSource->readLine();
    BOOST_REQUIRE_EQUAL( text[2], std::string_view( ba.data(), ba.size() ) );
    loop->quit();
  });

  //write the first line
  ::write( pipeFds[1], text[0].data(), text[0].size() );

  loop->run();

  BOOST_REQUIRE( !timeout );

}
