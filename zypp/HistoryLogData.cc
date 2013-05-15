/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

/** \file zypp/HistoryLogData.cc
 *
 */
#include <sstream>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/String.h"
#include "zypp/base/Logger.h"
#include "zypp/parser/ParseException.h"

#include "zypp/HistoryLogData.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{
  using parser::ParseException;

  ///////////////////////////////////////////////////////////////////
  //
  //	class HistoryActionID
  //
  ///////////////////////////////////////////////////////////////////

  const HistoryActionID HistoryActionID::NONE			(HistoryActionID::NONE_e);
  const HistoryActionID HistoryActionID::INSTALL		(HistoryActionID::INSTALL_e);
  const HistoryActionID HistoryActionID::REMOVE			(HistoryActionID::REMOVE_e);
  const HistoryActionID HistoryActionID::REPO_ADD		(HistoryActionID::REPO_ADD_e);
  const HistoryActionID HistoryActionID::REPO_REMOVE		(HistoryActionID::REPO_REMOVE_e);
  const HistoryActionID HistoryActionID::REPO_CHANGE_ALIAS	(HistoryActionID::REPO_CHANGE_ALIAS_e);
  const HistoryActionID HistoryActionID::REPO_CHANGE_URL	(HistoryActionID::REPO_CHANGE_URL_e);

  HistoryActionID::HistoryActionID(const std::string & strval_r)
    : _id(parse(strval_r))
  {}

  HistoryActionID::ID HistoryActionID::parse( const std::string & strval_r )
  {
    typedef std::map<std::string,ID> MapType;
    static MapType _table;
    if ( _table.empty() )
    {
      // initialize it
      _table["install"]	= INSTALL_e;
      _table["remove"]	= REMOVE_e;
      _table["radd"]	= REPO_ADD_e;
      _table["rremove"]	= REPO_REMOVE_e;
      _table["ralias"]	= REPO_CHANGE_ALIAS_e;
      _table["rurl"]	= REPO_CHANGE_URL_e;
      _table["NONE"]	= _table["none"] = NONE_e;
    }

    MapType::const_iterator it = _table.find( strval_r );
    if ( it != _table.end() )
      return it->second;
    // else:
    WAR << "Unknown history action ID '" + strval_r + "'" << endl;
    return NONE_e;
  }


  const std::string & HistoryActionID::asString( bool pad ) const
  {
    typedef std::pair<std::string,std::string> PairType;
    typedef std::map<ID, PairType> MapType;
    static MapType _table;
    if ( _table.empty() )
    {
      // initialize it                                     pad(7) (for now)
      _table[INSTALL_e]           = PairType( "install"	, "install" );
      _table[REMOVE_e]            = PairType( "remove"	, "remove " );
      _table[REPO_ADD_e]          = PairType( "radd"	, "radd   " );
      _table[REPO_REMOVE_e]       = PairType( "rremove"	, "rremove" );
      _table[REPO_CHANGE_ALIAS_e] = PairType( "ralias"	, "ralias " );
      _table[REPO_CHANGE_URL_e]   = PairType( "rurl"	, "rurl   " );
      _table[NONE_e]              = PairType( "NONE"	, "NONE   " );
    }

    return( pad ? _table[_id].second : _table[_id].first );
  }

  std::ostream & operator << (std::ostream & str, const HistoryActionID & id)
  { return str << id.asString(); }

  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	class HistoryLogData::Impl
  //
  ///////////////////////////////////////////////////////////////////
  class HistoryLogData::Impl
  {
  public:
    Impl( FieldVector & fields_r, size_type expect_r )
    {
      _checkFields( fields_r, expect_r );
      _field.swap( fields_r );
      // For whatever reason writer is ' '-padding the action field
      // but we don't want to modify the vector before we moved it.
      _field[ACTION_INDEX] = str::trim( _field[ACTION_INDEX] );
      _action = HistoryActionID( _field[ACTION_INDEX] );
    }

    Impl( FieldVector & fields_r, HistoryActionID action_r, size_type expect_r )
    {
      _checkFields( fields_r, expect_r );
      // For whatever reason writer is ' '-padding the action field
      // but we don't want to modify the vector before we moved it.
      std::string trimmed( str::trim( fields_r[ACTION_INDEX] ) );
      _action = HistoryActionID( trimmed );
      if ( _action != action_r )
      {
	ZYPP_THROW( ParseException( str::form( "Bad action id. Got %s, expected %s.",
					       _action.asString().c_str(),
					       action_r.asString().c_str() ) ) );
      }
      _field.swap( fields_r );
      // now adjust action field:
      _field[ACTION_INDEX] = trimmed;
    }

    void _checkFields( const FieldVector & fields_r, size_type expect_r )
    {
      if ( expect_r < 2 )	// at least 2 fields (date and action) are required
	expect_r = 2;
      if ( fields_r.size() < expect_r )
      {
	ZYPP_THROW( ParseException( str::form( "Bad number of fields. Got %zd, expected at least %zd.",
					       fields_r.size(),
					       expect_r ) ) );
      }
      try
      {
	_date = Date( fields_r[DATE_INDEX], HISTORY_LOG_DATE_FORMAT );
      }
      catch ( const std::exception & excpt )
      {
	ZYPP_THROW( ParseException( excpt.what() ) );	// invalid date format
      }
      // _action handled later
    }

  public:
    FieldVector 	_field;
    Date 		_date;
    HistoryActionID	_action;
  };

  ///////////////////////////////////////////////////////////////////
  //
  //	class HistoryLogData
  //
  ///////////////////////////////////////////////////////////////////

  HistoryLogData::HistoryLogData( FieldVector & fields_r, size_type expect_r )
  : _pimpl( new Impl( fields_r, expect_r ) )
  {}

  HistoryLogData::HistoryLogData( FieldVector & fields_r, HistoryActionID expectedId_r, size_type expect_r )
  : _pimpl( new Impl( fields_r, expectedId_r, expect_r ) )
  {}

  HistoryLogData::~HistoryLogData()
  {}

  HistoryLogData::Ptr HistoryLogData::create( FieldVector & fields_r )
  {
    if ( fields_r.size() >= 2 )
    {
      // str::trim( _field[ACTION_INDEX] );
      switch ( HistoryActionID( str::trim( fields_r[ACTION_INDEX] ) ).toEnum() )
      {
#define OUTS(E,T) case HistoryActionID::E: return Ptr( new T( fields_r ) ); break;
	OUTS( INSTALL_e,		HistoryLogDataInstall );
	OUTS( REMOVE_e,			HistoryLogDataRemove );
	OUTS( REPO_ADD_e,		HistoryLogDataRepoAdd );
	OUTS( REPO_REMOVE_e,		HistoryLogDataRepoRemove );
	OUTS( REPO_CHANGE_ALIAS_e,	HistoryLogDataRepoAliasChange );
	OUTS( REPO_CHANGE_URL_e,	HistoryLogDataRepoUrlChange );
#undef OUTS
	// intentionally no default:
	case HistoryActionID::NONE_e:
	  break;
      }
    }
    // unknown action or invalid fields? Ctor will accept or throw.
    return Ptr( new HistoryLogData( fields_r ) );
  }

  bool HistoryLogData::empty() const
  { return _pimpl->_field.empty(); }

  HistoryLogData::size_type HistoryLogData::size() const
  { return _pimpl->_field.size(); }

  HistoryLogData::const_iterator HistoryLogData::begin() const
  { return _pimpl->_field.begin(); }

  HistoryLogData::const_iterator HistoryLogData::end() const
  { return _pimpl->_field.end(); }

  const std::string & HistoryLogData::optionalAt( size_type idx_r ) const
  {
    static const std::string _empty;
    return( idx_r < size() ? _pimpl->_field[idx_r] : _empty );
  }

  const std::string & HistoryLogData::at( size_type idx_r ) const
  { return _pimpl->_field.at( idx_r ); }


  Date HistoryLogData::date() const
  { return _pimpl->_date; }

  HistoryActionID HistoryLogData::action() const
  { return _pimpl->_action; }


  std::ostream & operator<<( std::ostream & str, const HistoryLogData & obj )
  { return str << str::joinEscaped( obj.begin(), obj.end(), '|' ); }

  ///////////////////////////////////////////////////////////////////
  //	class HistoryLogDataInstall
  ///////////////////////////////////////////////////////////////////
    HistoryLogDataInstall::HistoryLogDataInstall( FieldVector & fields_r )
    : HistoryLogData( fields_r )
    {}
    std::string	HistoryLogDataInstall::name()		const { return optionalAt( NAME_INDEX ); }
    Edition	HistoryLogDataInstall::edition()	const { return Edition( optionalAt( EDITION_INDEX ) ); }
    Arch	HistoryLogDataInstall::arch()		const { return Arch( optionalAt( ARCH_INDEX ) ); }
    std::string	HistoryLogDataInstall::reqby()		const { return optionalAt( REQBY_INDEX ); }
    std::string	HistoryLogDataInstall::repoAlias()	const { return optionalAt( REPOALIAS_INDEX ); }
    CheckSum	HistoryLogDataInstall::checksum()	const { return optionalAt( CHEKSUM_INDEX ); }
    std::string	HistoryLogDataInstall::userdata()	const { return optionalAt( USERDATA_INDEX ); }

  ///////////////////////////////////////////////////////////////////
  //	class HistoryLogDataRemove
  ///////////////////////////////////////////////////////////////////
    HistoryLogDataRemove::HistoryLogDataRemove( FieldVector & fields_r )
    : HistoryLogData( fields_r )
    {}
    std::string	HistoryLogDataRemove::name()		const { return optionalAt( NAME_INDEX ); }
    Edition	HistoryLogDataRemove::edition()		const { return Edition( optionalAt( EDITION_INDEX ) ); }
    Arch	HistoryLogDataRemove::arch()		const { return Arch( optionalAt( ARCH_INDEX ) ); }
    std::string	HistoryLogDataRemove::reqby()		const { return optionalAt( REQBY_INDEX ); }
    std::string	HistoryLogDataRemove::userdata()	const { return optionalAt( USERDATA_INDEX ); }

  ///////////////////////////////////////////////////////////////////
  //	class HistoryLogDataRepoAdd
  ///////////////////////////////////////////////////////////////////
    HistoryLogDataRepoAdd::HistoryLogDataRepoAdd( FieldVector & fields_r )
    : HistoryLogData( fields_r )
    {}
    std::string	HistoryLogDataRepoAdd::alias()		const { return optionalAt( ALIAS_INDEX ); }
    Url		HistoryLogDataRepoAdd::url()		const { return optionalAt( URL_INDEX ); }
    std::string	HistoryLogDataRepoAdd::userdata()	const { return optionalAt( USERDATA_INDEX ); }

  ///////////////////////////////////////////////////////////////////
  //	class HistoryLogDataRepoRemove
  ///////////////////////////////////////////////////////////////////
    HistoryLogDataRepoRemove::HistoryLogDataRepoRemove( FieldVector & fields_r )
    : HistoryLogData( fields_r )
    {}
    std::string	HistoryLogDataRepoRemove::alias()	const { return optionalAt( ALIAS_INDEX ); }
    std::string	HistoryLogDataRepoRemove::userdata()	const { return optionalAt( USERDATA_INDEX ); }

  ///////////////////////////////////////////////////////////////////
  //	class HistoryLogDataRepoAliasChange
  ///////////////////////////////////////////////////////////////////
    HistoryLogDataRepoAliasChange::HistoryLogDataRepoAliasChange( FieldVector & fields_r )
    : HistoryLogData( fields_r )
    {}
    std::string	HistoryLogDataRepoAliasChange::oldAlias()	const { return optionalAt( OLDALIAS_INDEX ); }
    std::string	HistoryLogDataRepoAliasChange::newAlias()	const { return optionalAt( NEWALIAS_INDEX ); }
    std::string	HistoryLogDataRepoAliasChange::userdata()	const { return optionalAt( USERDATA_INDEX ); }

  ///////////////////////////////////////////////////////////////////
  //	class HistoryLogDataRepoUrlChange
  ///////////////////////////////////////////////////////////////////
    HistoryLogDataRepoUrlChange::HistoryLogDataRepoUrlChange( FieldVector & fields_r )
    : HistoryLogData( fields_r )
    {}
    std::string	HistoryLogDataRepoUrlChange::alias()	const { return optionalAt( ALIAS_INDEX ); }
    Url		HistoryLogDataRepoUrlChange::newUrl()	const { return optionalAt( NEWURL_INDEX ); }
    std::string	HistoryLogDataRepoUrlChange::userdata()	const { return optionalAt( USERDATA_INDEX ); }

} // namespace zypp
///////////////////////////////////////////////////////////////////
