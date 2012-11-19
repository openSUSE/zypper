/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

/** \file HistoryLogReader.cc
 *
 */
#include <iostream>

#include "zypp/base/InputStream.h"
#include "zypp/base/IOStream.h"
#include "zypp/base/Logger.h"
#include "zypp/parser/ParseException.h"

#include "zypp/parser/HistoryLogReader.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////


  /////////////////////////////////////////////////////////////////////
  //
  // CLASS NAME: HistoryLogReader::Impl
  //
  /////////////////////////////////////////////////////////////////////

  struct HistoryLogReader::Impl
  {
    Impl( const Pathname & historyFile, const ProcessItem & callback );
    ~Impl()
    {}

    HistoryItem::Ptr createHistoryItem(HistoryItem::FieldVector & fields);
    void parseLine(const string & line, unsigned int lineNr);

    void readAll( const ProgressData::ReceiverFnc & progress );
    void readFrom( const Date & date, const ProgressData::ReceiverFnc & progress );
    void readFromTo( const Date & fromDate, const Date & toDate, const ProgressData::ReceiverFnc & progress );

    Pathname _filename;
    ProcessItem _callback;
    bool _ignoreInvalid;
  };


  HistoryLogReader::Impl::Impl( const Pathname & historyFile, const ProcessItem & callback )
    : _filename( historyFile )
    , _callback( callback )
    , _ignoreInvalid( false )
  {}


  HistoryItem::Ptr HistoryLogReader::Impl::createHistoryItem( HistoryItem::FieldVector & fields )
  {
    HistoryActionID aid( str::trim( fields[1] ) );
    switch ( aid.toEnum() )
    {
    case HistoryActionID::INSTALL_e:
      return HistoryItemInstall::Ptr( new HistoryItemInstall( fields ) );
      break;

    case HistoryActionID::REMOVE_e:
      return HistoryItemRemove::Ptr( new HistoryItemRemove( fields ) );
      break;

    case HistoryActionID::REPO_ADD_e:
      return HistoryItemRepoAdd::Ptr( new HistoryItemRepoAdd( fields ) );
      break;

    case HistoryActionID::REPO_REMOVE_e:
      return HistoryItemRepoRemove::Ptr( new HistoryItemRepoRemove( fields ) );
      break;

    case HistoryActionID::REPO_CHANGE_ALIAS_e:
      return HistoryItemRepoAliasChange::Ptr( new HistoryItemRepoAliasChange( fields ) );
      break;

    case HistoryActionID::REPO_CHANGE_URL_e:
      return HistoryItemRepoUrlChange::Ptr( new HistoryItemRepoUrlChange( fields ) );
      break;
    }
    return HistoryItem::Ptr();
  }


  void HistoryLogReader::Impl::parseLine( const string & line, unsigned int lineNr )
  {
    // parse into fields
    HistoryItem::FieldVector fields;
    str::splitEscaped( line, back_inserter(fields), "|", true );

    if ( fields.size() <= 2 )
    {
      if ( !_ignoreInvalid )
      {
	ParseException e( str::form( "Error in history log on line #%u.", lineNr ) );
	e.addHistory( str::form( "Bad number of fields. Got %zd, expected more than %d.", fields.size(), 2 ) );
	ZYPP_THROW( e );
      }
      else
      {
	WAR << "Ignoring suspicious non-comment entry on line #" << lineNr << endl;
	return;
      }
    }

    HistoryItem::Ptr item_ptr;
    try
    {
      item_ptr = createHistoryItem( fields );
    }
    catch ( const Exception & e )
    {
      ZYPP_CAUGHT(e);
      ERR << "Invalid history log entry on line #" << lineNr << " '"<< line << "'" << endl;

      if ( !_ignoreInvalid )
      {
        ParseException newe( str::form( "Error in history log on line #%u.", lineNr ) );
	newe.remember( e );
	ZYPP_THROW( newe );
      }
    }

    if ( item_ptr )
    {
      _callback( item_ptr );
    }
    else if ( !_ignoreInvalid )
    {
      ParseException e( str::form( "Error in history log on line #%u.", lineNr ) );
      e.addHistory( "Unknown entry type." );
      ZYPP_THROW( e );
    }
    else
    {
      WAR << "Unknown history log action type: " << fields[1] << " on line #" << lineNr << endl;
    }
  }


  void HistoryLogReader::Impl::readAll(const ProgressData::ReceiverFnc & progress)
  {
    InputStream is(_filename);
    iostr::EachLine line(is);

    ProgressData pd;
    pd.sendTo( progress );
    pd.toMin();

    for (; line; line.next(), pd.tick() )
    {
      // ignore comments
      if ((*line)[0] == '#')
        continue;

      parseLine(*line, line.lineNo());
    }

    pd.toMax();
  }

  void HistoryLogReader::Impl::readFrom(const Date & date,
      const ProgressData::ReceiverFnc & progress)
  {
    InputStream is(_filename);
    iostr::EachLine line(is);

    ProgressData pd;
    pd.sendTo( progress );
    pd.toMin();

    bool pastDate = false;
    for (; line; line.next(), pd.tick())
    {
      const string & s = *line;

      // ignore comments
      if (s[0] == '#')
        continue;

      if (pastDate)
        parseLine(s, line.lineNo());
      else
      {
        Date logDate(s.substr(0, s.find('|')), HISTORY_LOG_DATE_FORMAT);
        if (logDate > date)
        {
          pastDate = true;
          parseLine(s, line.lineNo());
        }
      }
    }

    pd.toMax();
  }

  void HistoryLogReader::Impl::readFromTo(
      const Date & fromDate, const Date & toDate,
      const ProgressData::ReceiverFnc & progress)
  {
    InputStream is(_filename);
    iostr::EachLine line(is);

    ProgressData pd;
    pd.sendTo(progress);
    pd.toMin();

    bool pastFromDate = false;
    for (; line; line.next(), pd.tick())
    {
      const string & s = *line;

      // ignore comments
      if (s[0] == '#')
        continue;

      Date logDate(s.substr(0, s.find('|')), HISTORY_LOG_DATE_FORMAT);

      // past toDate - stop reading
      if (logDate >= toDate)
        break;

      // past fromDate - start reading
      if (!pastFromDate && logDate > fromDate)
        pastFromDate = true;

      if (pastFromDate)
        parseLine(s, line.lineNo());
    }

    pd.toMax();
  }

  /////////////////////////////////////////////////////////////////////
  //
  // CLASS NAME: HistoryLogReader
  //
  /////////////////////////////////////////////////////////////////////

  HistoryLogReader::HistoryLogReader( const Pathname & historyFile,
                                      const ProcessItem & callback )
    : _pimpl(new HistoryLogReader::Impl(historyFile, callback))
  {}

  HistoryLogReader::~HistoryLogReader()
  {}

  void HistoryLogReader::setIgnoreInvalidItems(bool ignoreInvalid)
  { _pimpl->_ignoreInvalid = ignoreInvalid; }

  bool HistoryLogReader::ignoreInvalidItems() const
  { return _pimpl->_ignoreInvalid; }

  void HistoryLogReader::readAll(const ProgressData::ReceiverFnc & progress)
  { _pimpl->readAll(progress); }

  void HistoryLogReader::readFrom(const Date & date,
      const ProgressData::ReceiverFnc & progress)
  { _pimpl->readFrom(date, progress); }

  void HistoryLogReader::readFromTo(
      const Date & fromDate, const Date & toDate,
      const ProgressData::ReceiverFnc & progress)
  { _pimpl->readFromTo(fromDate, toDate, progress); }


    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
