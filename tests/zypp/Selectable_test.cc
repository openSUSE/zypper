#include "TestSetup.h"
#include "zypp/ResPool.h"
#include "zypp/ui/Selectable.h"

#define BOOST_TEST_MODULE Selectable

/////////////////////////////////////////////////////////////////////////////

static TestSetup test;

BOOST_AUTO_TEST_CASE(testcase_init)
{
//   zypp::base::LogControl::instance().logToStdErr();
  test.loadTestcaseRepos( TESTS_SRC_DIR"/data/TCSelectable" );

//   dumpRange( USR, test.pool().knownRepositoriesBegin(),
//                   test.pool().knownRepositoriesEnd() ) << endl;
//   USR << "pool: " << test.pool() << endl;
}
/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(candiadate)
{
  ResPoolProxy poolProxy( test.poolProxy() );
  ui::Selectable::Ptr s( poolProxy.lookup( ResKind::package, "candidate" ) );
  //   (I 1) {
  //   I__s_(8)candidate-1-1.i586(@System)(openSUSE)
  // } (A 6) {
  //   U__s_(2)candidate-4-1.x86_64(RepoHIGH)(unkown)
  //   U__s_(3)candidate-4-1.i586(RepoHIGH)(unkown) <- (update) candidate if allowVendorChange
  //   U__s_(6)candidate-0-1.x86_64(RepoMID)(SUSE)
  //   U__s_(7)candidate-0-1.i586(RepoMID)(SUSE) <- candidate (highest prio matching arch and vendor)
  //   U__s_(4)candidate-2-1.x86_64(RepoLOW)(openSUSE)
  //   U__s_(5)candidate-2-1.i586(RepoLOW)(openSUSE)
  // }
  if ( ZConfig::instance().solver_allowVendorChange() )
  {
    BOOST_CHECK_EQUAL( s->candidateObj()->repoInfo().alias(), "RepoHIGH" );
    BOOST_CHECK_EQUAL( s->candidateObj()->edition(), Edition("4-1") );
    BOOST_CHECK_EQUAL( s->candidateObj()->arch(), Arch_i586 );
    // updateCandidate:
    BOOST_CHECK_EQUAL( s->updateCandidateObj(), s->candidateObj() );
  }
  else
  {
    BOOST_CHECK_EQUAL( s->candidateObj()->repoInfo().alias(), "RepoMID" );
    BOOST_CHECK_EQUAL( s->candidateObj()->edition(), Edition("0-1") );
    BOOST_CHECK_EQUAL( s->candidateObj()->arch(), Arch_i586 );
    // no updateCandidate due to low version
    BOOST_CHECK_EQUAL( s->updateCandidateObj(), PoolItem() );
  }
}

BOOST_AUTO_TEST_CASE(candiadatenoarch)
{
  ResPoolProxy poolProxy( test.poolProxy() );
  ui::Selectable::Ptr s( poolProxy.lookup( ResKind::package, "candidatenoarch" ) );
/*[package]candidatenoarch: S_KeepInstalled
   (I 1) {
   I__s_(17)candidatenoarch-1-1.i586(@System)
}  (A 8) {
 C U__s_(4)candidatenoarch-5-1.noarch(RepoHIGH) <- candidate (arch/noarch change)
   U__s_(5)candidatenoarch-4-1.x86_64(RepoHIGH)
   U__s_(6)candidatenoarch-4-1.i586(RepoHIGH)
   U__s_(7)candidatenoarch-4-1.noarch(RepoHIGH)
   U__s_(12)candidatenoarch-0-2.noarch(RepoMID)
   U__s_(13)candidatenoarch-0-1.x86_64(RepoMID)
   U__s_(14)candidatenoarch-0-1.i586(RepoMID)
   U__s_(15)candidatenoarch-0-1.noarch(RepoMID)
}  */
  BOOST_CHECK_EQUAL( s->candidateObj()->repoInfo().alias(), "RepoHIGH" );
  BOOST_CHECK_EQUAL( s->candidateObj()->edition(), Edition("5-1") );
  BOOST_CHECK_EQUAL( s->candidateObj()->arch(), Arch_noarch );
  // no updateCandidate due to low version
  BOOST_CHECK_EQUAL( s->updateCandidateObj(), s->candidateObj() );
}


/////////////////////////////////////////////////////////////////////////////
//
// Status change tests
//
/////////////////////////////////////////////////////////////////////////////

// build ResStatus and return whether the comination is supported. (e.g currently no LOCKED state below APPL_HIGH)
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

/////////////////////////////////////////////////////////////////////////////
// status verification helper
/////////////////////////////////////////////////////////////////////////////
//     enum TransactValue
//       {
//         KEEP_STATE = bit::RangeValue<TransactField,0>::value,
//         LOCKED     = bit::RangeValue<TransactField,1>::value, // locked, must not transact
//         TRANSACT   = bit::RangeValue<TransactField,2>::value  // transact according to state
//       };

template <class _Iter>
inline bool _all( _Iter begin_r, _Iter end_r, ResStatus::TransactValue val_r )
{
  for_( it, begin_r, end_r )
  {
    if ( it->status().getTransactValue() != val_r )
      return false;
  }
  return true;
}

template <class _Iter>
inline bool _none( _Iter begin_r, _Iter end_r, ResStatus::TransactValue val_r )
{
  for_( it, begin_r, end_r )
  {
    if ( it->status().getTransactValue() == val_r )
      return false;
  }
  return true;
}

template <class _Iter>
inline bool _atLeastOne( _Iter begin_r, _Iter end_r, ResStatus::TransactValue val_r )
{ return ! _none( begin_r, end_r, val_r ); }

inline bool _allBySolver( ui::Selectable::Ptr sel )
{
  for_( it, sel->installedBegin(), sel->installedEnd() )
  {
    if ( it->status().transacts() && ! it->status().isBySolver() )
      return false;
  }
  for_( it, sel->availableBegin(), sel->availableEnd() )
  {
    if ( it->status().transacts() && ! it->status().isBySolver() )
      return false;
  }
  return true;
}

inline bool _allInstalled( ui::Selectable::Ptr sel, ResStatus::TransactValue val_r )		{ return _all( sel->installedBegin(), sel->installedEnd(), val_r ); }
inline bool _noneInstalled( ui::Selectable::Ptr sel, ResStatus::TransactValue val_r )		{ return _none( sel->installedBegin(), sel->installedEnd(), val_r ); }
inline bool _atLeastOneInstalled( ui::Selectable::Ptr sel, ResStatus::TransactValue val_r )	{ return _atLeastOne( sel->installedBegin(), sel->installedEnd(), val_r ); }

inline bool _allAvailable( ui::Selectable::Ptr sel, ResStatus::TransactValue val_r )		{ return _all( sel->availableBegin(), sel->availableEnd(), val_r ); }
inline bool _noneAvailable( ui::Selectable::Ptr sel, ResStatus::TransactValue val_r )		{ return _none( sel->availableBegin(), sel->availableEnd(), val_r ); }
inline bool _atLeastOneAvailable( ui::Selectable::Ptr sel, ResStatus::TransactValue val_r )	{ return _atLeastOne( sel->availableBegin(), sel->availableEnd(), val_r ); }

inline bool _haveInstalled( ui::Selectable::Ptr sel )						{ return ! sel->installedEmpty(); }
inline bool _haveAvailable( ui::Selectable::Ptr sel )						{ return ! sel->availableEmpty(); }

inline bool _noInstalled( ui::Selectable::Ptr sel )						{ return sel->installedEmpty(); }
inline bool _noAvailable( ui::Selectable::Ptr sel )						{ return sel->availableEmpty(); }

#define allInstalled(V)			_allInstalled(sel,ResStatus::V)
#define noneInstalled(V)		_noneInstalled(sel,ResStatus::V)
#define atLeastOneInstalled(V)		_atLeastOneInstalled(sel,ResStatus::V)

#define allAvailable(V)			_allAvailable(sel,ResStatus::V)
#define noneAvailable(V)		_noneAvailable(sel,ResStatus::V)
#define atLeastOneAvailable(V)		_atLeastOneAvailable(sel,ResStatus::V)

#define haveInstalled			_haveInstalled(sel)
#define haveAvailable			_haveAvailable(sel)
#define noInstalled			_noInstalled(sel)
#define noAvailable			_noAvailable(sel)

#define allBySolver			_allBySolver(sel)

// Verify Selectable::status computes the right value.
//
//       S_Protected,           // Keep this unmodified ( have installedObj && S_Protected )
//       S_Taboo,               // Keep this unmodified ( have no installedObj && S_Taboo)
//       // requested by user:
//       S_Del,                 // delete  installedObj ( clears S_Protected if set )
//       S_Update,              // install candidateObj ( have installedObj, clears S_Protected if set )
//       S_Install,             // install candidateObj ( have no installedObj, clears S_Taboo if set )
//       // not requested by user:
//       S_AutoDel,             // delete  installedObj
//       S_AutoUpdate,          // install candidateObj ( have installedObj )
//       S_AutoInstall,         // install candidateObj ( have no installedObj )
//       // no modification:
//       S_KeepInstalled,       // no modification      ( have installedObj && !S_Protected, clears S_Protected if set )
//       S_NoInst,              // no modification      ( have no installedObj && !S_Taboo, clears S_Taboo if set )
void verifyState( ui::Selectable::Ptr sel )
{
  ui::Status status( sel->status() );
  SEC << dump(sel) << endl;
  switch ( status )
  {
    case ui::S_Update:
    case ui::S_AutoUpdate:
      BOOST_CHECK( haveInstalled );
      BOOST_CHECK( atLeastOneAvailable(TRANSACT) );
      BOOST_CHECK_EQUAL( allBySolver, status==ui::S_AutoUpdate );
      break;

    case ui::S_Del:
    case ui::S_AutoDel:
      BOOST_CHECK( haveInstalled );
      BOOST_CHECK( noneAvailable(TRANSACT) );	// else would be UPDATE
      BOOST_CHECK( atLeastOneInstalled(TRANSACT) );
      BOOST_CHECK_EQUAL( allBySolver, status==ui::S_AutoDel );
      break;

    case ui::S_Protected:
      BOOST_CHECK( haveInstalled );
      BOOST_CHECK( noneAvailable(TRANSACT) );	// else would be UPDATE
      BOOST_CHECK( noneInstalled(TRANSACT) );	// else would be DEL
      BOOST_CHECK( allInstalled(LOCKED) );	// implies noneInstalled(TRANSACT)
      break;

    case ui::S_KeepInstalled:
      BOOST_CHECK( haveInstalled );
      BOOST_CHECK( noneAvailable(TRANSACT) );	// else would be UPDATE
      BOOST_CHECK( noneInstalled(TRANSACT) );	// else would be DEL
      BOOST_CHECK( ! allInstalled(LOCKED) );	// else would be PROTECTED
      break;


    case ui::S_Install:
    case ui::S_AutoInstall:
      BOOST_CHECK( noInstalled );
      BOOST_CHECK( atLeastOneAvailable(TRANSACT) );
      BOOST_CHECK_EQUAL( allBySolver, status==ui::S_AutoInstall );
      break;

    case ui::S_Taboo:
      BOOST_CHECK( noInstalled );
      BOOST_CHECK( noneAvailable(TRANSACT) );	// else would be INSTALL
      BOOST_CHECK( allAvailable(LOCKED) );	// implies noneAvailable(TRANSACT)
      break;


    case ui::S_NoInst:
      BOOST_CHECK( noInstalled );
      BOOST_CHECK( noneAvailable(TRANSACT) );	// else would be INSTALL
      BOOST_CHECK( ! allAvailable(LOCKED) );	// else would be TABOO
      break;
  }
}

// Create all ResStatus combinations over a Selectables PoolItems
struct StatusCombination
{
  StatusCombination()
  {}
  StatusCombination( ui::Selectable::Ptr sel_r )
  {
   _items.insert( _items.end(), sel_r->installedBegin(), sel_r->installedEnd() );
   _items.insert( _items.end(), sel_r->availableBegin(), sel_r->availableEnd() );
  }
  bool next()
  {
    for (auto i : _items)
    {
      switch ( i.status().getTransactValue() )
      {
        case ResStatus::KEEP_STATE:
          i.status().setTransactValue( ResStatus::LOCKED, ResStatus::USER );
          return true;
          break;
        case ResStatus::LOCKED:
          i.status().setTransactValue( ResStatus::TRANSACT, ResStatus::USER );
          return true;
          break;
        case ResStatus::TRANSACT:
          i.status().setTransactValue( ResStatus::KEEP_STATE, ResStatus::USER );
          break;
      }
    }
    return false; // back at the beginning
  }

  std::vector<PoolItem> _items;
};

// Create all ResStatus combinations over the Selectables PoolItems and verify the result.
void testStatusTable( ui::Selectable::Ptr sel )
{
  StatusCombination comb( sel );
  do {
    verifyState( sel );
  } while ( comb.next() );
}

BOOST_AUTO_TEST_CASE(status_verify)
{
  // this verifies the Selectables computes ui::Status
  ResPoolProxy poolProxy( test.poolProxy() );
  ResPoolProxy::ScopedSaveState saveState( poolProxy );
  {
    ui::Selectable::Ptr sel( poolProxy.lookup( ResKind::package, "installed_only" ) );
    BOOST_REQUIRE( !sel->installedEmpty() );
    BOOST_REQUIRE( sel->availableEmpty() );
    BOOST_CHECK_EQUAL( sel->status(), ui::S_KeepInstalled );
    testStatusTable( sel );
  }
  {
    ui::Selectable::Ptr sel( poolProxy.lookup( ResKind::package, "installed_and_available" ) );
    BOOST_REQUIRE( !sel->installedEmpty() );
    BOOST_REQUIRE( !sel->availableEmpty() );
    BOOST_CHECK_EQUAL( sel->status(), ui::S_KeepInstalled );
    testStatusTable( sel );
  }
  {
    ui::Selectable::Ptr sel( poolProxy.lookup( ResKind::package, "available_only" ) );
    BOOST_REQUIRE( sel->installedEmpty() );
    BOOST_REQUIRE( !sel->availableEmpty() );
    BOOST_CHECK_EQUAL( sel->status(), ui::S_NoInst );
    testStatusTable( sel );
  }

  // TODO: Test the pickStatus computation (w./w.o. multiinstall)
  // TODO: Test status/pickStatus transactions (w./w.o. multiinstall)
}

/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(pickstatus_cycle)
{
  return;
  // TODO: automate it
  ResPoolProxy poolProxy( test.poolProxy() );
  ResPoolProxy::ScopedSaveState saveState( poolProxy );
  ui::Selectable::Ptr sel( poolProxy.lookup( ResKind::package, "installed_and_available" ) );

  USR << dump(sel) << endl;
  for ( const PoolItem & pi : sel->picklist() )
  {
    (sel->pickInstall( pi, ResStatus::USER ) ? WAR : ERR) << (pi.multiversionInstall() ? "M " : "  " ) << pi << endl;
    USR << dump(sel) << endl;
  }
}

/////////////////////////////////////////////////////////////////////////////
