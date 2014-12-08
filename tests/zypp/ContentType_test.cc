#include <boost/test/auto_unit_test.hpp>
#include <iostream>
#include <set>
#include "zypp/ContentType.h"

using std::cout;
using std::endl;

using zypp::ContentType;

BOOST_AUTO_TEST_CASE(contenttype_default)
{
  ContentType v;
  BOOST_CHECK( !v );
  BOOST_CHECK( v.empty() );
  BOOST_CHECK( v.emptyType() );
  BOOST_CHECK( v.emptySubtype() );

  ContentType w( "/" );
  BOOST_CHECK_EQUAL( v == w, true );
  BOOST_CHECK_EQUAL( v != w, false );
  BOOST_CHECK_EQUAL( v <  w, false );
  BOOST_CHECK_EQUAL( v <= w, true );
  BOOST_CHECK_EQUAL( v >  w, false );
  BOOST_CHECK_EQUAL( v >= w, true );

  BOOST_CHECK_EQUAL( v.asString(), "" );
}

BOOST_AUTO_TEST_CASE(contenttype_val)
{
  BOOST_CHECK_THROW( ContentType( " " ), std::invalid_argument );

  BOOST_CHECK_THROW( ContentType( "//" ), std::invalid_argument );
  BOOST_CHECK_THROW( ContentType( "/ " ), std::invalid_argument );

  BOOST_CHECK_THROW( ContentType( "/", "a" ), std::invalid_argument );
  BOOST_CHECK_THROW( ContentType( "a", "/" ), std::invalid_argument );

  BOOST_CHECK_THROW( ContentType( " ", "a" ), std::invalid_argument );
  BOOST_CHECK_THROW( ContentType( "a", " " ), std::invalid_argument );
}

BOOST_AUTO_TEST_CASE(contenttype_cmp)
{
  std::set<ContentType> c( {
    ContentType( "" ),
    ContentType( "/" ),		// == ""
    ContentType( "a" ),
    ContentType( "a/" ),	// == "a"
    ContentType( "/a" ),
    ContentType( "" , "a" ),	// == "/a"
    ContentType( "a/b" ),
    ContentType( "b/b" ),
    ContentType( "b/c" )
  });

  std::set<ContentType>::const_iterator i = c.begin();
  BOOST_CHECK_EQUAL( *(i++), ContentType() );
  BOOST_CHECK_EQUAL( *(i++), ContentType( "", "a" ) );
  BOOST_CHECK_EQUAL( *(i++), ContentType( "a", "" ) );
  BOOST_CHECK_EQUAL( *(i++), ContentType( "a", "b" ) );
  BOOST_CHECK_EQUAL( *(i++), ContentType( "b", "b" ) );
  BOOST_CHECK_EQUAL( *(i++), ContentType( "b", "c" ) );
  BOOST_CHECK( i == c.end() );
}
