#include <boost/test/unit_test.hpp>
#include <zypp/zyppng/base/EventLoop>
#include <zypp/zyppng/base/EventDispatcher>
#include <zypp/zyppng/base/Timer>
#include <zypp/base/Exception.h>

#include <iostream>

BOOST_AUTO_TEST_CASE(eventloop)
{
  zyppng::EventLoop::Ptr loop = zyppng::EventLoop::create();

  //we should hit that timer first
  zyppng::Timer::Ptr t1 = zyppng::Timer::create();

  //second hit
  zyppng::Timer::Ptr t2 = zyppng::Timer::create();

  //this in theory would be the 2nd hit but is cleaned up by t1 before that
  zyppng::Timer::Ptr t3 = zyppng::Timer::create();

  //this is a singleshot timer, it should just hit once
  zyppng::Timer::Ptr t4 = zyppng::Timer::create();

  int hitT1 = 0;
  int hitT2 = 0;
  int hitT4 = 0;
  int executedIdle = 0;
  int executedIdleOnce = 0;

  t1->sigExpired().connect( [ &hitT1, &t3 ]( zyppng::Timer & ) {
    hitT1++;
    t3.reset();
  });

  t2->sigExpired().connect( [ &hitT1, &hitT2, &loop ]( zyppng::Timer & ){
    BOOST_REQUIRE_GT( hitT1, 0 );
    hitT2++;
    if ( hitT2 >= 3 )
      loop->quit();
  });

  t3->sigExpired().connect( [ ]( zyppng::Timer & ){
    BOOST_TEST( false );
  });

  t4->setSingleShot( true );
  t4->sigExpired().connect( [ &hitT4 ]( zyppng::Timer & ){
    hitT4++;

    //timer deviation should not be too big, can only be tested on a singleShot timer
    // ma: disabled as OBS builder have quite big deviation >15
    // BOOST_REQUIRE_LE( t.now() - t.expires(), 3 );
  });

  //convenience function to execute a function later
  zyppng::EventDispatcher::invokeOnIdle( [ &executedIdle](){ executedIdle++; return ( executedIdle < 2 );} );

  zyppng::EventDispatcher::invokeOnIdle( [ &executedIdleOnce ](){ executedIdleOnce++; return false;} );

  t1->start( 10 );
  t3->start( 13 );
  t2->start( 15 );
  t4->start( 2 );

  loop->run();

  BOOST_REQUIRE_EQUAL( executedIdle, 2 );
  BOOST_REQUIRE_EQUAL( executedIdleOnce, 1 );
  BOOST_REQUIRE_GE ( hitT1, 3 );
  BOOST_REQUIRE_EQUAL( hitT2, 3 );
  BOOST_REQUIRE_EQUAL( hitT4, 1 );

  t1->stop();
  t2->stop();

  BOOST_REQUIRE_EQUAL( loop->eventDispatcher()->runningTimers(), 0 );
}

BOOST_AUTO_TEST_CASE(createTimerWithoutEV)
{
  BOOST_CHECK_THROW( zyppng::Timer::create(), zypp::Exception);
}

BOOST_AUTO_TEST_CASE(checkcleanup)
{
  zyppng::EventLoop::Ptr loop = zyppng::EventLoop::create();
  BOOST_REQUIRE_EQUAL( loop->eventDispatcher().get(), zyppng::EventDispatcher::instance().get() );

  //explicit cleanup
  loop.reset();
  BOOST_REQUIRE_EQUAL( static_cast<zyppng::EventDispatcher *>(nullptr), zyppng::EventDispatcher::instance().get() );

  //implicit cleanup
  {
    zyppng::EventLoop::create();
    BOOST_REQUIRE_EQUAL( static_cast<zyppng::EventDispatcher *>(nullptr), zyppng::EventDispatcher::instance().get() );
  }
}
