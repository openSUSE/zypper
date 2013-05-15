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
#include "zypp/base/Flags.h"
#include "zypp/ProgressData.h"
#include "zypp/Pathname.h"

#include "zypp/HistoryLogData.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{

  class Date;

  ///////////////////////////////////////////////////////////////////
  namespace parser
  {

  ///////////////////////////////////////////////////////////////////
  /// \class HistoryLogReader
  /// \brief Zypp history file parser
  /// \ingroup g_ZyppHistory
  /// \ingroup g_ZyppParser
  ///
  /// Reads a zypp history log file and calls the \ref ProcessData callback
  /// passed in the constructor for each valid history line read. The callbacks
  /// return value indicates whether to continue parsing.
  ///
  /// \code
  ///   std::vector<HistoryLogData::Ptr> history;
  ///   parser::HistoryLogReader parser( ZConfig::instance().historyLogFile(),
  ///                                    HistoryLogReader::Options(),
  ///     [&history]( HistoryLogData::Ptr ptr )->bool {
  ///       history.push_back( ptr );
  ///       return true;
  ///     } );
  ///   parser.readAll();
  ///   ...
  ///   if ( history[0]->action() == HistoryActionID::INSTALL )
  ///   {
  ///     // generic access to data fields plain string values:
  ///     MIL << (*p)[HistoryLogDataInstall::USERDATA_INDEX] << endl;
  ///
  ///     // The same maybe more convenient though derived classes:
  ///     HistoryLogDataInstall::Ptr ip( dynamic_pointer_cast<HistoryLogDataInstall>( p ) );
  ///     MIL << ip->userdata() << endl;
  ///   }
  /// \endcode
  /// \see \ref HistoryLogData for how to access the individual data fields.
  ///
  ///////////////////////////////////////////////////////////////////
  class HistoryLogReader
  {
  public:

    enum OptionBits	///< Parser option flags
    {
      IGNORE_INVALID_ITEMS	= (1 << 0)	///< ignore invalid items and continue parsing
    };
    ZYPP_DECLARE_FLAGS( Options, OptionBits );

  public:
    /** Callback type to consume a single history line split into fields.
     * The return value indicates whether to continue parsing.
     */
    typedef function< bool( const HistoryLogData::Ptr & )> ProcessData;

    /** Ctor taking file to parse and data consumer callback.
     * As \a options_r argument pass \c HistoryLogReader::Options() to
     * use the default stettings, or an OR'ed combination of \ref OptionBits.
     */
    HistoryLogReader( const Pathname & historyFile_r, const Options & options_r, const ProcessData & callback_r );

    ~HistoryLogReader();

    /**
     * Read the whole log file.
     *
     * \param progress An optional progress data receiver function.
     */
    void readAll( const ProgressData::ReceiverFnc & progress = ProgressData::ReceiverFnc() );

    /**
     * Read log from specified \a date.
     *
     * \param date     Date from which to read.
     * \param progress An optional progress data receiver function.
     *
     * \see readFromTo()
     */
    void readFrom( const Date & date, const ProgressData::ReceiverFnc & progress = ProgressData::ReceiverFnc() );

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
    void readFromTo( const Date & fromDate, const Date & toDate, const ProgressData::ReceiverFnc & progress = ProgressData::ReceiverFnc() );

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

  /** \relates HistoryLogReader::Options */
  ZYPP_DECLARE_OPERATORS_FOR_FLAGS( HistoryLogReader::Options );

  ///////////////////////////////////////////////////////////////////

  } // namespace parser
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

#endif /* ZYPP_PARSER_HISTORYLOGREADER_H_ */
