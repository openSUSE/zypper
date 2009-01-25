/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

/** \file HistoryLogReader.h
 *
 */
#ifndef ZYPP_HISTORYLOGREADER_H_
#define ZYPP_HISTORYLOGREADER_H_

#include "zypp/base/PtrTypes.h"
#include "zypp/ProgressData.h"
#include "zypp/Pathname.h"

#include "zypp/HistoryLogData.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Date;

  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////


  /////////////////////////////////////////////////////////////////////
  //
  // CLASS NAME: HistoryLogReader
  //
  /**
   *
   */
  class HistoryLogReader
  {
  public:
    typedef function< bool( const HistoryItem::Ptr & )> ProcessItem;

  public:
    HistoryLogReader( const Pathname & repo_file,
                      const ProcessItem & callback );
    ~HistoryLogReader();

    /**
     * Read the whole log file.
     */
    void readAll(
        const ProgressData::ReceiverFnc &progress = ProgressData::ReceiverFnc());

    /*
     * readFrom(Date);
     * readFromTo(Date, Date);
     */

  private:
    Pathname _filename;
    ProcessItem _callback;
  };


  /////////////////////////////////////////////////////////////////
} // namespace parser
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

#endif /* ZYPP_HISTORYLOGREADER_H_ */
