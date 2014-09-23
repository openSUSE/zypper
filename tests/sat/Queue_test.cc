#include <stdio.h>
#include <iostream>
#include <boost/test/auto_unit_test.hpp>

#include "zypp/base/LogTools.h"
#include "zypp/base/Easy.h"
#include "zypp/sat/Queue.h"


#define BOOST_TEST_MODULE Queue

using std::endl;
using std::cout;
using namespace zypp;
using namespace boost::unit_test;


BOOST_AUTO_TEST_CASE(basic)
{
  sat::Queue m;
  BOOST_CHECK_EQUAL( m.empty(), true );
  BOOST_CHECK_EQUAL( m.size(), 0 );
  BOOST_CHECK_EQUAL( m.begin(), m.end() );
  BOOST_CHECK( m == sat::Queue() );
  BOOST_CHECK_EQUAL( m.first(), 0 );
  BOOST_CHECK_EQUAL( m.last(), 0 );

  m.push( 13 );
  BOOST_CHECK_EQUAL( m.empty(), false );
  BOOST_CHECK_EQUAL( m.size(), 1 );
  BOOST_CHECK( m.begin() != m.end() );
  BOOST_CHECK_EQUAL( m.begin()+1, m.end() );

  BOOST_CHECK_EQUAL( m.first(), 13 );
  BOOST_CHECK_EQUAL( m.last(), 13 );
  BOOST_CHECK_EQUAL( m.at(0), 13 );
  BOOST_CHECK_THROW( m.at(1), std::out_of_range );

  BOOST_CHECK( m.contains(13) );
  BOOST_CHECK( !m.contains(14) );

  BOOST_CHECK_EQUAL( m.find(13), m.begin() );
  BOOST_CHECK_EQUAL( m.find(14), m.end() );

  m.pushUnique( 13 );
  BOOST_CHECK_EQUAL( m.size(), 1 );
  m.push( 13 );
  BOOST_CHECK_EQUAL( m.size(), 2 );
  m.remove( 13 );
  BOOST_CHECK_EQUAL( m.size(), 0 );
}
