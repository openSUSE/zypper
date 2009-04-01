#include "TestSetup.h"
#include <zypp/sat/WhatObsoletes.h>

namespace zypp
{
  namespace sat
  {
    // Obsoletes may either match against provides, or names.
    // Configuration depends on the behaviour of rpm.
    extern bool obsoleteUsesProvides;
  }
}

BOOST_AUTO_TEST_CASE(WhatObsoletes)
{
  TestSetup test( Arch_x86_64 );
  test.loadTestcaseRepos( TESTS_SRC_DIR"/data/TCWhatObsoletes" );

  sat::Solvable test1( 2 );
  BOOST_REQUIRE_EQUAL( test1.name(), "test1" );

  {
    sat::obsoleteUsesProvides = true;
    sat::WhatObsoletes w( test1 );
    BOOST_REQUIRE( w.size() == 2 );
    // (3)goaway-1-1.i586(@System)
    // (5)meetoo-1-1.i586(@System)
  }
  {
    sat::obsoleteUsesProvides = false;
    sat::WhatObsoletes w( test1 );
    BOOST_REQUIRE( w.size() == 1 );
    // (3)goaway-1-1.i586(@System)
  }
}