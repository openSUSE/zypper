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
   * Reads a zypp history log file and calls the ProcessItem function passed
   * in the constructor for each item found.
   *
   * Example:
   * <code>
   *
   * struct HistoryItemCollector
   * {
   *   vector<HistoryItem::Ptr> items;
   *
   *   bool processEntry( const HistoryItem::Ptr & item_ptr )
   *   {
   *     items.push_back(item_ptr);
   *     return true;
   *   }
   * }
   *
   * ...
   *
   * HistoryItemCollector ic;
   * HistoryLogReader reader("/var/log/zypp/history", boost::ref(ic));
   *
   * try
   * {
   *   reader.readAll();
   * }
   * catch (const Exception & e)
   * {
   *   cout << e.asUserHistory() << endl;
   * }
   *
   * </code>
   *
   * \see http://en.opensuse.org/Libzypp/Package_History
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
      const ProgressData::ReceiverFnc &progress = ProgressData::ReceiverFnc() );

    /**
     * Read log from specified \a date.
     */
    void readFrom( const Date & date,
      const ProgressData::ReceiverFnc &progress = ProgressData::ReceiverFnc() );

    /**
     * Read log between \a fromDate and \a toDate.
     */
    void readFromTo( const Date & fromDate, const Date & toDate,
      const ProgressData::ReceiverFnc &progress = ProgressData::ReceiverFnc() );

    /**
     * Set the reader to ignore invalid log entries and continue with the rest.
     *
     * \param ignoreInvalid <tt>true</tt> will cause the reader to ignore invalid entries
     */
    void setIgnoreInvalidItems( bool ignoreInvalid = false );

    /**
     * Whether the reader is set to ignore invalid log entries.
     */
    bool ignoreInvalidItems() const;

  private:
    /** Implementation */
    class Impl;
    RW_pointer<Impl,rw_pointer::Scoped<Impl> > _pimpl;
  };
  ///////////////////////////////////////////////////////////////////


  /////////////////////////////////////////////////////////////////
} // namespace parser
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

#endif /* ZYPP_HISTORYLOGREADER_H_ */
