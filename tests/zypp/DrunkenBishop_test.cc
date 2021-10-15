#include <boost/test/unit_test.hpp>
#include <iostream>
#include <zypp/base/DrunkenBishop.h>

using boost::unit_test::test_case;

using namespace zypp;
using base::DrunkenBishop;

BOOST_AUTO_TEST_CASE(drunkenbishop)
{
  {
    DrunkenBishop b;
    BOOST_CHECK_EQUAL( b.asString(),
                       "++\n"
                       "++" );
  }
  {
    DrunkenBishop b( "94", 0, 0 );
    BOOST_CHECK_EQUAL( b.asString(),
                       "+-+\n"
                       "|E|\n"
                       "+-+" );
  }
  {
    BOOST_CHECK_THROW( DrunkenBishop( "9g" ), std::invalid_argument );
  }
  {
    DrunkenBishop b( "" );
    BOOST_CHECK_EQUAL( b.asString(),
                       "+-----------------+\n"
                       "|                 |\n"
                       "|                 |\n"
                       "|                 |\n"
                       "|                 |\n"
                       "|        E        |\n"
                       "|                 |\n"
                       "|                 |\n"
                       "|                 |\n"
                       "|                 |\n"
                       "+-----------------+" );
  }
  {
    DrunkenBishop b( "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC", "Title" );
    BOOST_CHECK_EQUAL( b.asString(),
                       "+-----[Title]-----+\n"
                       "|                 |\n"
                       "|                 |\n"
                       "|                 |\n"
                       "|       ^         |\n"
                       "|        E        |\n"
                       "|                 |\n"
                       "|                 |\n"
                       "|                 |\n"
                       "|                 |\n"
                       "+---[CCCCCCCC]----+" );
  }
  {
    DrunkenBishop b( "9c6fb17fa201ad829d808739379a2a51", "very very long Title"   );
    BOOST_CHECK_EQUAL( b.asString(),
                       "+[very very long ]+\n"
                       "|                 |\n"
                       "|                 |\n"
                       "|                 |\n"
                       "|  E+   . o       |\n"
                       "| .= =   S o      |\n"
                       "|.  * = . + o     |\n"
                       "| .o . + . =      |\n"
                       "|..     . . o. .  |\n"
                       "|o         ...o   |\n"
                       "+---[379A2A51]----+" );
  }
  {
    DrunkenBishop b( "4E98E67519D98DC7362A5990E3A5C360307E3D54" );
    BOOST_CHECK_EQUAL( b.asString(),
                       "+-------------------+\n"
                       "|       ^.  .^E     |\n"
                       "|      . .^^: .     |\n"
                       "|       ...:^? :    |\n"
                       "|        .  ?.: i   |\n"
                       "|        ^   i : .  |\n"
                       "|       : S l .     |\n"
                       "|      ^ : . .      |\n"
                       "|       . .         |\n"
                       "|                   |\n"
                       "|                   |\n"
                       "|                   |\n"
                       "+----[307E3D54]-----+" );
  }
}
