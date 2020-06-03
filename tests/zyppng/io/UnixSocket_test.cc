#include <boost/test/unit_test.hpp>
#include <zypp/zyppng/base/EventLoop>
#include <zypp/zyppng/base/Timer>
#include <zypp/zyppng/io/Socket>
#include <zypp/zyppng/io/SockAddr>
#include <thread>
#include <string_view>
#include <iostream>

BOOST_AUTO_TEST_CASE ( lorem )
{

  const std::string payloadT =
    "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, "
    "sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem "
    "ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore "
    "magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata "
    "sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut "
    "labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, "
    "no sea takimata sanctus est Lorem ipsum dolor sit amet.";

  // make sure we get over the OS buffers
  std::string payload;
  for ( int i = 0; i < 700; i++ ) {
    payload += payloadT;
  }

  auto ev = zyppng::EventLoop::create();
  auto listeningSock = zyppng::Socket::create( AF_UNIX, SOCK_STREAM, 0 );
  auto error = zyppng::Socket::NoError;

  size_t bytesWritten = 0;
  zyppng::ByteArray dataReceived; //< This is where the thread will write the data

  auto addr = std::make_shared<zyppng::UnixSockAddr>( "socktest", true );
  BOOST_REQUIRE( listeningSock->bind( addr ) );
  BOOST_REQUIRE( listeningSock->listen() );
  BOOST_REQUIRE_EQUAL( listeningSock->state(), zyppng::Socket::ListeningState );

  zyppng::Socket::Ptr connection;

  listeningSock->sigIncomingConnection().connect( [&](){
    connection = listeningSock->accept();
    BOOST_REQUIRE_EQUAL( connection->state(), zyppng::Socket::ConnectedState );

    connection->sigError().connect([&]( const auto err ){
      if ( err == zyppng::Socket::ConnectionDelayed )
        return;
      error = err;
      ev->quit();
    });

    connection->sigDisconnected().connect( [&](){
      BOOST_REQUIRE_EQUAL( connection->state(), zyppng::Socket::ClosedState );
      ev->quit();
    });

    connection->sigBytesWritten().connect( [&]( const auto bytes ){
      bytesWritten += bytes;
    });

    connection->write( payload.data(), payload.size() );
  });

  std::thread clientThread( [ payload, &dataReceived ](){
    auto ev   = zyppng::EventLoop::create();
    auto sock = zyppng::Socket::create( AF_UNIX, SOCK_STREAM, 0 );
    auto timer = zyppng::Timer::create();

    bool timedOut = false;
    bool gotConnected = false;
    bool gotDisconnected = false;
    auto error = zyppng::Socket::NoError;

    sock->sigError().connect( [&]( const zyppng::Socket::SocketError err ){
      if ( err == zyppng::Socket::ConnectionDelayed )
        return;
      error = err;
      ev->quit();
    });

    sock->sigConnected().connect( [&](){
      BOOST_REQUIRE_EQUAL( sock->state(), zyppng::Socket::ConnectedState );
      gotConnected = true;
    });

    sock->sigDisconnected().connect( [&](){
      gotDisconnected = true;
      BOOST_REQUIRE_EQUAL( sock->state(), zyppng::Socket::ClosedState );
      ev->quit();
    });

    sock->sigReadyRead().connect([&](){
      const auto bA = sock->bytesAvailable();
      auto all = sock->readAll();

      std::cout << "Received" << std::endl;

      dataReceived.insert( dataReceived.end(), all.begin(), all.end() );
      BOOST_REQUIRE_GE( all.size(), bA );

      if ( dataReceived.size() == payload.length() ) {
        std::string_view rec( dataReceived.data(), dataReceived.size() );
        BOOST_REQUIRE_EQUAL( rec, payload );
        sock->disconnect();
      }
    });

    timer->sigExpired().connect([&]( zyppng::Timer &) {
      timedOut = true;
      ev->quit();
    });

    timer->start( 30000 );

    sock->connect( std::make_shared<zyppng::UnixSockAddr>( "socktest", true ) );
    BOOST_REQUIRE( sock->state() == zyppng::Socket::ConnectedState || sock->state() == zyppng::Socket::ConnectingState );


    ev->run();
    BOOST_REQUIRE_EQUAL( error, zyppng::Socket::NoError );
    BOOST_REQUIRE( gotConnected );
    BOOST_REQUIRE( gotDisconnected );
    BOOST_REQUIRE_EQUAL( timedOut, false );
  });

  ev->run();
  clientThread.join();

  BOOST_REQUIRE_EQUAL( payload , std::string_view( dataReceived.data(), dataReceived.size() ) );
  BOOST_REQUIRE_EQUAL( bytesWritten, payload.size() );

}

BOOST_AUTO_TEST_CASE ( echo )
{

  const std::string payload = "Hello World";

  auto ev = zyppng::EventLoop::create();
  auto listeningSock = zyppng::Socket::create( AF_UNIX, SOCK_STREAM, 0 );
  auto error = zyppng::Socket::NoError;

  auto addr = std::make_shared<zyppng::UnixSockAddr>( "socktest", true );
  BOOST_REQUIRE( listeningSock->bind( addr ) );
  BOOST_REQUIRE( listeningSock->listen() );
  BOOST_REQUIRE_EQUAL( listeningSock->state(), zyppng::Socket::ListeningState );

  zyppng::Socket::Ptr connection;
  zyppng::ByteArray received;

  listeningSock->sigIncomingConnection().connect( [&](){
    connection = listeningSock->accept();
    BOOST_REQUIRE_EQUAL( connection->state(), zyppng::Socket::ConnectedState );

    connection->sigError().connect([&]( const auto err ){
      if ( err == zyppng::Socket::ConnectionDelayed )
        return;
      error = err;
      ev->quit();
    });

    connection->sigDisconnected().connect( [&](){
      BOOST_REQUIRE_EQUAL( connection->state(), zyppng::Socket::ClosedState );
      ev->quit();
    });

    // just echo received data back right away
    connection->sigReadyRead().connect( [&](){
      connection->write( connection->readAll() );
    });
  });

  std::thread clientThread( [ payload ](){
    auto ev   = zyppng::EventLoop::create();
    auto sock = zyppng::Socket::create( AF_UNIX, SOCK_STREAM, 0 );
    auto timer = zyppng::Timer::create();

    bool timedOut = false;
    bool gotConnected = false;
    bool gotDisconnected = false;
    auto error = zyppng::Socket::NoError;

    sock->sigError().connect( [&]( const zyppng::Socket::SocketError err ){
      if ( err == zyppng::Socket::ConnectionDelayed )
        return;
      error = err;
      ev->quit();
    });

    sock->sigConnected().connect( [&](){
      BOOST_REQUIRE_EQUAL( sock->state(), zyppng::Socket::ConnectedState );
      gotConnected = true;


      sock->write( payload.data(), payload.size() );
    });

    sock->sigDisconnected().connect( [&](){
      gotDisconnected = true;
      BOOST_REQUIRE_EQUAL( sock->state(), zyppng::Socket::ClosedState );
      ev->quit();
    });

    zyppng::ByteArray dataReceived;
    sock->sigReadyRead().connect([&](){
      const auto bA = sock->bytesAvailable();
      auto all = sock->readAll();

      dataReceived.insert( dataReceived.end(), all.begin(), all.end() );
      BOOST_REQUIRE_EQUAL( bA, all.size() );

      if ( dataReceived.size() == payload.length() ) {
        std::string_view rec( dataReceived.data(), dataReceived.size() );
        BOOST_REQUIRE_EQUAL( rec, payload );
        sock->disconnect();
      }
    });

    timer->sigExpired().connect([&]( zyppng::Timer &) {
      timedOut = true;
      ev->quit();
    });

    timer->start( 30000 );

    sock->connect( std::make_shared<zyppng::UnixSockAddr>( "socktest", true ) );
    BOOST_REQUIRE( sock->state() == zyppng::Socket::ConnectedState || sock->state() == zyppng::Socket::ConnectingState );


    ev->run();
    BOOST_REQUIRE_EQUAL( error, zyppng::Socket::NoError );
    BOOST_REQUIRE( gotConnected );
    BOOST_REQUIRE( gotDisconnected );
    BOOST_REQUIRE_EQUAL( timedOut, false );
  });

  ev->run();
  clientThread.join();
}
