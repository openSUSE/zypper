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
  // CLASS NAME: HistoryLogReader
  //
  /////////////////////////////////////////////////////////////////////

  HistoryLogReader::HistoryLogReader( const Pathname & historyFile,
                                      const ProcessItem & callback )
    : _filename(historyFile), _callback(callback)
  {}

  void HistoryLogReader::readAll(const ProgressData::ReceiverFnc & progress)
  {
    InputStream is(_filename);
    iostr::EachLine line(is);

    ProgressData pd;
    pd.sendTo( progress );
    pd.toMin();

    for (; line; line.next(), pd.tick())
    {
      const string & s = *line;
      if (s[0] == '#') // ignore comments
        continue;

      // determine action
      HistoryItem::FieldVector fields;
      str::splitEscaped(s, back_inserter(fields), "|", true);

      if (fields.size() <= 2)
        ZYPP_THROW(ParseException(
          str::form("Bad number of fields. Got %ld, expected more than %d.",
            fields.size(), 2)));

      // parse into the data structures
      HistoryActionID aid(str::trim(fields[1]));
      try
      {
        switch (aid.toEnum())
        {
        case HistoryActionID::INSTALL_e:
            _callback(HistoryItemInstall::Ptr(new HistoryItemInstall(fields)));
          break;

        case HistoryActionID::REMOVE_e:
          _callback(HistoryItemRemove::Ptr(new HistoryItemRemove(fields)));
          break;

        case HistoryActionID::REPO_ADD_e:
          _callback(HistoryItemRepoAdd::Ptr(new HistoryItemRepoAdd(fields)));
          break;

        case HistoryActionID::REPO_REMOVE_e:
          _callback(HistoryItemRepoRemove::Ptr(new HistoryItemRepoRemove(fields)));
          break;

        case HistoryActionID::REPO_CHANGE_ALIAS_e:
          _callback(HistoryItemRepoAliasChange::Ptr(new HistoryItemRepoAliasChange(fields)));
          break;

        case HistoryActionID::REPO_CHANGE_URL_e:
          _callback(HistoryItemRepoUrlChange::Ptr(new HistoryItemRepoUrlChange(fields)));
          break;

        default:
          WAR << "Unknown history log action type: " << fields[1] << endl;
        }
      }
      catch (const Exception & e)
      {
        ParseException newe(
            str::form("Error in history log on line #%u.", line.lineNo()));
        newe.remember(e);
        ZYPP_THROW(newe);
      }
    }

    pd.toMax();
  }


  HistoryLogReader::~HistoryLogReader()
  {}


    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
