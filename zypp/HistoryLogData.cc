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

namespace zypp
{
  using parser::ParseException;


  ///////////////////////////////////////////////////////////////////
  //
  //    CLASS NAME : HistoryActionID
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
    if ( it == _table.end() )
    {
      WAR << "Unknown history action ID '" + strval_r + "'" << endl;
      return NONE_e;
    }
    return it->second;
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


  /////////////////////////////////////////////////////////////////////
  //
  // CLASS NAME: HistoryItem
  //
  /////////////////////////////////////////////////////////////////////

  HistoryItem::HistoryItem(FieldVector & fields)
  {
    if (fields.size() <= 2)
      ZYPP_THROW(ParseException(
        str::form("Bad number of fields. Got %zd, expected more than %d.",
          fields.size(), 2)));

    date = Date(fields[0], HISTORY_LOG_DATE_FORMAT);
    action = HistoryActionID(str::trim(fields[1]));
  }

  void HistoryItem::dumpTo(ostream & str) const
  {
    str << date.form(HISTORY_LOG_DATE_FORMAT) << "|" << action.asString();
  }

  ostream & operator<<(ostream & str, const HistoryItem & obj)
  {
    obj.dumpTo(str);
    return str;
  }


  /////////////////////////////////////////////////////////////////////
  //
  // CLASS NAME: HistoryItemInstall
  //
  /////////////////////////////////////////////////////////////////////

  HistoryItemInstall::HistoryItemInstall(FieldVector & fields)
    : HistoryItem(fields)
  {
    if (fields.size() != 8)
      ZYPP_THROW(ParseException(
        str::form("Bad number of fields. Got %zu, expected %u.",
          fields.size(), 8)));

    name      = fields[2];
    edition   = Edition(fields[3]);
    arch      = Arch(fields[4]);
    reqby     = fields[5];
    repoalias = fields[6];
    checksum  = CheckSum::sha(fields[7]);
  }

  void HistoryItemInstall::dumpTo(ostream & str) const
  {
    HistoryItem::dumpTo(str);
    str << "|"
      << name << "|"
      << edition << "|"
      << arch << "|"
      << reqby << "|"
      << repoalias << "|"
      << checksum;
  }

  ostream & operator<<(ostream & str, const HistoryItemInstall & obj)
  {
    obj.dumpTo(str);
    return str;
  }


  /////////////////////////////////////////////////////////////////////
  //
  // CLASS NAME: HistoryItemRemove
  //
  /////////////////////////////////////////////////////////////////////

  HistoryItemRemove::HistoryItemRemove(FieldVector & fields)
    : HistoryItem(fields)
  {
    if (fields.size() != 6)
      ZYPP_THROW(ParseException(
        str::form("Bad number of fields. Got %zu, expected %u.",
          fields.size(), 6)));

    name      = fields[2];
    edition   = Edition(fields[3]);
    arch      = Arch(fields[4]);
    reqby     = fields[5];
  }

  void HistoryItemRemove::dumpTo(ostream & str) const
  {
    HistoryItem::dumpTo(str);
    str << "|"
      << name << "|"
      << edition << "|"
      << arch << "|"
      << reqby;
  }

  ostream & operator<<(ostream & str, const HistoryItemRemove & obj)
  {
    obj.dumpTo(str);
    return str;
  }


  /////////////////////////////////////////////////////////////////////
  //
  // CLASS NAME: HistoryItemRepoAdd
  //
  /////////////////////////////////////////////////////////////////////

  HistoryItemRepoAdd::HistoryItemRepoAdd(FieldVector & fields)
    : HistoryItem(fields)
  {
    if (fields.size() != 4)
      ZYPP_THROW(ParseException(
        str::form("Bad number of fields. Got %zu, expected %u.",
          fields.size(), 4)));

    alias = fields[2];
    url = Url(fields[3]);
  }

  void HistoryItemRepoAdd::dumpTo(ostream & str) const
  {
    HistoryItem::dumpTo(str);
    str << "|"
      << alias << "|"
      << url;
  }

  ostream & operator<<(ostream & str, const HistoryItemRepoAdd & obj)
  {
    obj.dumpTo(str);
    return str;
  }


  /////////////////////////////////////////////////////////////////////
  //
  // CLASS NAME: HistoryItemRepoRemove
  //
  /////////////////////////////////////////////////////////////////////

  HistoryItemRepoRemove::HistoryItemRepoRemove(FieldVector & fields)
    : HistoryItem(fields)
  {
    if (fields.size() != 3)
      ZYPP_THROW(ParseException(
        str::form("Bad number of fields. Got %zu, expected %u.",
          fields.size(), 3)));

    alias = fields[2];
  }

  void HistoryItemRepoRemove::dumpTo(ostream & str) const
  {
    HistoryItem::dumpTo(str);
    str << "|" << alias;
  }

  ostream & operator<<(ostream & str, const HistoryItemRepoRemove & obj)
  {
    obj.dumpTo(str);
    return str;
  }


  /////////////////////////////////////////////////////////////////////
  //
  // CLASS NAME: HistoryItemRepoAliasChange
  //
  /////////////////////////////////////////////////////////////////////

  HistoryItemRepoAliasChange::HistoryItemRepoAliasChange(FieldVector & fields)
    : HistoryItem(fields)
  {
    if (fields.size() != 4)
      ZYPP_THROW(ParseException(
        str::form("Bad number of fields. Got %zu, expected %u.",
          fields.size(), 4)));

    oldalias = fields[2];
    newalias = fields[3];
  }

  void HistoryItemRepoAliasChange::dumpTo(ostream & str) const
  {
    HistoryItem::dumpTo(str);
    str << "|" << oldalias << "|" << newalias;
  }

  ostream & operator<<(ostream & str, const HistoryItemRepoAliasChange & obj)
  {
    obj.dumpTo(str);
    return str;
  }


  /////////////////////////////////////////////////////////////////////
  //
  // CLASS NAME: HistoryItemRepoUrlChange
  //
  /////////////////////////////////////////////////////////////////////

  HistoryItemRepoUrlChange::HistoryItemRepoUrlChange(FieldVector & fields)
    : HistoryItem(fields)
  {
    if (fields.size() != 4)
      ZYPP_THROW(ParseException(
        str::form("Bad number of fields. Got %zu, expected %u.",
          fields.size(), 4)));

    alias = fields[2];
    newurl = Url(fields[3]);
  }

  void HistoryItemRepoUrlChange::dumpTo(ostream & str) const
  {
    HistoryItem::dumpTo(str);
    str << "|" << alias << "|" << newurl;
  }

  ostream & operator<<(ostream & str, const HistoryItemRepoUrlChange & obj)
  {
    obj.dumpTo(str);
    return str;
  }


}
