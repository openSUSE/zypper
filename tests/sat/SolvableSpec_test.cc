#include <stdio.h>
#include <iostream>
#include <boost/test/unit_test.hpp>

#include <zypp/sat/SolvableSpec.h>
#include <zypp/base/Logger.h>
#include "TestSetup.h"

#define BOOST_CHECK_MODULE SolvableSpec

using std::cout;
using std::endl;
using std::string;
using namespace zypp;
using namespace boost::unit_test;


BOOST_AUTO_TEST_CASE(parsing)
{
  {
    sat::SolvableSpec specs;
    // adds no empty values
    specs.addIdent   ( IdString() );
    specs.addIdent   ( IdString( "" ) );
    specs.addProvides( Capability() );
    specs.addProvides( Capability( "" ) );
    BOOST_CHECK( ! specs.containsIdent( IdString() ) );
    BOOST_CHECK( ! specs.containsProvides( Capability() ) );
    BOOST_CHECK( ! specs.containsIdent( IdString( "" ) ) );
    BOOST_CHECK( ! specs.containsProvides( Capability( "" ) ) );
  }
  {
    sat::SolvableSpec specs;
    specs.addIdent   ( IdString( "idstr" ) );
    specs.addProvides( Capability( "idcap" ) );
    specs.addProvides( Capability( "idvcap=13" ) );
    specs.addProvides( Capability( "idvwcap = 14" ) );
    BOOST_CHECK( specs.containsIdent   ( IdString( "idstr" ) ) );
    BOOST_CHECK( specs.containsProvides( Capability( "", "idcap", "", "" ) ) );
    BOOST_CHECK( specs.containsProvides( Capability( "", "idvcap", "=", "13" ) ) );
    BOOST_CHECK( specs.containsProvides( Capability( "", "idvwcap", "=", "14" ) ) );
  }
  {
    sat::SolvableSpec specs;
    specs.parse( "idstr" );
    specs.parse( "provides:idcap" );
    specs.parse( "provides:idvcap=13" );
    specs.parse( "provides:idvwcap = 14" );
    BOOST_CHECK( specs.containsIdent   ( IdString( "idstr" ) ) );
    BOOST_CHECK( specs.containsProvides( Capability( "", "idcap", "", "" ) ) );
    BOOST_CHECK( specs.containsProvides( Capability( "", "idvcap", "=", "13" ) ) );
    BOOST_CHECK( specs.containsProvides( Capability( "", "idvwcap", "=", "14" ) ) );
  }
  {
    sat::SolvableSpec specs;
    std::stringstream str;
    str << "# some comment" << endl;
    str << "  # maybe indented" << endl;
    str << "  \t   " << endl;	// white line
    str << " idstr " << endl;
      str << " provides:idcap  " << endl;
    str << " provides:idvcap=13 " << endl;
    str << " provides:idvwcap = 14 " << endl;
    str << "" << endl;
    specs.parseFrom( str );
    BOOST_CHECK( specs.containsIdent   ( IdString( "idstr" ) ) );
    BOOST_CHECK( specs.containsProvides( Capability( "", "idcap", "", "" ) ) );
    BOOST_CHECK( specs.containsProvides( Capability( "", "idvcap", "=", "13" ) ) );
    BOOST_CHECK( specs.containsProvides( Capability( "", "idvwcap", "=", "14" ) ) );
  }
  {
    sat::SolvableSpec specs;
    specs.splitParseFrom( "idstr  provides:idcap  provides:idvcap=13  provides:idvwcap\\ =\\ 14  id\\ ws\\ str  provides:id\\ ws\\ cap\\ =\\ 99" );
    BOOST_CHECK( specs.containsIdent   ( IdString( "idstr" ) ) );
    BOOST_CHECK( specs.containsProvides( Capability( "", "idcap", "", "" ) ) );
    BOOST_CHECK( specs.containsProvides( Capability( "", "idvcap", "=", "13" ) ) );
    BOOST_CHECK( specs.containsProvides( Capability( "", "idvwcap", "=", "14" ) ) );
    BOOST_CHECK( specs.containsIdent   ( IdString( "id ws str" ) ) );
    BOOST_CHECK( specs.containsProvides( Capability( "", "id ws cap", "=", "99" ) ) );
  }
}

BOOST_AUTO_TEST_CASE(eval)
{
  sat::SolvableSpec specs;
  specs.parse( "candidate" );
  BOOST_CHECK( !specs.dirty() );	// ident don't
  specs.parse( "provides:available_only > 1-1" );
  BOOST_CHECK( specs.dirty() );		// deps do

  TestSetup test( Arch_x86_64 );
  test.loadTestcaseRepos( TESTS_SRC_DIR "/data/TCSelectable" );
  std::set<sat::Solvable::IdType> matches {2,3,8,9,14,15,16,17,28};	// matching Solvable IDs in TestcaseRepo (does not match srcpackage:!)
  for ( const auto & solv : ResPool::instance() )
  {
    //cout << ( specs.contains( solv ) ? "* " : "  " ) << solv << endl;
    BOOST_CHECK_MESSAGE( specs.contains( solv ) == matches.count( solv.id() ), str::Str() << "Wrong: " << ( specs.contains( solv ) ? "* " : "  " ) << solv );
  }

  BOOST_CHECK( !specs.dirty() );	// loop built the cache
  specs.parse( "provides:available_only > 1-1" );
  BOOST_CHECK( !specs.dirty() );	// already have this spec, no need to set dirty

  specs.parse( "provides:available_only = 1-1" );
  BOOST_CHECK( specs.dirty() );		// This is a new one, so got dirty
  matches.insert( 13 );

  for ( const auto & solv : ResPool::instance() )
  {
    BOOST_CHECK_MESSAGE( specs.contains( solv ) == matches.count( solv.id() ), str::Str() << "Wrong: " << ( specs.contains( solv ) ? "* " : "  " ) << solv );
  }
}
