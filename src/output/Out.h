#ifndef OUT_H_
#define OUT_H_

#include <string>

#include "zypp/base/NonCopyable.h"
#include "zypp/base/Exception.h"
#include "zypp/Url.h"

#include "utils/prompt.h"

#include "output/prompt.h"

class Table;

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

  /**
   * Show an error message and an optional hint.
   *
   * An error message should be shown regardless of the verbosity level.
   *
   * \param problem_desc Problem description (what happend)
   * \param hint         Hint for the user (what to do, or explanation)
   */
  virtual void error(const std::string & problem_desc, const std::string & hint = "") = 0;

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

#endif /*OUT_H_*/
