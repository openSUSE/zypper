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

  static std::map<std::string,HistoryActionID::ID> _table;

  const HistoryActionID HistoryActionID::NONE(HistoryActionID::NONE_e);
  const HistoryActionID HistoryActionID::INSTALL(HistoryActionID::INSTALL_e);
  const HistoryActionID HistoryActionID::REMOVE(HistoryActionID::REMOVE_e);
  const HistoryActionID HistoryActionID::REPO_ADD(HistoryActionID::REPO_ADD_e);
  const HistoryActionID HistoryActionID::REPO_REMOVE(HistoryActionID::REPO_REMOVE_e);
  const HistoryActionID HistoryActionID::REPO_CHANGE_ALIAS(HistoryActionID::REPO_CHANGE_ALIAS_e);
  const HistoryActionID HistoryActionID::REPO_CHANGE_URL(HistoryActionID::REPO_CHANGE_URL_e);

  HistoryActionID::HistoryActionID(const std::string & strval_r)
    : _id(parse(strval_r))
  {}

  HistoryActionID::ID HistoryActionID::parse(const std::string & strval_r)
  {
    if (_table.empty())
    {
      // initialize it
      _table["install"] = INSTALL_e;
      _table["remove"]  = REMOVE_e;
      _table["radd"]    = REPO_ADD_e;
      _table["rremove"] = REPO_REMOVE_e;
      _table["ralias"]  = REPO_CHANGE_ALIAS_e;
      _table["rurl"]    = REPO_CHANGE_URL_e;
      _table["NONE"] = _table["none"] = HistoryActionID::NONE_e;
    }

    std::map<std::string,HistoryActionID::ID>::const_iterator it =
      _table.find(strval_r);

    if (it == _table.end())
      WAR << "Unknown history action ID '" + strval_r + "'";

    return it->second;
  }


  const std::string & HistoryActionID::asString(bool pad) const
  {
    static std::map<ID, std::string> _table;
    if ( _table.empty() )
    {
      // initialize it
      _table[INSTALL_e]           = "install";
      _table[REMOVE_e]            = "remove";
      _table[REPO_ADD_e]          = "radd";
      _table[REPO_REMOVE_e]       = "rremove";
      _table[REPO_CHANGE_ALIAS_e] = "ralias";
      _table[REPO_CHANGE_URL_e]   = "rurl";
      _table[NONE_e]              = "NONE";
    }
    // add spaces so that the size of the returned string is always 7 (for now)
    if (pad)
      return _table[_id].append(7 - _table[_id].size(), ' ');
    return _table[_id];
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
        str::form("Bad number of fields. Got %ld, expected more than %d.",
          fields.size(), 2)));

    date = Date(fields[0], HISTORY_LOG_DATE_FORMAT);
    action = HistoryActionID(str::trim(fields[1]));
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
        str::form("Bad number of fields. Got %lu, expected %u.",
          fields.size(), 8)));

    name      = fields[2];
    edition   = Edition(fields[3]);
    arch      = Arch(fields[4]);
    reqby     = fields[5];
    repoalias = fields[6];
    checksum  = CheckSum::sha(fields[7]);
  }

  const std::string HistoryItemInstall::asString() const
  {
    ostringstream str;
    str
      << date.form(HISTORY_LOG_DATE_FORMAT) << "|"
      << action.asString() << "|"
      << name << "|"
      << edition << "|"
      << arch << "|"
      << reqby << "|"
      << repoalias << "|"
      << checksum;
    return str.str();
  }

  std::ostream & operator<<(std::ostream & str, const HistoryItemInstall & obj)
  {
    return str << obj.asString();
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
        str::form("Bad number of fields. Got %lu, expected %u.",
          fields.size(), 6)));

    name      = fields[2];
    edition   = Edition(fields[3]);
    arch      = Arch(fields[4]);
    reqby     = fields[5];
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
        str::form("Bad number of fields. Got %lu, expected %u.",
          fields.size(), 4)));

    alias = fields[2];
    url = Url(fields[3]);
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
        str::form("Bad number of fields. Got %lu, expected %u.",
          fields.size(), 3)));

    alias = fields[2];
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
        str::form("Bad number of fields. Got %lu, expected %u.",
          fields.size(), 4)));

    oldalias = fields[2];
    newalias = fields[3];
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
        str::form("Bad number of fields. Got %lu, expected %u.",
          fields.size(), 4)));

    alias = fields[2];
    newurl = Url(fields[3]);
  }


}
