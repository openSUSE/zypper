
#include <iostream>
#include <fstream>
#include <list>
#include <string>

#include <zypp/base/Logger.h>
#include <zypp/base/Exception.h>

#include <boost/test/unit_test.hpp>

#include <zypp-core/ui/ProgressData>
#include <zypp/ZYppCallbacks.h>
using boost::unit_test::test_case;
using std::cout;
using std::endl;

using namespace zypp;

BOOST_AUTO_TEST_CASE(progressdata_test)
{
  {
    ProgressData progress(100);
    CombinedProgressData sub1rcv(progress, 80);

    ProgressData sub1progress(100);
    sub1progress.sendTo(sub1rcv);

    // task 1 goes to 50%
    sub1progress.set(50);
    // which is 50% of 80% in task 1
    BOOST_CHECK_EQUAL( progress.val(), 40 );
  }

  {
    ProgressData progress(40000);
    CombinedProgressData sub2rcv(progress, 10000);

    ProgressData sub2progress(500);
    sub2progress.sendTo(sub2rcv);
    sub2progress.set(250);

    // which is 50% of 80% in task 1
    BOOST_CHECK_EQUAL( progress.val(), 5000 );
  }

  {
    ProgressData progress(20000,60000);
    CombinedProgressData sub2rcv(progress, 10000);

    ProgressData sub2progress(500);
    sub2progress.sendTo(sub2rcv);
    sub2progress.set(250);

    // which is 50% of 80% in task 1
    BOOST_CHECK_EQUAL( progress.val(), 25000 );
  }

}

// Check expected ProgressReport triggers:
// none | (start progress)  progress*  (progress end)
struct PReceive : public callback::ReceiveReport<ProgressReport>
{
  int frame = 0;  // 0 -start-> 1(progress) -finish-> 2
  int pings = 0;  // count progress triggers

  void reportbegin() override
  {
    // cout << "REPORT+++" << endl;
    frame = pings = 0;
  }

  void start( const ProgressData & task ) override
  {
    // cout << "REPBEG " << task << " " << frame <<":"<< pings << endl;
    BOOST_CHECK_EQUAL( frame, 0 );
    BOOST_CHECK_EQUAL( pings, 0 );
    frame = 1;
    pings -= 1; // start must be followed by 1 progress
  }

  bool progress( const ProgressData & task ) override
  {
    // cout << "REP... " << task << " " << frame <<":"<< pings << endl;
    BOOST_CHECK_EQUAL( frame, 1 );  // progress
    pings += 1;
    return true;
  }

  void finish( const ProgressData & task ) override
  {
    // cout << "REPEND " << task << " " << frame <<":"<< pings << endl;
    BOOST_CHECK_EQUAL( frame, 1 );
    frame = 2;
    pings -= 1; // finish must be preceded by 1 progress
  }

  void reportend() override
  {
    // cout << "REPORT--- " << " " << frame <<":"<< pings << endl;
    BOOST_CHECK( ( frame == 0 && pings == 0 ) || ( frame == 2 && pings >= 0 ) );
  }
};

BOOST_AUTO_TEST_CASE(progress_report)
{
  PReceive preceive;
  preceive.connect();
  {
    callback::SendReport<ProgressReport> report;
    ProgressData ticks;
    ticks.sendTo( ProgressReportAdaptor( report ) );
    ticks.range( 5 );
  }
  BOOST_CHECK_EQUAL( preceive.frame, 0 ); // If no value was set no reports were sent.

  {
    callback::SendReport<ProgressReport> report;
    ProgressData ticks;
    ticks.sendTo( ProgressReportAdaptor( report ) );
    ticks.range( 5 );
    ticks.toMin();
    ticks.set(3);
    ticks.toMax();
  }
  BOOST_CHECK_EQUAL( preceive.frame, 2 ); // Values were set, so finish was sent.

}
