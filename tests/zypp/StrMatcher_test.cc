#include "TestSetup.h"
#include <zypp/sat/LookupAttr.h>
#include <zypp/base/StrMatcher.h>
#include <zypp/ResObjects.h>

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : Matcher
//
///////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(Match_default)
{
  Match m;
  BOOST_CHECK( !m ); // eval in boolean context
  BOOST_CHECK_EQUAL( m, Match::NOTHING );
  BOOST_CHECK_EQUAL( m.get(), 0 );

  // set the mode part
  BOOST_CHECK_EQUAL( m |= Match::STRING, 	Match::STRING );

  m.setModeSubstring();
  BOOST_CHECK_EQUAL( m,			 	Match::SUBSTRING );

  m.setMode( Match::GLOB );
  BOOST_CHECK_EQUAL( m, 			Match::GLOB );

  BOOST_CHECK_EQUAL( m = Match::REGEX, 		Match::REGEX );

  BOOST_CHECK( m.isModeRegex() );
  m |= Match::NOCASE | Match::FILES;
  BOOST_CHECK_EQUAL( m, Match::REGEX | Match::NOCASE | Match::FILES );

  BOOST_CHECK( m.testAnyOf( Match::SUBSTRING | Match::NOCASE | Match::FILES ) );
  BOOST_CHECK( !m.test( Match::SUBSTRING | Match::NOCASE | Match::FILES ) );
  BOOST_CHECK( m.test( Match::REGEX | Match::NOCASE | Match::FILES ) );
  BOOST_CHECK( m.test( Match::NOCASE | Match::FILES ) );
  BOOST_CHECK( m != (Match::NOCASE | Match::FILES) );
  BOOST_CHECK_EQUAL( m.flags(),Match::NOCASE | Match::FILES );

  m -= Match::NOCASE; // remove flags
  BOOST_CHECK( m.test( Match::REGEX | Match::FILES ) );
  m -= Match::REGEX;
  BOOST_CHECK_EQUAL( m, Match::FILES );
}

BOOST_AUTO_TEST_CASE(Match_operator)
{
  // Test whether implicit conversions from enum Match::Mode to
  // Matcher work. There must be no difference in using mode and flag
  // constants. These tests usually fail at compiletime, if some operator
  // overload is missing.
  //
  // E.G.:
  // inline Match operator|( const Match & lhs, const Match & rhs )
  //  this does not cover (REGEX|SUBSTRING), because if both arguments
  //  are enum Mode the compiler might want to use operator|(int,int)
  //  instead.

  Match m( Match::GLOB );
  m = Match::GLOB;

  m |= Match::GLOB;
  m = Match::SUBSTRING | Match::GLOB;

  m -= Match::GLOB;
  m = Match::SUBSTRING - Match::GLOB;
}

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : StrMatcher
//
///////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(StrMatcher_defaultconstructed)
{
  StrMatcher m;
  BOOST_CHECK_EQUAL( m.flags(), Match::NOTHING );
  BOOST_CHECK( !m );	// eval in boolean context
  BOOST_CHECK( m.searchstring().empty() );
  BOOST_CHECK_EQUAL( m.flags(), Match() );
  // matches nothing:
  BOOST_CHECK( !m( "" ) );
  BOOST_CHECK( !m( " " ) );
  BOOST_CHECK( !m( "a" ) );
  BOOST_CHECK( !m( "default" ) );

  m.setSearchstring( "fau" );
  BOOST_CHECK( m );	// eval in boolean context
}

BOOST_AUTO_TEST_CASE(StrMatcher_OTHER)
{
  StrMatcher m( "fau", Match::OTHER );
  BOOST_CHECK_THROW( m.compile(), MatchUnknownModeException );
}

BOOST_AUTO_TEST_CASE(StrMatcher_STRING)
{
  StrMatcher m( "fau" );
  BOOST_CHECK_EQUAL( m.flags(), Match::STRING );
  BOOST_CHECK( !m( "" ) );
  BOOST_CHECK( !m( "a" ) );
  BOOST_CHECK( m( "fau" ) );
  BOOST_CHECK( !m( "default" ) );
}

BOOST_AUTO_TEST_CASE(StrMatcher_STRINGSTART)
{
  StrMatcher m( "fau", Match::STRINGSTART );
  BOOST_CHECK( !m( "" ) );
  BOOST_CHECK( !m( "a" ) );
  BOOST_CHECK( m( "fau" ) );
  BOOST_CHECK( m( "fault" ) );
  BOOST_CHECK( !m( "default" ) );
}

BOOST_AUTO_TEST_CASE(StrMatcher_STRINGEND)
{
  StrMatcher m( "fau", Match::STRINGEND );
  BOOST_CHECK( !m( "" ) );
  BOOST_CHECK( !m( "a" ) );
  BOOST_CHECK( m( "fau" ) );
  BOOST_CHECK( m( "defau" ) );
  BOOST_CHECK( !m( "default" ) );
}

BOOST_AUTO_TEST_CASE(StrMatcher_REGEX)
{
  StrMatcher m( "fau" );

  BOOST_CHECK( !m.isCompiled() );
  BOOST_CHECK_NO_THROW( m.compile() );

  m.setSearchstring( "wa[" );
  BOOST_CHECK( !m.isCompiled() );
  m.setFlags( Match::REGEX );
  BOOST_CHECK( !m.isCompiled() );
  BOOST_CHECK_THROW( m.compile(), MatchInvalidRegexException );
  BOOST_CHECK( !m.isCompiled() );

  m.setSearchstring( "wa[a]" );
  BOOST_CHECK_NO_THROW( m.compile() );
  BOOST_CHECK( m.isCompiled() );

  BOOST_CHECK( !m( "was" ) );
  BOOST_CHECK( !m( "qwasq" ) );
  BOOST_CHECK( m( "qwaaq" ) );
}

#if 0
BOOST_AUTO_TEST_CASE(StrMatcher_)
{
  base::LogControl::TmpLineWriter shutUp( new log::FileLineWriter( "/tmp/YLOG" ) );
  MIL << "GO" << endl;
}
#endif
