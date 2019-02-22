/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPERPROMPT_H_
#define ZYPPERPROMPT_H_

#include <set>

#include "output/prompt.h"
#include "utils/console.h"
#include "main.h" // for gettext macros
#include "ansi.h"

/**
 * Example:
 * <code>
 * PromptOptions popts;
 * popts.setOptions(_("y/n/p"), 0 / * default reply * /);
 * popts.setOptionHelp(0, _("Help for option 'y'"));
 * popts.setOptionHelp(1, _("Help for option 'n'"));
 * ...
 * zypper.out().prompt(PROMPT_YN_INST_REMOVE_CONTINUE, prompt_text, popts);
 * unsigned reply =
 *   get_prompt_reply(zypper, PROMPT_YN_INST_REMOVE_CONTINUE, popts);
 * </code>
 */
class PromptOptions
{
public:
  typedef std::vector<std::string> StrVector;

  /**
   * Default c-tor.
   */
  PromptOptions() {};

  /** Ctor taking the option values as vector */
  PromptOptions( StrVector options_r, unsigned defaultOpt_r );

  /**
   * Constructor.
   *
   * \param optionstr_r  translated option string containing one or more
   *                     options separated by slash '/' character
   *                     e.g. "yes/no/?" or "1/s/r/c"
   * \param defaultOpt_r index of the default answer within the \a option_str
   */
  PromptOptions( const std::string & optionstr_r, unsigned defaultOpt_r );

  /** D-tor */
  ~PromptOptions();

  const StrVector & options() const { return _options; }
  void setOptions( StrVector options_r, unsigned defaultOpt_r );
  void setOptions( const std::string & optionstr_r, unsigned defaultOpt_r );

  unsigned defaultOpt() const { return _default; }
  /** Option string (may have embedded color codes) */
  ColorString optionString() const;
  bool empty() const { return _options.empty(); }
  bool isYesNoPrompt() const;

  const std::string & optionHelp(unsigned opt) const
  { static std::string empty; return opt < _opt_help.size() ? _opt_help[opt] : empty; }
  //const std::string getOptionHelp(const std::string & opt_str);
  void setOptionHelp(unsigned opt, const std::string & help_str);
  bool helpEmpty() const { return _opt_help.empty(); }

  bool isEnabled(unsigned opt) const
  { return !isDisabled(opt); }
  bool isDisabled(unsigned opt) const
  { return _disabled.count(opt); }
  void disable(unsigned opt)
  { _disabled.insert(opt); }
  void enable(unsigned opt)
  { _disabled.erase(opt); }
  void enableAll()
  { _disabled.clear(); }

  unsigned shownCount() const
  { return _shown_count; }
  void setShownCount(unsigned count)
  { _shown_count = count; }

  /** Return the indices of option string matches (lowercase/prefix or #NUM). */
  std::vector<int> getReplyMatches( const std::string & reply_r ) const;
  /** The returned reply matches as '(,)' list. */
  std::string replyMatchesStr( const std::vector<int> & matches_r ) const;

private:
  /** option strings */
  StrVector _options;
  /** index of the default option */
  unsigned _default = 0;
  /** help strings corresponding to options */
  StrVector _opt_help;
  /** set of options to ignore */
  std::set<unsigned> _disabled;
  /**
   * Number of options to show (the rest will still be available and visible
   * through '?' help). If negative, all options will be shown. Zero is allowed.
   */
  int _shown_count = -1;
};


/**
 * Abort, Retry, Ignore stdin prompt with timeout.
 * \param default_action Answer to be returned in non-interactive mode. If none
 *    is specified, 0 (ABORT) is returned. In interactive mode, this parameter
 *    is ignored.
 * \param timeout how many seconds wait until return default action
 */
int read_action_ari_with_timeout (PromptId pid, unsigned timeout,
    int default_action = 0);

/**
 * Abort, Retry, Ignore stdin prompt.
 * \param default_action Answer to be returned in non-interactive mode. If none
 *    is specified, 0 (ABORT) is returned. In interactive mode, this parameter
 *    is ignored.
 */
int read_action_ari (PromptId pid, int default_action = -1);

/**
 * Prompt for y/n answer (localized) from stdin.
 *
 * \param question Question to be printed on prompt.
 * \param default_answer Value to be returned in non-interactive mode or on
 *      input failure.
 */
bool read_bool_answer(PromptId pid, const std::string & question, bool default_answer);

/**
 * Prompt for y/n/always/never answer (localized) from stdin.
 *
 * \param question Question to be printed on prompt.
 * \param default_answer Value to be returned in non-interactive mode or on
 *      input failure.
 *
 * The answer is returned in the first \c bool. The second \c bool tells
 * whether this answer should be remembered:
 *          |  1st  |  2nd   |
 *   yes    | true  | false  |
 *   no     | false | false  |
 *   always | true  | true   |
 *   never  | false | true   |
 */
std::pair<bool,bool> read_bool_answer_opt_save( PromptId pid_r, const std::string & question_r, bool defaultAnswer_r );

class Zypper;
unsigned get_prompt_reply(Zypper & zypper,
                              PromptId pid,
                              const PromptOptions & poptions);

/**
 * Get text from user using readline without history.
 * \param prompt prompt text or empty string
 * \param prefilled prefilled text
 */
std::string get_text(const std::string & prompt, const std::string & prefilled = "");

/*
enum Error {
    NO_ERROR,
    NOT_FOUND,
    IO,
    INVALID,
};
*/

//! \todo this must be rewritten as the error enums are different for some zypp callbacks
template<typename Error>
std::string zcb_error2str (Error error, const std::string & reason)
{
  std::string errstr;
  if (error != 0 /*NO_ERROR*/)
  {
    static const char * error_s[] = {
      // TranslatorExplanation These are reasons for various failures.
      "", _("Not found"), _("I/O error"), _("Invalid object")
    };

    // error type:
    if (error < 3)
      errstr += error_s[error];
    else
      errstr += _("Error");
    // description
    if (!reason.empty())
      errstr += ": " + reason;
  }

  return errstr;
}

#endif /*ZYPPERPROMPT_H_*/
