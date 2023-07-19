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
#include "output/Out.h"
#include "utils/ansi.h"
#include "utils/console.h"
#include "main.h" // for gettext macros



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
