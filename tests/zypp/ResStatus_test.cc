#include "TestSetup.h"
#include "zypp/ResStatus.h"

#define BOOST_TEST_MODULE ResStatus

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

////////////////////////////////////////////////////////////////////////////////
// tools
////////////////////////////////////////////////////////////////////////////////
template<class Tp>
inline const Tp & max( const Tp & lhs, const Tp & rhs )
{ return lhs < rhs ? rhs : lhs; }

template<class Tp, int N>
inline Tp * begin( Tp (& _array)[N] ) { return _array; }

template<class Tp, int N>
inline Tp * end( Tp (& _array)[N] ) { return _array + (sizeof(_array)/sizeof(Tp)); }

ResStatus::TransactByValue transactByValues[] = {
  ResStatus::USER, ResStatus::APPL_HIGH, ResStatus::APPL_LOW, ResStatus::SOLVER
};

ResStatus::TransactValue transactValues[] = {
  ResStatus::TRANSACT, ResStatus::KEEP_STATE, ResStatus::LOCKED
};

bool transactTo[] = {
  true, false
};

// Status transition like setTransact, setLock, setSoftTransact
typedef bool (ResStatus::* Transition)( bool, ResStatus::TransactByValue );

// Result evaluation
typedef void (* Evaluate)( ResStatus::TransactValue, ResStatus::TransactByValue, /* fromState, fromBy */
                           bool,                     ResStatus::TransactByValue, /* toState,   toBy */
                           bool,                     ResStatus );                /* done,      result */

// build status and return whether the comination is supported. (e.g currently no LOCKED state below APPL_HIGH)
inline bool initStatus( ResStatus::TransactValue fromState, ResStatus::TransactByValue fromBy, ResStatus & from )
{
  from = ResStatus();
  if ( fromState == ResStatus::KEEP_STATE )
  {
    from.setSoftLock( fromBy );
  }
  else
  {
    from.setTransactValue( fromState, fromBy );
    if ( fromState == ResStatus::LOCKED && ! from.isLocked() )
      return false; // no lock at this level (by now just USER APPL_HIGH)
  }
  return true;
}

void testTable( Transition transition, Evaluate evaluate )
{
  // Table: For each causer combination (fromBy -> toBy) invoke transition:
  //
  //    bool ok = ResStatus(fromState,fromBy).transition( toState, toBy )
  //
  // And evaluate the result.
  //
  for ( ResStatus::TransactByValue * toBy = begin( transactByValues ); toBy != end( transactByValues ); ++toBy )
  {
    for ( ResStatus::TransactByValue * fromBy = begin( transactByValues ); fromBy != end( transactByValues ); ++fromBy )
    {
      INT << "=== " << *fromBy << " ==> " << *toBy << " ===" << endl;
      for ( ResStatus::TransactValue * fromState = begin( transactValues ); fromState != end( transactValues ); ++fromState )
      {
        ResStatus from;
        if ( ! initStatus( *fromState, *fromBy, from ) )
        {
          //WAR << "Unsupported ResStatus(" << *fromState << "," << *fromBy << ")" << endl;
          continue; // Unsupported ResStatus
        }
        for ( bool * toState = begin( transactTo ); toState != end( transactTo ); ++toState )
        {
          ResStatus result( from );
          bool done = (result.*transition)( *toState, *toBy );
          if ( ! done )
            BOOST_CHECK_EQUAL( from, result ); // status stays unchaged on failure!
          evaluate( *fromState, *fromBy, *toState, *toBy, done, result );
        }
      }
    }
  }
}

// BOOST_CHECK_EQUAL or BOOST_REQUIRE_EQUAL
#define X BOOST_CHECK_EQUAL


// Transition must succeeds always
#define CHECK_DONE_ALWAYS       X( done, true ); if ( ! done ) return

// Transition succeeds if same or higher TransactByValue
#define CHECK_DONE_IFCAUSER     X( done, toBy >= fromBy ); if ( ! done ) return

// Transition succeeds if a locker (APPL_HIGH or USER)
#define CHECK_DONE_ALWAYS_IFLOCKER     X( done, toBy >= ResStatus::APPL_HIGH ); if ( ! done ) return

// Transition succeeds if a locker (APPL_HIGH or USER) and  same or higher TransactByValue
#define CHECK_DONE_IFCAUSER_ISLOCKER     X( done, toBy >= max(fromBy,ResStatus::APPL_HIGH) ); if ( ! done ) return


// Expected target state after transistion
#define CHECK_STATE(NEW)        X( result.getTransactValue(), ResStatus::NEW )


// Transition result: Remember the causer (i.e. may downgrade superior causer of previous state)
#define CHECK_CAUSER_SET        X( result.getTransactByValue(), toBy )

// Transition result: Remember a superior causer
#define CHECK_CAUSER_RAISED     X( result.getTransactByValue(), max(fromBy,toBy) )

// Transition result: Causer stays the same
#define CHECK_CAUSER_STAYS      X( result.getTransactByValue(), fromBy )

// Transition result: Causer reset to least (SOLVER) level.
#define CHECK_CAUSER_TO_SOLVER  X( result.getTransactByValue(), ResStatus::SOLVER )


////////////////////////////////////////////////////////////////////////////////
// test cases (see BOOST_AUTO_TEST_CASE(transition))
////////////////////////////////////////////////////////////////////////////////
// All tests below should define 3 checks, abbrev. by defines
//
//    CHECK_DONE_*:         When does the tranaction succeed? (return if not)
//    CHECK_STATE( NEXT ):  The state the transition leads to (if successfull)
//    CHECK_CAUSER_*:       Changes to the remembered causer (if successfull)
//

#define DOCHECK( FROMSTATE, TOSTATE, C_DONE, C_STATE, C_CAUSER ) \
	if ( ResStatus::FROMSTATE == fromState && TOSTATE == toState ) { C_DONE; CHECK_STATE( C_STATE ); C_CAUSER; }

void evaluateSetTransact( ResStatus::TransactValue fromState, ResStatus::TransactByValue fromBy,
                          bool                     toState,   ResStatus::TransactByValue toBy,
                          bool                     done,      ResStatus                  result )
{
  ResStatus from;
  initStatus( fromState, fromBy, from );
  MIL << from << " =setTransact("<<toState<<","<<toBy<<")=>\t" << done << ":" << result << endl;

  DOCHECK( TRANSACT,	true,	CHECK_DONE_ALWAYS,	TRANSACT,	CHECK_CAUSER_RAISED	);
  DOCHECK( TRANSACT,	false,	CHECK_DONE_IFCAUSER,	KEEP_STATE,	CHECK_CAUSER_RAISED	); // from transact into softlock
  DOCHECK( KEEP_STATE,	true,	CHECK_DONE_ALWAYS,	TRANSACT,	CHECK_CAUSER_SET	);
  DOCHECK( KEEP_STATE,	false,	CHECK_DONE_ALWAYS,	KEEP_STATE,	CHECK_CAUSER_STAYS	); // keep is not raised to softlock
  DOCHECK( LOCKED,	true,	CHECK_DONE_IFCAUSER,	TRANSACT,	CHECK_CAUSER_SET	);
  DOCHECK( LOCKED,	false,	CHECK_DONE_ALWAYS,	LOCKED,	 	CHECK_CAUSER_STAYS	);
}

void evaluateSetSoftTransact( ResStatus::TransactValue fromState, ResStatus::TransactByValue fromBy,
			      bool                     toState,   ResStatus::TransactByValue toBy,
			      bool                     done,      ResStatus                  result )
{
  ResStatus from;
  initStatus( fromState, fromBy, from );
  MIL << from << " =setSoftTransact("<<toState<<","<<toBy<<")=>\t" << done << ":" << result << endl;

  DOCHECK( TRANSACT,	true,	CHECK_DONE_ALWAYS,	TRANSACT,	CHECK_CAUSER_RAISED	);
  DOCHECK( TRANSACT,	false,	CHECK_DONE_IFCAUSER,	KEEP_STATE,	CHECK_CAUSER_RAISED	); // from transact into softlock
  DOCHECK( KEEP_STATE,	true,	CHECK_DONE_IFCAUSER,	TRANSACT,	CHECK_CAUSER_SET	); // leaving KEEP requires sup. causer
  DOCHECK( KEEP_STATE,	false,	CHECK_DONE_ALWAYS,	KEEP_STATE,	CHECK_CAUSER_STAYS	); // keep is not raised to softlock
  DOCHECK( LOCKED,	true,	CHECK_DONE_IFCAUSER,	TRANSACT,	CHECK_CAUSER_SET	);
  DOCHECK( LOCKED,	false,	CHECK_DONE_ALWAYS,	LOCKED,		CHECK_CAUSER_STAYS	);
}

// Check whether failures are ok and whether success lead to the correct state
void evaluateSetLock( ResStatus::TransactValue fromState, ResStatus::TransactByValue fromBy,
                      bool                     toState,   ResStatus::TransactByValue toBy,
                      bool                     done,      ResStatus                  result )
{
  ResStatus from;
  initStatus( fromState, fromBy, from );
  MIL << from << " =setLock("<<toState<<","<<toBy<<")=>\t" << done << ":" << result << endl;

  DOCHECK( TRANSACT,	true,	CHECK_DONE_IFCAUSER_ISLOCKER,	LOCKED,		CHECK_CAUSER_SET	); // transact is 'not locked'
  DOCHECK( TRANSACT,	false,	CHECK_DONE_ALWAYS,		TRANSACT,	CHECK_CAUSER_STAYS	);
  DOCHECK( KEEP_STATE,	true,	CHECK_DONE_ALWAYS_IFLOCKER,	LOCKED,		CHECK_CAUSER_SET	);
  DOCHECK( KEEP_STATE,	false,	CHECK_DONE_ALWAYS,		KEEP_STATE,	CHECK_CAUSER_STAYS	);
  DOCHECK( LOCKED,	true,	CHECK_DONE_ALWAYS,		LOCKED,		CHECK_CAUSER_RAISED	);
  DOCHECK( LOCKED,	false,	CHECK_DONE_IFCAUSER,		KEEP_STATE,	CHECK_CAUSER_TO_SOLVER	);
}

BOOST_AUTO_TEST_CASE(transition)
{
  //base::LogControl::TmpLineWriter shutUp( new log::FileLineWriter( "-" ) );
  MIL << endl;
  testTable( &ResStatus::setTransact,		&evaluateSetTransact );
  testTable( &ResStatus::setSoftTransact,	&evaluateSetSoftTransact );
  testTable( &ResStatus::setLock, 		&evaluateSetLock );
}


bool WhilePoolItemSameStateIsPrivate( ResStatus ostatus, ResStatus nstatus )
{
  if ( nstatus == ostatus )
    return true;
        // some bits changed...
  if ( nstatus.getTransactValue() != ostatus.getTransactValue()
       && ( ! nstatus.isBySolver() // ignore solver state changes
                  // removing a user lock also goes to bySolver
       || ostatus.getTransactValue() == ResStatus::LOCKED ) )
    return false;
  if ( nstatus.isLicenceConfirmed() != ostatus.isLicenceConfirmed() )
    return false;
  return true;
}

BOOST_AUTO_TEST_CASE(savestate)
{
  ResStatus ostatus;
  ResStatus nstatus;

  BOOST_CHECK_EQUAL( WhilePoolItemSameStateIsPrivate( ostatus, nstatus ), true );
  nstatus.setLock( true, ResStatus::USER );
  BOOST_CHECK_EQUAL( WhilePoolItemSameStateIsPrivate( ostatus, nstatus ), false );
  ostatus = nstatus;
  nstatus.setLock( false, ResStatus::USER );
  BOOST_CHECK_EQUAL( WhilePoolItemSameStateIsPrivate( ostatus, nstatus ), false );
}



