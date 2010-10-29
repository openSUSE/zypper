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

/**
 * Example:
 * <code>
 * PromptOptions popts;
 * popts.setOptions(_("y/n/p"), 0 / * default reply * /);
 * popts.setOptionHelp(0, _("Help for option 'y'"));
 * popts.setOptionHelp(1, _("Help for option 'n'"));
 * ...
 * zypper.out().prompt(PROMPT_YN_INST_REMOVE_CONTINUE, prompt_text, popts);
 * unsigned int reply =
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
  PromptOptions() : _shown_count(-1) {};

  /**
   * Constructor.
   *
   * \param option_str translated option string containing one or more
   *                   options separated by slash '/' character
   *                   e.g. "yes/no/?" or "1/s/r/c"
   * \param default_opt index of the default answer within the \a option_str
   */
  PromptOptions(const std::string & option_str, unsigned int default_opt);

  /** D-tor */
  ~PromptOptions();

  const StrVector & options() const { return _options; }
  void setOptions(const std::string & option_str, unsigned int default_opt);
  unsigned int defaultOpt() const { return _default; }
  const std::string optionString() const;
  const std::string optionStringColored() const;
  bool empty() const { return _options.empty(); }
  bool isYesNoPrompt() const;

  const std::string optionHelp(unsigned int opt) const
  { static std::string empty; return opt < _opt_help.size() ? _opt_help[opt] : empty; }
  //const std::string getOptionHelp(const std::string & opt_str);
  void setOptionHelp(unsigned int opt, const std::string & help_str);
  bool helpEmpty() const { return _opt_help.empty(); }

  bool isEnabled(unsigned int opt) const
  { return _disabled.find(opt) == _disabled.end(); }
  bool isDisabled(unsigned int opt) const
  { return _disabled.find(opt) != _disabled.end(); }
  void disable(unsigned int opt)
  { _disabled.insert(opt); }
  void enable(unsigned int opt)
  { _disabled.erase(opt); }
  void enableAll()
  { _disabled.clear(); }

  unsigned int shownCount() const
  { return _shown_count; }
  void setShownCount(unsigned int count)
  { _shown_count = count; }

  int getReplyIndex(const std::string & reply) const;

private:
  /** option strings */
  StrVector _options;
  /** index of the default option */
  unsigned int _default;
  /** help strings corresponding to options */
  StrVector _opt_help;
  /** set of options to ignore */
  std::set<unsigned int> _disabled;
  /**
   * Number of options to show (the rest will still be available and visible
   * through '?' help). If negative, all options will be shown. Zero is allowed.
   */
  int _shown_count;
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

class Zypper;
unsigned int get_prompt_reply(Zypper & zypper,
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
