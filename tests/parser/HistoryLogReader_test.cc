#include "TestSetup.h"
#define WITH_DEPRECATED_HISTORYITEM_API
#include "zypp/parser/HistoryLogReader.h"
#include "zypp/parser/ParseException.h"

using namespace zypp;

#if defined(WITH_DEPRECATED_HISTORYITEM_API)
namespace
{
  bool OldApi_ProcessItem( const HistoryItem::Ptr & ptr )
  {
    DBG << ptr << endl;
    return true;
  }
}

BOOST_AUTO_TEST_CASE(OldApi_basic)
{
  parser::HistoryLogReader parser( TESTS_SRC_DIR "/parser/HistoryLogReader_test.dat",
				   OldApi_ProcessItem );

  BOOST_CHECK_EQUAL( parser.ignoreInvalidItems(), false );
  BOOST_CHECK_THROW( parser.readAll(), parser::ParseException );

  parser.setIgnoreInvalidItems( true );
  BOOST_CHECK_EQUAL( parser.ignoreInvalidItems(), true );
  parser.readAll();
}

#endif // WITH_DEPRECATED_HISTORYITEM_API

namespace
{
  bool ProcessData( const HistoryLogData::Ptr & ptr )
  {
    DBG << ptr->date() << " | " << ptr << endl;

    return true;
  }
}


BOOST_AUTO_TEST_CASE(basic)
{
  std::vector<HistoryLogData::Ptr> history;
  parser::HistoryLogReader parser( TESTS_SRC_DIR "/parser/HistoryLogReader_test.dat",
				   parser::HistoryLogReader::Options(),
    [&history]( HistoryLogData::Ptr ptr )->bool {
      history.push_back( ptr );
      return true;
    } );

  BOOST_CHECK_EQUAL( parser.ignoreInvalidItems(), false );
  BOOST_CHECK_THROW( parser.readAll(), parser::ParseException );

  parser.setIgnoreInvalidItems( true );
  BOOST_CHECK_EQUAL( parser.ignoreInvalidItems(), true );

  history.clear();
  parser.readAll();

  BOOST_CHECK_EQUAL( history.size(), 7 );
  BOOST_CHECK( dynamic_pointer_cast<HistoryLogDataRepoAdd>	( history[0] ) );
  BOOST_CHECK( dynamic_pointer_cast<HistoryLogDataInstall>	( history[1] ) );
  BOOST_CHECK( dynamic_pointer_cast<HistoryLogDataInstall>	( history[2] ) );
  BOOST_CHECK( dynamic_pointer_cast<HistoryLogDataRemove>	( history[3] ) );
  BOOST_CHECK( dynamic_pointer_cast<HistoryLogDataRepoRemove>	( history[4] ) );
  BOOST_CHECK( dynamic_pointer_cast<HistoryLogDataRemove>	( history[5] ) );
  BOOST_CHECK( dynamic_pointer_cast<HistoryLogData>		( history[6] ) );

  BOOST_CHECK_EQUAL( (*history[1])[HistoryLogDataInstall::USERDATA_INDEX], "trans|ID" ); // properly (un)escaped?
  HistoryLogDataInstall::Ptr p = dynamic_pointer_cast<HistoryLogDataInstall>( history[1] );
  BOOST_CHECK_EQUAL( p->userdata(), "trans|ID" ); // properly (un)escaped?
}
