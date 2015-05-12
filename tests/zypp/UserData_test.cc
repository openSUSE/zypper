#include <boost/test/auto_unit_test.hpp>
#include <iostream>
#include <set>
#include "zypp/UserData.h"

using std::cout;
using std::endl;

using zypp::callback::UserData;
const std::string key( "key" );

#define checkIsEmpty(v)	\
  BOOST_CHECK( !v );	\
  BOOST_CHECK( v.empty() );	\
  BOOST_CHECK_EQUAL( v.size(), 0 );	\
  BOOST_CHECK_EQUAL( v.haskey( key ), false );	\
  BOOST_CHECK_EQUAL( v.hasvalue( key ), false );	\
  BOOST_CHECK_EQUAL( v.getvalue( key ).empty(), true );

#define checkIsNotEmpty(v,s)	\
  BOOST_CHECK( v );	\
  BOOST_CHECK( !v.empty() );	\
  if ( s )	\
  { BOOST_CHECK_EQUAL( v.size(), s ); }	\
  else	\
  { BOOST_CHECK( v.size() ); }	\
  BOOST_CHECK_EQUAL( v.haskey( key ), true );


BOOST_AUTO_TEST_CASE(useruata_default)
{
  UserData v;
  checkIsEmpty( v );

  // set key with empty value
  v.reset( key );
  checkIsNotEmpty( v, 1 );
  BOOST_CHECK_EQUAL( v.hasvalue( key ), false );
  BOOST_CHECK_EQUAL( v.getvalue( key ).empty(), true );

  std::string rs;
  unsigned    ru = 0;
  int         ri = 0;
  char        rc = 0;

  // set key with value
  v.set( key, 42 );
  BOOST_CHECK_EQUAL( v.hasvalue( key ), true );
  BOOST_CHECK_EQUAL( v.getvalue( key ).empty(), false );

  // get back data
  BOOST_CHECK_EQUAL( v.get( key, rs ), false );
  BOOST_CHECK_EQUAL( v.get( key, ru ), false );
  BOOST_CHECK_EQUAL( v.get( key, ri ), true );
  BOOST_CHECK_EQUAL( v.get( key, rc ), false );
  BOOST_CHECK_EQUAL( ru, 0 );
  BOOST_CHECK_EQUAL( ri, 42 );
  BOOST_CHECK_EQUAL( rc, 0 );

  v.set( key, 43U );
  BOOST_CHECK_EQUAL( v.get( key, rs ), false );
  BOOST_CHECK_EQUAL( v.get( key, ru ), true );
  BOOST_CHECK_EQUAL( v.get( key, ri ), false );
  BOOST_CHECK_EQUAL( v.get( key, rc ), false );
  BOOST_CHECK_EQUAL( ru, 43 );
  BOOST_CHECK_EQUAL( ri, 42 );
  BOOST_CHECK_EQUAL( rc, 0 );

  // set key with empty value
  v.reset( key );
  BOOST_CHECK_EQUAL( v.hasvalue( key ), false );
  BOOST_CHECK_EQUAL( v.getvalue( key ).empty(), true );
  checkIsNotEmpty( v, 1 );

  // erase key
  v.erase( key );
  BOOST_CHECK_EQUAL( v.hasvalue( key ), false );
  BOOST_CHECK_EQUAL( v.getvalue( key ).empty(), true );
  checkIsEmpty( v );

  // const may add but not manip non-empty values
  const UserData & cv( v );
  BOOST_CHECK_EQUAL( cv.reset( key ), true );	// add new key: ok
  BOOST_CHECK_EQUAL( cv.set( key, 42 ), true );	// empty -> non-empty: ok
  BOOST_CHECK_EQUAL( cv.set( key, 43 ), false );// change non-empty: not ok
  BOOST_CHECK_EQUAL( cv.reset( key ), false );	// change non-empty: not ok
}
