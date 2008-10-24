#include "TestSetup.h"
#include "zypp/ResStatus.h"

#define BOOST_TEST_MODULE ResStatus

BOOST_AUTO_TEST_CASE(ResStatus_default)
{
  {
    ResStatus s;
    BOOST_CHECK( s.isUninstalled() );
    BOOST_CHECK_EQUAL( s.isInstalled(), ! s.isUninstalled() );
    BOOST_CHECK_EQUAL( s.getTransactValue(),   KEEP_STATE );
    BOOST_CHECK_EQUAL( s.getTransactByValue(), SOLVER );
  }
  {
    ResStatus s( true );
    BOOST_CHECK( s.Installed() );
    BOOST_CHECK_EQUAL( s.isInstalled(), ! s.isUninstalled() );
    BOOST_CHECK_EQUAL( s.getTransactValue(),   KEEP_STATE );
    BOOST_CHECK_EQUAL( s.getTransactByValue(), SOLVER );
  }
}
#if 0

//     enum TransactValue
//       {
//         KEEP_STATE = bit::RangeValue<TransactField,0>::value,
//         LOCKED     = bit::RangeValue<TransactField,1>::value, // locked, must not transact
//         TRANSACT   = bit::RangeValue<TransactField,2>::value  // transact according to state
//       };
//     enum TransactByValue
//       {
//         SOLVER    = bit::RangeValue<TransactByField,0>::value,
//         APPL_LOW  = bit::RangeValue<TransactByField,1>::value,
//         APPL_HIGH = bit::RangeValue<TransactByField,2>::value,
//         USER      = bit::RangeValue<TransactByField,3>::value
//       };

ResStatus doTestSet( TransactValue tv, TransactByValue tb )
{
  ResStatus s;
  s.setTransactValue( tv, tb );
  BOOST_CHECK_EQUAL( getTransactValue( s ), tv );
  BOOST_CHECK_EQUAL( getTransactByValue( s ), tb );
}



BOOST_AUTO_TEST_CASE(ResStatus)
{
  std::vector<TransactValue> TVs;
  TVs.push_back( KEEP_STATE );
  TVs.push_back( LOCKED );
  TVs.push_back( TRANSACT );

  std::vector<TransactByValue> TBs;
  TBs.push_back( SOLVER );
  TBs.push_back( APPL_LOW );
  TBs.push_back( APPL_HIGH );
  TBs.push_back( USER );

  for_( tv, TVs.begin(), TVs end() )
  {
    for_( tb, TBs.begin(), TBs end() )
    {
      doTestSet

    }
  }




}
#endif