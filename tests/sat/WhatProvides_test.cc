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

  {
    // sat::WhatProvides::const_iterator requires proper
    // copyctor and assignment. If they break
    sat::WhatProvides q;
    BOOST_CHECK( q.begin() == q.begin() );
    BOOST_CHECK( q.begin() == q.end()  );

    q = sat::WhatProvides( Capability("zypper.x86_64 == 0.12.5-1.1") );
    // q no longer empty
    BOOST_CHECK( q.begin() == q.begin() );
    BOOST_CHECK( q.begin() != q.end() );

    sat::WhatProvides::const_iterator a;
    BOOST_CHECK( a == q.end() );

    sat::WhatProvides::const_iterator b( q.begin() );
    BOOST_CHECK( b == q.begin() );

//     SEC << LABELED(q.begin()) << endl;
//     SEC << LABELED(q.end()) << endl;
//     SEC << LABELED(a) << endl;
//     SEC << LABELED(b) << endl;

    {
      a = q.begin();
      BOOST_CHECK( a == q.begin() );
//       SEC << LABELED(a) << endl;
//       SEC << LABELED(b) << endl;

      a = b;
      BOOST_CHECK( a == b );
//       SEC << LABELED(a) << endl;
//       SEC << LABELED(b) << endl;
    }
    BOOST_CHECK( a == q.begin() );
  }
}
