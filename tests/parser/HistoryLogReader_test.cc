#include "TestSetup.h"
#include "zypp/parser/HistoryLogReader.h"
#include "zypp/parser/ParseException.h"

using namespace zypp;

namespace
{
  bool ProcessItem( const HistoryItem::Ptr & ptr )
  {
    DBG << ptr << endl;
    return true;
  }
}

// Must be the first test!
BOOST_AUTO_TEST_CASE(basic)
{
  parser::HistoryLogReader parser( TESTS_SRC_DIR "/parser/HistoryLogReader_test.dat",
				   ProcessItem );

  BOOST_CHECK_EQUAL( parser.ignoreInvalidItems(), false );
  BOOST_CHECK_THROW( parser.readAll(), parser::ParseException );

  parser.setIgnoreInvalidItems( true );
  BOOST_CHECK_EQUAL( parser.ignoreInvalidItems(), true );
  parser.readAll();

}
