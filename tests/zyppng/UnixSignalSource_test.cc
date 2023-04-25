#include <boost/test/unit_test.hpp>
#include <zypp-core/zyppng/base/EventLoop>
#include <zypp-core/zyppng/base/EventDispatcher>
#include <zypp-core/zyppng/base/Timer>
#include <zypp-core/zyppng/base/UnixSignalSource>

#include <signal.h>

BOOST_AUTO_TEST_CASE(signalCatcher)
{
  {
    zyppng::EventLoopRef ev = zyppng::EventLoop::create();
    zyppng::TimerRef secTimer  = zyppng::Timer::create();
    secTimer->setSingleShot (true);
    secTimer->sigExpired ().connect ([&]( auto &){
      ev->quit ();
    });

    auto sigSource = ev->eventDispatcher ()->unixSignalSource ();

    int receivedSig = -1;
    sigSource->sigReceived ().connect ( [&]( int signum ) {
      receivedSig = signum;
      ev->quit ();
    });

    sigSource->addSignal ( SIGALRM );
    sigSource->addSignal ( SIGINT );

    // send alarm to our process
    alarm(1);
    secTimer->start ( 5000 );
    ev->run();

    BOOST_CHECK_EQUAL( receivedSig, SIGALRM );

    sigSource->removeSignal ( SIGALRM );

    sigset_t siset;
    ::sigemptyset ( &siset );
    ::pthread_sigmask ( SIG_SETMASK, nullptr, &siset );

    BOOST_CHECK_EQUAL( ::sigismember( &siset, SIGALRM ), 0 );
    BOOST_CHECK_EQUAL( ::sigismember( &siset, SIGINT ), 1 );
  }

  //after cleaning the event loop the sigmask should be normal again
  sigset_t siset;
  ::sigemptyset ( &siset );
  ::pthread_sigmask ( SIG_SETMASK, nullptr, &siset );

  BOOST_CHECK_EQUAL( ::sigismember( &siset, SIGINT ), 0 );
}


BOOST_AUTO_TEST_CASE(signalCleanup)
{

  {
    zyppng::EventLoopRef ev = zyppng::EventLoop::create();
    auto sigSource = ev->eventDispatcher ()->unixSignalSource ();

    {
      sigset_t siset;
      ::sigemptyset ( &siset );
      ::pthread_sigmask ( SIG_SETMASK, nullptr, &siset );

      BOOST_CHECK_EQUAL( ::sigismember( &siset, SIGALRM ), 0 );
      BOOST_CHECK_EQUAL( ::sigismember( &siset, SIGINT ), 0 );
    }

    sigSource->addSignal ( SIGINT );
    sigSource->addSignal ( SIGINT );
    sigSource->addSignal ( SIGINT );

    {
      sigset_t siset;
      ::sigemptyset ( &siset );
      ::pthread_sigmask ( SIG_SETMASK, nullptr, &siset );

      BOOST_CHECK_EQUAL( ::sigismember( &siset, SIGALRM ), 0 );
      BOOST_CHECK_EQUAL( ::sigismember( &siset, SIGINT ), 1 );
    }


    sigSource->removeSignal ( SIGINT );
    {
      sigset_t siset;
      ::sigemptyset ( &siset );
      ::pthread_sigmask ( SIG_SETMASK, nullptr, &siset );

      BOOST_CHECK_EQUAL( ::sigismember( &siset, SIGALRM ), 0 );
      BOOST_CHECK_EQUAL( ::sigismember( &siset, SIGINT ), 1 );
    }

    sigSource->removeSignal ( SIGINT );
    {
      sigset_t siset;
      ::sigemptyset ( &siset );
      ::pthread_sigmask ( SIG_SETMASK, nullptr, &siset );

      BOOST_CHECK_EQUAL( ::sigismember( &siset, SIGALRM ), 0 );
      BOOST_CHECK_EQUAL( ::sigismember( &siset, SIGINT ), 1 );
    }

    sigSource->removeSignal ( SIGINT );
    {
      sigset_t siset;
      ::sigemptyset ( &siset );
      ::pthread_sigmask ( SIG_SETMASK, nullptr, &siset );

      BOOST_CHECK_EQUAL( ::sigismember( &siset, SIGALRM ), 0 );
      BOOST_CHECK_EQUAL( ::sigismember( &siset, SIGINT ), 0 );
    }

    sigSource->addSignal ( SIGALRM );
    sigSource->addSignal ( SIGALRM );
    sigSource->addSignal ( SIGINT );
    sigSource->addSignal ( SIGINT );

    {
      sigset_t siset;
      ::sigemptyset ( &siset );
      ::pthread_sigmask ( SIG_SETMASK, nullptr, &siset );

      BOOST_CHECK_EQUAL( ::sigismember( &siset, SIGALRM ), 1 );
      BOOST_CHECK_EQUAL( ::sigismember( &siset, SIGINT ), 1 );
    }

  }

  {
    sigset_t siset;
    ::sigemptyset ( &siset );
    ::pthread_sigmask ( SIG_SETMASK, nullptr, &siset );

    BOOST_CHECK_EQUAL( ::sigismember( &siset, SIGALRM ), 0 );
    BOOST_CHECK_EQUAL( ::sigismember( &siset, SIGINT ), 0 );
  }

}
