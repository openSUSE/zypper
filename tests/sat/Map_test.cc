#include <stdio.h>
#include <iostream>
#include <boost/test/auto_unit_test.hpp>

#include "zypp/base/LogTools.h"
#include "zypp/base/Easy.h"
#include "zypp/sat/Map.h"


#define BOOST_TEST_MODULE Map

using std::endl;
using std::cout;
using namespace zypp;
using namespace boost::unit_test;


BOOST_AUTO_TEST_CASE(basic)
{
  sat::Map m;
  BOOST_CHECK_EQUAL( m.empty(), true );
  BOOST_CHECK_EQUAL( m.size(), 0 );
  BOOST_CHECK_EQUAL( m.asString(), "" );
  BOOST_CHECK( m == sat::Map() );

  m.grow( 8 );
  BOOST_CHECK_EQUAL( m.empty(), false );
  BOOST_CHECK_EQUAL( m.size(), 8 );
  BOOST_CHECK_EQUAL( m.asString(), "00000000" );
  BOOST_CHECK( m != sat::Map() );

  m.grow( 9 );
  BOOST_CHECK_EQUAL( m.empty(), false );
  BOOST_CHECK_EQUAL( m.size(), 16 );
  BOOST_CHECK_EQUAL( m.asString(), "0000000000000000" );

  m.grow( 0 ); // no shrink!
  BOOST_CHECK_EQUAL( m.size(), 16 );

  m.setAll();
  BOOST_CHECK_EQUAL( m.asString(), "1111111111111111" );

  m.clear( 0 );
  m.assign( 3, false );
  BOOST_CHECK_EQUAL( m.asString(), "0110111111111111" );
  BOOST_CHECK_EQUAL( m.test( 0 ), false );
  BOOST_CHECK_EQUAL( m.test( 1 ), true );

  // COW
  m.clearAll();
  sat::Map n(m);
  BOOST_CHECK_EQUAL( m.asString(), "0000000000000000" );
  BOOST_CHECK_EQUAL( n.asString(), "0000000000000000" );
  BOOST_CHECK_EQUAL( m, n );

  m.set( 1 );
  BOOST_CHECK_EQUAL( m.asString(), "0100000000000000" );
  BOOST_CHECK_EQUAL( n.asString(), "0000000000000000" );
  BOOST_CHECK( m != n );

  n.set( 1 );
  BOOST_CHECK_EQUAL( m.asString(), "0100000000000000" );
  BOOST_CHECK_EQUAL( n.asString(), "0100000000000000" );
  BOOST_CHECK( m == n );


  BOOST_CHECK_THROW( m.set( 99 ), std::out_of_range );
}
