#include "TestSetup.h"
#include "zypp/ResStatus.h"

#define BOOST_TEST_MODULE ResStatus

template<class _Tp, int N>
inline _Tp * begin( _Tp (& _array)[N] ) { return _array; }

template<class _Tp, int N>
inline _Tp * end( _Tp (& _array)[N] ) { return _array + (sizeof(_array)/sizeof(_Tp)); }

ResStatus::TransactByValue transactByValues[] = {
  ResStatus::USER, ResStatus::APPL_HIGH, ResStatus::APPL_LOW, ResStatus::SOLVER
};

ResStatus::TransactValue transactValues[] = {
  ResStatus::TRANSACT, ResStatus::KEEP_STATE, ResStatus::LOCKED
};

bool transactTo[] = { true, false };

BOOST_AUTO_TEST_CASE(Default)
{
  {
    ResStatus s;
    BOOST_CHECK( s.isUninstalled() );
    BOOST_CHECK_EQUAL( s.isInstalled(), ! s.isUninstalled() );
    BOOST_CHECK_EQUAL( s.getTransactValue(),   ResStatus::KEEP_STATE );
    BOOST_CHECK_EQUAL( s.getTransactByValue(), ResStatus::SOLVER );
  }
  {
    ResStatus s( true );
    BOOST_CHECK( s.isInstalled() );
    BOOST_CHECK_EQUAL( s.isInstalled(), ! s.isUninstalled() );
    BOOST_CHECK_EQUAL( s.getTransactValue(),   ResStatus::KEEP_STATE );
    BOOST_CHECK_EQUAL( s.getTransactByValue(), ResStatus::SOLVER );
  }
}


// Status transition like setTransact, setLock, setSoftTransact
typedef bool (ResStatus::* Transition)( bool, ResStatus::TransactByValue );

// Result evaluation
typedef void (* Evaluate)( ResStatus::TransactValue, ResStatus::TransactByValue, /* fromState, fromBy */
                           bool,                     ResStatus::TransactByValue, /* toState,   toBy */
                           bool,                     ResStatus );                /* done,      result */

void testTable( Transition transition, Evaluate evaluate )
{
  // Table: For each causer combination (fromBy -> toBy) invoke transition:
  //
  //    bool ok = ResStatus(fromState,fromBy).transition( toState, toBy )
  //
  // And evaluate the result.
  //
  for ( ResStatus::TransactByValue * fromBy = begin( transactByValues ); fromBy != end( transactByValues ); ++fromBy )
  {
    for ( ResStatus::TransactByValue * toBy = begin( transactByValues ); toBy != end( transactByValues ); ++toBy )
    {
      for ( ResStatus::TransactValue * fromState = begin( transactValues ); fromState != end( transactValues ); ++fromState )
      {
        for ( bool * toState = begin( transactTo ); toState != end( transactTo ); ++toState )
        {
          ResStatus from;
          from.setTransactValue( *fromState, *fromBy ); // NEEDS FIX!

          ResStatus result( from );
          bool done = (result.*transition)( *toState, *toBy );

          evaluate( *fromState, *fromBy, *toState, *toBy, done, result );
        }
      }
    }
  }
}

// Transitions succeeds always:
#define CHECK_DONE_ALWAYS       BOOST_CHECK_EQUAL( done, true )

// Transitions succeeds if same or higher TransactByValue:
#define CHECK_DONE_IFCAUSER     BOOST_CHECK_EQUAL( done, *toBy >= *fromBy )

void evaluateSetTransact( ResStatus::TransactValue fromState, ResStatus::TransactByValue fromBy,
                          bool                     toState,   ResStatus::TransactByValue toBy,
                          bool                     done,      ResStatus                  result )
{
  ResStatus from;
  from.setTransactValue( fromState, fromBy );
  MIL << from << " =setTransact("<<toState<<","<<toBy<<")=>\t" << done << ":" << result << endl;

  switch ( fromState )
  {
    case ResStatus::TRANSACT:
      if ( toState )
      {
      }
      else
      {
      }
      break;
    case ResStatus::KEEP_STATE:
      if ( toState )
      {
      }
      else
      {
      }
      break;
    case ResStatus::LOCKED:
      if ( toState )
      {
      }
      else
      {
      }
      break;
  }
}

#if 0
BOOST_AUTO_TEST_CASE(transition)
{
  base::LogControl::TmpLineWriter shutUp( new log::FileLineWriter( "/home/ma/zypp/BUILD/libzypp/devel/devel.ma/LOGFILE" ) );
  MIL << endl;
  testTable( &ResStatus::setTransact, &evaluateSetTransact );
}
#endif
