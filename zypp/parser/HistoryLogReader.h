/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

/** \file zypp/parser/HistoryLogReader.h
 *
 */
#ifndef ZYPP_PARSER_HISTORYLOGREADER_H_
#define ZYPP_PARSER_HISTORYLOGREADER_H_

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

  ///////////////////////////////////////////////////////////////////
  /// \class HistoryLogReader
  /// \brief Zypp history file parser
  /// \ingroup g_ZyppHistory
  /// \ingroup g_ZyppParser
  ///
  /// Reads a zypp history log file and calls the ProcessItem function
  /// passed in the constructor for each item read.
  ///
  /// \code
  /// struct HistoryItemCollector
  /// {
  ///   vector<HistoryItem::Ptr> items;
  ///
  ///   bool operator()( const HistoryItem::Ptr & item_ptr )
  ///   {
  ///     items.push_back(item_ptr);
  ///     return true;
  ///   }
  /// }
  /// ...
  /// HistoryItemCollector ic;
  /// HistoryLogReader reader("/var/log/zypp/history", boost::ref(ic));
  ///
  /// try
  /// {
  ///   reader.readAll();
  /// }
  /// catch (const Exception & e)
  /// {
  ///   cout << e.asUserHistory() << endl;
  /// }
  /// \endcode
  /////////////////////////////////////////////////////////////////////
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
     *
     * \param progress An optional progress data receiver function.
     */
    void readAll(
      const ProgressData::ReceiverFnc &progress = ProgressData::ReceiverFnc() );

    /**
     * Read log from specified \a date.
     *
     * \param date     Date from which to read.
     * \param progress An optional progress data receiver function.
     *
     * \see readFromTo()
     */
    void readFrom( const Date & date,
      const ProgressData::ReceiverFnc &progress = ProgressData::ReceiverFnc() );

    /**
     * Read log between \a fromDate and \a toDate.
     *
     * The date comparison's precision goes to seconds. Omitted time parts
     * get replaced by zeroes, so if e.g. the time is not specified at all, the
     * date means midnight of the specified date. So
     *
     * <code>
     * fromDate = Date("2009-01-01", "%Y-%m-%d");
     * toDate   = Date("2009-01-02", "%Y-%m-%d");
     * </code>
     *
     * will yield log entries from midnight of January, 1st untill
     * one second before midnight of January, 2nd.
     *
     * \param fromDate Date from which to read.
     * \param toDate   Date on which to stop reading.
     * \param progress An optional progress data receiver function.
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
     *
     * \see setIngoreInvalidItems()
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

#endif /* ZYPP_PARSER_HISTORYLOGREADER_H_ */
