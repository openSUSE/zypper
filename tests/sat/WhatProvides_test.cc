#include "TestSetup.h"

BOOST_AUTO_TEST_CASE(WhatProvides)
{
  TestSetup test( Arch_x86_64 );
  test.loadRepo( TESTS_SRC_DIR"/data/openSUSE-11.1" );

  {
    sat::WhatProvides q( Capability("zypper") );
    BOOST_CHECK( ! q.empty() );
    BOOST_CHECK( q.size() == 1 );
  }

  {
    sat::WhatProvides q( Capability("zypper.x86_64 == 0.12.5-1.1") );
    BOOST_CHECK( ! q.empty() );
    BOOST_CHECK( q.size() == 1 );
  }

  {
    sat::WhatProvides q( Capability("zypper.i586 == 0.12.5-1.1") );
    BOOST_CHECK( q.empty() );
    BOOST_CHECK( q.size() == 0 );
  }
}
