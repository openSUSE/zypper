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

    void readAll(const ProgressData::ReceiverFnc & progress);

    Pathname _filename;
    ProcessItem _callback;
    bool _ignoreInvalid;
  };

  HistoryLogReader::Impl::Impl( const Pathname & historyFile,
                                const ProcessItem & callback )
    : _filename(historyFile), _callback(callback), _ignoreInvalid(false)
  {}

  HistoryItem::Ptr
  HistoryLogReader::Impl::createHistoryItem(HistoryItem::FieldVector & fields)
  {
    HistoryActionID aid(str::trim(fields[1]));
    switch (aid.toEnum())
    {
    case HistoryActionID::INSTALL_e:
        return HistoryItemInstall::Ptr(new HistoryItemInstall(fields));
      break;

    case HistoryActionID::REMOVE_e:
      return HistoryItemRemove::Ptr(new HistoryItemRemove(fields));
      break;

    case HistoryActionID::REPO_ADD_e:
      return HistoryItemRepoAdd::Ptr(new HistoryItemRepoAdd(fields));
      break;

    case HistoryActionID::REPO_REMOVE_e:
      return HistoryItemRepoRemove::Ptr(new HistoryItemRepoRemove(fields));
      break;

    case HistoryActionID::REPO_CHANGE_ALIAS_e:
      return HistoryItemRepoAliasChange::Ptr(new HistoryItemRepoAliasChange(fields));
      break;

    case HistoryActionID::REPO_CHANGE_URL_e:
      return HistoryItemRepoUrlChange::Ptr(new HistoryItemRepoUrlChange(fields));
      break;

    default:
      WAR << "Unknown history log action type: " << fields[1] << endl;
    }

    return HistoryItem::Ptr();
  }

  void HistoryLogReader::Impl::readAll(const ProgressData::ReceiverFnc & progress)
  {
    InputStream is(_filename);
    iostr::EachLine line(is);

    ProgressData pd;
    pd.sendTo( progress );
    pd.toMin();

    HistoryItem::FieldVector fields;
    HistoryItem::Ptr item_ptr;
    for (; line; line.next(), pd.tick(), fields.clear(), item_ptr.reset())
    {
      const string & s = *line;
      if (s[0] == '#') // ignore comments
        continue;

      // parse fields
      str::splitEscaped(s, back_inserter(fields), "|", true);

      if (fields.size() <= 2)
        ZYPP_THROW(ParseException(
          str::form("Bad number of fields. Got %ld, expected more than %d.",
            fields.size(), 2)));

      try
      {
        item_ptr = createHistoryItem(fields);
      }
      catch (const Exception & e)
      {
        ZYPP_CAUGHT(e);
        ERR << "Invalid history log entry on line #" << line.lineNo() << ":" << endl
            << s << endl;

        if (!_ignoreInvalid)
        {
          ParseException newe(
              str::form("Error in history log on line #%u.", line.lineNo() ) );
          newe.remember(e);
          ZYPP_THROW(newe);
        }
      }

      if (item_ptr)
        _callback(item_ptr);
      else if (!_ignoreInvalid)
      {
        ParseException
          e(str::form("Error in history log on line #%u.", line.lineNo()));
        e.addHistory("Unknown entry type.");
        ZYPP_THROW(e);
      }
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


    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
