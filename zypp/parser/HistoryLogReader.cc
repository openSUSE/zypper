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

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace parser
  {

  /////////////////////////////////////////////////////////////////////
  //
  //	class HistoryLogReader::Impl
  //
  /////////////////////////////////////////////////////////////////////
  struct HistoryLogReader::Impl
  {
    Impl( const Pathname & historyFile_r, const Options & options_r, const ProcessData & callback_r )
    :  _filename( historyFile_r )
    , _options( options_r )
    , _callback( callback_r )
    {}

    bool parseLine( const std::string & line_r, unsigned int lineNr_r );

    void readAll( const ProgressData::ReceiverFnc & progress_r );
    void readFrom( const Date & date_r, const ProgressData::ReceiverFnc & progress_r );
    void readFromTo( const Date & fromDate_r, const Date & toDate_r, const ProgressData::ReceiverFnc & progress_r );

    Pathname _filename;
    Options  _options;
    ProcessData _callback;

#if defined(WITH_DEPRECATED_HISTORYITEM_API)
    ProcessItem _oldAPICallback;
  private:
    HistoryItem::Ptr _oldAPIcreateHistoryItem( HistoryItem::FieldVector & fields );
    void _oldAPIparseLine(const std::string & line, unsigned int lineNr);
#endif // WITH_DEPRECATED_HISTORYITEM_API
  };

  bool HistoryLogReader::Impl::parseLine( const std::string & line_r, unsigned lineNr_r )
  {
#if defined(WITH_DEPRECATED_HISTORYITEM_API)
    if ( _oldAPICallback )
    {
      _oldAPIparseLine( line_r, lineNr_r );
      return true;	// old api did not eavluate callback return value :(
    }
#endif // WITH_DEPRECATED_HISTORYITEM_API

    // parse into fields
    HistoryLogData::FieldVector fields;
    str::splitEscaped( line_r, std::back_inserter(fields), "|", true );
    if ( fields.size() >= 2 )
      str::trim( fields[1] );	// for whatever reason writer is padding the action field

    // move into data class
    HistoryLogData::Ptr data;
    try
    {
      data = HistoryLogData::create( fields );
    }
    catch ( const Exception & excpt )
    {
      ZYPP_CAUGHT( excpt );
      if ( _options.testFlag( IGNORE_INVALID_ITEMS ) )
      {
	WAR << "Ignore invalid history log entry on line #" << lineNr_r << " '"<< line_r << "'" << endl;
	return true;
      }
      else
      {
	ERR << "Invalid history log entry on line #" << lineNr_r << " '"<< line_r << "'" << endl;
	ParseException newexcpt( str::Str() << "Error in history log on line #" << lineNr_r );
	newexcpt.remember( excpt );
	ZYPP_THROW( newexcpt );
      }
    }

    // consume data
    if ( _callback && !_callback( data ) )
    {
      WAR << "Stop parsing requested by consumer callback on line #" << lineNr_r << endl;
      return false;
    }
    return true;
  }

  void HistoryLogReader::Impl::readAll( const ProgressData::ReceiverFnc & progress_r )
  {
    InputStream is( _filename );
    iostr::EachLine line( is );

    ProgressData pd;
    pd.sendTo( progress_r );
    pd.toMin();

    for ( ; line; line.next(), pd.tick() )
    {
      // ignore comments
      if ( (*line)[0] == '#' )
        continue;

      if ( ! parseLine( *line, line.lineNo() ) )
	break;	// requested by consumer callback
    }

    pd.toMax();
  }

  void HistoryLogReader::Impl::readFrom( const Date & date_r, const ProgressData::ReceiverFnc & progress_r )
  {
    InputStream is( _filename );
    iostr::EachLine line( is );

    ProgressData pd;
    pd.sendTo( progress_r );
    pd.toMin();

    bool pastDate = false;
    for ( ; line; line.next(), pd.tick() )
    {
      const std::string & s = *line;

      // ignore comments
      if ( s[0] == '#' )
        continue;

      if ( pastDate )
      {
	if ( ! parseLine( s, line.lineNo() ) )
	  break;	// requested by consumer callback
      }
      else
      {
        Date logDate( s.substr( 0, s.find('|') ), HISTORY_LOG_DATE_FORMAT );
        if ( logDate > date_r )
        {
          pastDate = true;
          if ( ! parseLine( s, line.lineNo() ) )
	    break;	// requested by consumer callback
        }
      }
    }

    pd.toMax();
  }

  void HistoryLogReader::Impl::readFromTo( const Date & fromDate_r, const Date & toDate_r, const ProgressData::ReceiverFnc & progress_r )
  {
    InputStream is( _filename );
    iostr::EachLine line( is );

    ProgressData pd;
    pd.sendTo( progress_r );
    pd.toMin();

    bool pastFromDate = false;
    for ( ; line; line.next(), pd.tick() )
    {
      const std::string & s = *line;

      // ignore comments
      if ( s[0] == '#' )
        continue;

      Date logDate( s.substr( 0, s.find('|') ), HISTORY_LOG_DATE_FORMAT );

      // past toDate - stop reading
      if ( logDate >= toDate_r )
        break;

      // past fromDate - start reading
      if ( !pastFromDate && logDate > fromDate_r )
        pastFromDate = true;

      if ( pastFromDate )
      {
	if ( ! parseLine( s, line.lineNo() ) )
	  break;	// requested by consumer callback
      }
    }

    pd.toMax();
  }


#if defined(WITH_DEPRECATED_HISTORYITEM_API)
  HistoryItem::Ptr HistoryLogReader::Impl::_oldAPIcreateHistoryItem( HistoryItem::FieldVector & fields )
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

    case HistoryActionID::NONE_e:
      break;
    }
    return HistoryItem::Ptr();
  }
  void HistoryLogReader::Impl::_oldAPIparseLine( const std::string & line, unsigned int lineNr )
  {
    // parse into fields
    HistoryItem::FieldVector fields;
    str::splitEscaped( line, back_inserter(fields), "|", true );

    if ( fields.size() <= 2 )
    {
      if ( ! _options.testFlag( IGNORE_INVALID_ITEMS ) )
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
      item_ptr = _oldAPIcreateHistoryItem( fields );
    }
    catch ( const Exception & e )
    {
      ZYPP_CAUGHT(e);
      ERR << "Invalid history log entry on line #" << lineNr << " '"<< line << "'" << endl;

      if ( ! _options.testFlag( IGNORE_INVALID_ITEMS ) )
      {
        ParseException newe( str::form( "Error in history log on line #%u.", lineNr ) );
	newe.remember( e );
	ZYPP_THROW( newe );
      }
    }

    if ( item_ptr )
    {
      _oldAPICallback( item_ptr );
    }
    else if ( ! _options.testFlag( IGNORE_INVALID_ITEMS ) )
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
#endif // WITH_DEPRECATED_HISTORYITEM_API

  /////////////////////////////////////////////////////////////////////
  //
  //	class HistoryLogReader
  //
  /////////////////////////////////////////////////////////////////////

#if defined(WITH_DEPRECATED_HISTORYITEM_API)
  HistoryLogReader::HistoryLogReader( const Pathname & historyFile,
                                      const ProcessItem & callback )
  : _pimpl(new HistoryLogReader::Impl( historyFile, Options(), ProcessData() ) )
  { _pimpl->_oldAPICallback = callback; }
#endif // WITH_DEPRECATED_HISTORYITEM_API

  HistoryLogReader::HistoryLogReader( const Pathname & historyFile_r, const Options & options_r, const ProcessData & callback_r )
  : _pimpl( new HistoryLogReader::Impl( historyFile_r, options_r, callback_r ) )
  {}

  HistoryLogReader::~HistoryLogReader()
  {}

  void HistoryLogReader::setIgnoreInvalidItems( bool ignoreInvalid_r )
  { _pimpl->_options.setFlag( IGNORE_INVALID_ITEMS, ignoreInvalid_r ); }

  bool HistoryLogReader::ignoreInvalidItems() const
  { return _pimpl->_options.testFlag( IGNORE_INVALID_ITEMS ); }

  void HistoryLogReader::readAll( const ProgressData::ReceiverFnc & progress_r )
  { _pimpl->readAll( progress_r ); }

  void HistoryLogReader::readFrom( const Date & date_r, const ProgressData::ReceiverFnc & progress_r )
  { _pimpl->readFrom( date_r, progress_r ); }

  void HistoryLogReader::readFromTo( const Date & fromDate_r, const Date & toDate_r, const ProgressData::ReceiverFnc & progress_r )
  { _pimpl->readFromTo( fromDate_r, toDate_r, progress_r ); }

  } // namespace parser
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
