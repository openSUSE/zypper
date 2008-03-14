#ifndef ZYPPERPROMPT_H_
#define ZYPPERPROMPT_H_

#include "output/prompt.h"
#include "zypper-main.h" // for gettext macros

class PromptOptions
{
public:
  typedef std::vector<std::string> OptionList;

  /**
   * Default c-tor.
   */
  PromptOptions() {};

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

  const OptionList & options() const { return _options; }
  void setOptions(const std::string & option_str, unsigned int default_opt);
  unsigned int defaultOpt() const { return _default; }

private:
  OptionList _options;
  unsigned int _default;
};


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
