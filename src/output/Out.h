#ifndef OUT_H_
#define OUT_H_

#include <string>
#include <boost/format.hpp>

#include <zypp/base/Xml.h>
#include <zypp/base/NonCopyable.h>
#include <zypp/base/Exception.h>
#include <zypp/base/String.h>
#include <zypp/base/Flags.h>
#include <zypp/base/DefaultIntegral.h>
#include <zypp/Url.h>
#include <zypp/TriBool.h>
#include <zypp/ProgressData.h>

#include "utils/prompt.h"
#include "output/prompt.h"

using namespace zypp;

class Table;
class Zypper;

// Too simple on small terminals as esc-sequences may get truncated.
// A table like writer for attributed strings is desirable.
struct TermLine
{
  enum SplitFlag
  {
    SF_CRUSH	= 1<<0,	//< truncate lhs, then rhs
    SF_SPLIT	= 1<<1,	//< split line across two
    SF_EXPAND	= 1<<2	//< expand short lines iff stdout is a tty
  };
  ZYPP_DECLARE_FLAGS( SplitFlags, SplitFlag );

  TermLine( SplitFlags flags_r, char exp_r ) : flagsHint( flags_r ), expHint( exp_r ) {}
  TermLine( SplitFlags flags_r ) : flagsHint( flags_r ) {}
  TermLine( char exp_r ) : expHint( exp_r ) {}
  TermLine() {}

  SplitFlags flagsHint;				//< flags to use if not passed to \ref get
  zypp::DefaultIntegral<char,' '> expHint;	//< expand char to use if not passed to \ref get
  zypp::DefaultIntegral<int,-1> percentHint;	//< draw progress indicator in expanded space if in [0,100]

  zypp::str::Str lhs;				//< left side
  zypp::str::Str rhs;				//< right side
  zypp::DefaultIntegral<unsigned,0> lhidden;	//< size of embedded esc sequences
  zypp::DefaultIntegral<unsigned,0> rhidden;	//< size of embedded esc sequences


  /** Return plain line made of lhs + rhs */
  std::string get() const
  { return std::string(lhs) + std::string(rhs); }

  /** Return line optionally formated according to \a width_r and \a flags_r.
   * If \a width_r or \a flags_r is zero a plain line made of lhs + rhs is returned.
   */
  std::string get( unsigned width_r, SplitFlags flags_r, char exp_r ) const;
  /** \overload */
  std::string get( unsigned width_r, SplitFlags flags_r ) const
  { return get( width_r, flags_r, expHint ); }
  /** \overload */
  std::string get( unsigned width_r, char exp_r ) const
  { return get( width_r, flagsHint, exp_r ); }
  /** \overload */
  std::string get( unsigned width_r ) const
  { return get( width_r, flagsHint, expHint ); }
};
ZYPP_DECLARE_OPERATORS_FOR_FLAGS( TermLine::SplitFlags );

/**
 * Base class for producing common (for now) zypper output.
 *
 * This is an abstract class providing interface for writing output like
 * info messages, warnings, error messages, user prompts, progress reports,
 * and download progress reports. See descriptions of the methods for more
 * details.
 *
 * The output is produced using Out derived class instances.
 *
 * <code>
 *
 * // create output object
 * SomePointerType<Out> out;
 * if (options.count("xmlout"))
 *   out = new OutXML();
 * else
 *   out = new OutNormal();
 *
 * out->info("output instance ready to use", Out::HIGH);
 * out->info("Doing foo");
 * try
 * {
 *   foo();
 *   out->prompt(PROMPT_FOO, "Need your action?", "y/n"); // see output/prompt.h
 *   if (action())
 *     out->info("result", Out::QUIET);                // always show the result
 *   else
 *     cout << "special result" << endl; // special output must be done
 *                                       // the usual way for now
 * }
 * catch(const zypp::Exception & e)
 * {
 *   out->error(e, "Problem doing foo", "Do 'bar' to deal with this");
 * }
 *
 * </code>
 */
class Out : private zypp::base::NonCopyable
{
public:
  /** Verbosity levels. */
  typedef enum
  {
    /** Only important messages (no progress or status, only the result). */
    QUIET  = 0,
    /** Default output verbosity level. Progress for important tasks, moderate
     * amount of status messages, operation information, result. */
    NORMAL = 1,
    /** More detailed description of the operations. */
    HIGH   = 2,
    /** \todo drop this level in favor of zypper.log? */
    DEBUG  = 3
  } Verbosity;

  /** Known output types implemented by derived classes. */
  typedef enum
  {
    TYPE_NORMAL = 1,
    TYPE_XML    = 2,
    TYPE_ALL    = 0xff
  } Type;

public:
  Out(Type type, Verbosity verbosity = NORMAL)
    : _verbosity(verbosity), _type(type)
  {}
  virtual ~Out();

public:
  /**
   * Show an info message.
   *
   * \param msg       The message to be displayed.
   * \param verbosity Minimal level o verbosity in which the message will be
   *                  shown. Out::QUIET means the message will be always be
   *                  displayed. Out::HIGH means the message will be displayed
   *                  only if the current verbosity level is HIGH (-v) or DEBUG
   *                  (-vv).
   * \param mask      Determines the types of output for which is this message
   *                  intended. By default, the message will be shown in all
   *                  types of output.
   */
  virtual void info(const std::string & msg, Verbosity verbosity = NORMAL, Type mask = TYPE_ALL) = 0;
  /** \overload taking boost::format */
  void info( const boost::format & msg, Verbosity verbosity = NORMAL, Type mask = TYPE_ALL )
  { info( msg.str(), verbosity, mask ); }
  /** \overload concatenating 2 strings (e.g. translated and untranslated parts) */
  void info( std::string msg, const std::string & msg2, Verbosity verbosity = NORMAL, Type mask = TYPE_ALL )
  { info( (msg+=msg2), verbosity, mask ); }
  /** \overload concatenating 2 strings (e.g. translated and untranslated parts) */
  void info( const boost::format & msg, const std::string & msg2, Verbosity verbosity = NORMAL, Type mask = TYPE_ALL )
  { info( msg.str(), msg2, verbosity, mask ); }

  /** \ref info taking a \ref TermLine */
  virtual void infoLine(const TermLine & msg_r, Verbosity verbosity_r = NORMAL, Type mask_r = TYPE_ALL)
  { info( msg_r.get(), verbosity_r, mask_r ); }

  /**
   * Show a warning.
   *
   * \param msg       The warning message to be displayed.
   * \param verbosity Minimal level o verbosity in which the message will be
   *                  shown. Out::QUIET means the message will be always be
   *                  displayed. Out::HIGH means the message will be displayed
   *                  only if the current verbosity level is HIGH (-v) or DEBUG
   *                  (-vv).
   * \param mask      Determines the types of output for which is this message
   *                  intended. By default, the message will be shown in all
   *                  types of output.
   */
  virtual void warning(const std::string & msg, Verbosity verbosity = NORMAL, Type mask = TYPE_ALL) = 0;
  void warning( const boost::format & msg, Verbosity verbosity = NORMAL, Type mask = TYPE_ALL )
  { warning( msg.str(), verbosity, mask ); }

  /** Convenience class for error reporting. */
  class Error;

  /**
   * Show an error message and an optional hint.
   *
   * An error message should be shown regardless of the verbosity level.
   *
   * \param problem_desc Problem description (what happend)
   * \param hint         Hint for the user (what to do, or explanation)
   */
  virtual void error(const std::string & problem_desc, const std::string & hint = "") = 0;
  void error( const boost::format & problem_desc, const std::string & hint = "")
  { error( problem_desc.str(), hint ); }

  /**
   * Prints the problem description caused by an exception, its cause and,
   * optionaly, a hint for the user.
   *
   * \param e Exception which caused the problem.
   * \param Problem description for the user.
   * \param Hint for the user how to cope with the problem.
   */
  virtual void error(const zypp::Exception & e,
                     const std::string & problem_desc,
                     const std::string & hint = "") = 0;

  //! \todo provide an error() method with a/r/i prompt, more read_action_ari here

  /** \name Progress of an operation. */
  //@{

  /** Convenience class for progress output. */
  class ProgressBar;

  /**
   * Start of an operation with reported progress.
   *
   * \param id      Identifier. Any string used to match multiple overlapping
   *                progress reports (doesn't happen now,
   *                but probably will in the future).
   * \param label   Progress description.
   * \param is_tick <tt>false</tt> for known progress percentage, <tt>true</tt>
   *                for 'still alive' notifications
   */
  virtual void progressStart(const std::string & id,
                             const std::string & label,
                             bool is_tick = false) = 0;

  /**
   * Progress report for an on-going operation.
   *
   * \param id      Identifier. Any string used to match multiple overlapping
   *                progress reports.
   * \param label   Progress description.
   * \param value   Percentage value or <tt>-1</tt> if unknown ('still alive'
   *                notification)
   */
  virtual void progress(const std::string & id,
                        const std::string & label,
                        int value = -1) = 0;

  /**
   * End of an operation with reported progress.
   *
   * \param id      Identifier. Any string used to match multiple overlapping
   *                progress reports.
   * \param label   Progress description.
   * \param error   <tt>false</tt> if the operation finished with success,
   *                <tt>true</tt> otherwise.
   */
  virtual void progressEnd(const std::string & id,
                           const std::string & label,
                           bool error = false) = 0; // might be a string with error message instead
  //@}

  /** \name Download progress with download rate */
  //@{
  /**
   * Reoprt start of a download.
   *
   * \param uri   Uri of the file to download.
   */
  virtual void dwnldProgressStart(const zypp::Url & uri) = 0;

  /**
   * Reports download progress.
   *
   * \param uri   Uri of the file being downloaded.
   * \param value Value of the progress in percents. -1 if unknown.
   * \param rate  Current download rate in B/s. -1 if unknown.
   */
  virtual void dwnldProgress(const zypp::Url & uri,
                             int value = -1,
                             long rate = -1) = 0;
  /**
   * Reports end of a download.
   *
   * \param uri   Uri of the file to download.
   * \param rate  Average download rate at the end. -1 if unknown.
   * \param error Error flag - did the download finish with error?
   */
  virtual void dwnldProgressEnd(const zypp::Url & uri,
                                long rate = -1,
                                bool error = false) = 0;
  //@}

  /**
   * Print out a search result.
   *
   * Default implementation prints \a table_r on \c stdout.
   *
   * \param table_r Table containing the search result.
   *
   * \todo Using a more generic format than a Table is desired.
   */
  virtual void searchResult( const Table & table_r );

  /**
   * Prompt the user for a decision.
   *
   * \param id           Unique prompt identifier for use by machines.
   * \param prompt       Prompt text.
   * \param options      A PromptOptions object
   * \param startdesc    Initial detailed description of the prompt to be
   *                     prepended to the \a prompt text. Should be used
   *                     only whe prompting for the first time and left empty
   *                     when retrying after an invalid answer has been given.
   * \see prompt.h
   * \see ../zypper-prompt.h
   */
  virtual void prompt(PromptId id,
                      const std::string & prompt,
                      const PromptOptions & poptions,
                      const std::string & startdesc = "") = 0;

  /**
   * Print help for prompt, if available.
   * This method should be called after '?' prompt option has been entered.
   */
  virtual void promptHelp(const PromptOptions & poptions) = 0;

public:
  /** Get current verbosity. */
  Verbosity verbosity() { return _verbosity; }

  /** Set current verbosity. */
  void setVerbosity(Verbosity verbosity) { _verbosity = verbosity; }

  /** Return the type of the instance. */
  Type type() { return _type; }

protected:
  /**
   * Determine whether the output is intended for the particular type.
   */
  virtual bool mine(Type type) = 0;

  /**
   * Determine whether to show progress.
   *
   * \return <tt>true</tt> if the progress should be filtered out,
   *         <tt>false</tt> if it should be shown.
   */
  virtual bool progressFilter();

  /**
   * Return a zypp::Exception as a string suitable for output.
   */
  virtual std::string zyppExceptionReport(const zypp::Exception & e);

private:
  Verbosity _verbosity;
  Type      _type;
};

///////////////////////////////////////////////////////////////////
/// \class Out::ProgressBar
/// \brief Convenience class for progress output.
///
/// Progress start and end messages are provided upon object
/// construction and deletion. Progress data are sent through a
/// zypp::ProgressData object accessible via \ref operator->.
///
/// \code
///    {
///      Out::ProgressBar report( _zypper.out(), "Prepare action" );
///      for ( unsigned i = 0; i < 10; ++ i )
///      {
///        report->tick();	// turn wheel
///        sleep(1);
///      }
///      report->range( 10 );	// switch to percent mode [0,10]
///      report.print( "Running action" );
///      for ( unsigned i = 0; i < 10; ++ i )
///      {
///        report->
///        report->set( i );	// send 0%, 10%, ...
///        sleep(1);
///      }
///      // report.error( "Action failed" );
///    }
/// \endcode
///
/// If non zero values for \a current_r or \a total_r are passed to
/// the ctor, the label is prefixed by either "(#C)" or "(#C/#T)"
///
/// \todo ProgressData provides NumericId which might be used as
/// id for_out.progress*().
///////////////////////////////////////////////////////////////////
class Out::ProgressBar : private zypp::base::NonCopyable
{
public:
  /** Indicator type for ctor not drawing an initial start bar. */
  struct NoStartBar {};
  /** Indicator argument for ctor not drawing an initial start bar.*/
  static constexpr NoStartBar noStartBar = NoStartBar();

public:
  /** Ctor not displaying an initial progress bar.
   * If non zero values for \a current_r or \a total_r are passed,
   * the label is prefixed by either "(#C)" or "(#C/#T)"
   */
  ProgressBar( Out & out_r, NoStartBar, const std::string & progressId_r, const std::string & label_r, unsigned current_r = 0, unsigned total_r = 0 )
    : _out( out_r )
    , _error( indeterminate )
    , _progressId( progressId_r )
  {
    if ( total_r )
      _labelPrefix = zypp::str::form( "(%*u/%u) ", numDigits( total_r ), current_r, total_r );
    else if ( current_r )
      _labelPrefix = zypp::str::form( "(%u) ", current_r );
    _progress.name( label_r );
    _progress.sendTo( Print( *this ) );
  }
  ProgressBar( Out & out_r, NoStartBar, const std::string & progressId_r, const boost::format & label_r, unsigned current_r = 0, unsigned total_r = 0 )
  : ProgressBar( out_r, noStartBar, progressId_r, label_r.str(), current_r, total_r )
  {}

  ProgressBar( Out & out_r,NoStartBar,  const std::string & label_r, unsigned current_r = 0, unsigned total_r = 0 )
  : ProgressBar( out_r, noStartBar, "", label_r, current_r, total_r )
  {}

  ProgressBar( Out & out_r, NoStartBar, const boost::format & label_r, unsigned current_r = 0, unsigned total_r = 0 )
  : ProgressBar( out_r, noStartBar, "", label_r.str(), current_r, total_r )
  {}


  /** Ctor displays initial progress bar.
   * If non zero values for \a current_r or \a total_r are passed,
   * the label is prefixed by either "(#C)" or "(#C/#T)"
   */
  ProgressBar( Out & out_r, const std::string & progressId_r, const std::string & label_r, unsigned current_r = 0, unsigned total_r = 0 )
  : ProgressBar( out_r, noStartBar, progressId_r, label_r, current_r, total_r )
  {
    // print the initial progress bar
    _out.progressStart( _progressId, outLabel( _progress.name() ) );
  }

  ProgressBar( Out & out_r, const std::string & progressId_r, const boost::format & label_r, unsigned current_r = 0, unsigned total_r = 0 )
  : ProgressBar( out_r, progressId_r, label_r.str(), current_r, total_r )
  {}

  ProgressBar( Out & out_r, const std::string & label_r, unsigned current_r = 0, unsigned total_r = 0 )
  : ProgressBar( out_r, "", label_r, current_r, total_r )
  {}

  ProgressBar( Out & out_r, const boost::format & label_r, unsigned current_r = 0, unsigned total_r = 0 )
  : ProgressBar( out_r, "", label_r.str(), current_r, total_r )
  {}

  /** Dtor displays final progress bar.
    * Unless \ref error has explicitly been set, an error is indicated if
    * a \ref ProgressData range has been set, but 100% were not reached.
    */
  ~ProgressBar()
  {
    _progress.noSend();	// suppress ~ProgressData final report
    if ( indeterminate( _error ) )
      _error = ( _progress.reportValue() != 100 && _progress.reportPercent() );
    _out.progressEnd( _progressId, outLabel( _progress.name() ), _error );
  }

  /** Immediately print the progress bar not waiting for a new trigger. */
  void print()
  { _out.progress( _progressId, outLabel( _progress.name() ), _progress.reportValue() ); }

  /** \overload also change the progress bar label. */
  void print( const std::string & label_r )
  { _progress.name( label_r ); print(); }

  /** Explicitly indicate the error condition for the final progress bar. */
  void error( tribool error_r = true )
  { _error = error_r; }

  /** \overload to disambiguate. */
  void error( bool error_r )
  { _error = error_r; }

  /** \overload also change the progress bar label. */
  void error( const std::string & label_r )
  { _progress.name( label_r ); error( true ); }

  /** \overload also change the progress bar label. */
  void error( tribool error_r, const std::string & label_r )
  { _progress.name( label_r ); error( error_r ); }

public:
  /** \name Access the embedded ProgressData object */
  //@{
  zypp::ProgressData * operator->()
  { return &_progress; }

  const zypp::ProgressData * operator->() const
  { return &_progress; }

  zypp::ProgressData & operator*()
  { return _progress; }

  const zypp::ProgressData & operator*() const
  { return _progress; }
  //@}

private:
  /** ProgressData::ReceiverFnc printing to a ProgressBar.
    *
    * \note This could also be used to let an external \ref ProgressData object
    * trigger a \ref ProgressBar. \ref ProgressBar::label and \ref ProgressBar::print
    * however use the embedded ProgressData object (esp. it's label). So don't mix this.
    */
  struct Print
  {
    Print( ProgressBar & bar_r ) : _bar( &bar_r ) {}
    bool operator()( const ProgressData & progress_r )
    { _bar->_out.progress( _bar->_progressId, _bar->outLabel( progress_r.name() ), progress_r.reportValue() ); return true; }
  private:
    ProgressBar * _bar;
  };

  std::string outLabel( const std::string & msg_r ) const
  { return _labelPrefix.empty() ? msg_r : _labelPrefix + msg_r; }

  int numDigits( unsigned num_r ) const
  { int ret = 1; while ( num_r /= 10 ) ++ret; return ret; }

private:
  Out & _out;
  tribool _error;
  ProgressData _progress;
  std::string _progressId;
  std::string _labelPrefix;
};
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
/// \class Out::Error
/// \brief Convenience class Error reporting.
///
/// Called action methods may \c throw this as exception. The calling function
/// should catch and process it (e.g. by calling \ref report).
///
/// This allows e.g. active \ref Out::ProgressBar objects to close properly
/// before the error message is displayed.
///
/// \code
///     try {
///       Out::ProgressBar report( zypper_r.out(), _("Scanning download directory") );
///       report->range( todolist.size() );
///       // now report will indicate an error id closed before reaching 100%
///       ....
///       if ( some error )
///         throw( Out::Error( ZYPPER_EXIT_ERR_BUG,
///                           _("Failed to read download directory"),
///                           Errno().asString() ) );
///
///     }
///     catch ( const SourceDownloadImpl::Error & error_r )
///     {
///       // Default way of processing a caught Error exception:
///       // - Write error message and optional hint to screen.
///       // - Set the ZYPPER_EXIT_ code if necessary.
///       // - Return the current ZYPPER_EXIT_ code.
///       return error_r.report( zypper_r );
///     }
/// \endcode
///////////////////////////////////////////////////////////////////
struct Out::Error
{
  Error()
  : _exitcode( ZYPPER_EXIT_OK ) {}
  Error( int exitcode_r )
  : _exitcode( exitcode_r ) {}

  Error( int exitcode_r, const std::string & msg_r, const std::string & hint_r = std::string() )
  : _exitcode( exitcode_r ) , _msg( msg_r ) , _hint( hint_r ) {}
  Error( int exitcode_r, const boost::format & msg_r, const std::string & hint_r = std::string() )
  : _exitcode( exitcode_r ) , _msg( boost::str( msg_r ) ) , _hint( hint_r ) {}
  Error( int exitcode_r, const std::string & msg_r, const boost::format & hint_r )
  : _exitcode( exitcode_r ) , _msg( msg_r ) , _hint(  boost::str( hint_r ) ) {}
  Error( int exitcode_r, const boost::format & msg_r, const boost::format & hint_r )
  : _exitcode( exitcode_r ) , _msg( boost::str( msg_r ) ) , _hint( boost::str( hint_r ) ) {}

  Error( const std::string & msg_r, const std::string & hint_r = std::string() )
  : _exitcode( ZYPPER_EXIT_OK ) , _msg( msg_r ) , _hint( hint_r ) {}
  Error( const boost::format & msg_r, const std::string & hint_r = std::string() )
  : _exitcode( ZYPPER_EXIT_OK ) , _msg( boost::str( msg_r ) ) , _hint( hint_r ) {}
  Error( const std::string & msg_r, const boost::format & hint_r )
  : _exitcode( ZYPPER_EXIT_OK ) , _msg( msg_r ) , _hint(  boost::str( hint_r ) ) {}
  Error( const boost::format & msg_r, const boost::format & hint_r )
  : _exitcode( ZYPPER_EXIT_OK ) , _msg( boost::str( msg_r ) ) , _hint( boost::str( hint_r ) ) {}

  /** Default way of processing a caught \ref Error exception.
   * \li Write error message and optional hint to screen.
   * \li Set the ZYPPER_EXIT_ code if necessary.
   * \returns the zypper exitcode.
   */
  int report( Zypper & zypper_r ) const;

  int _exitcode;	//< ZYPPER_EXIT_OK indicates exitcode is already set.
  std::string _msg;
  std::string _hint;
};
///////////////////////////////////////////////////////////////////

#endif /*OUT_H_*/
