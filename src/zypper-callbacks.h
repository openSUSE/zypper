/*-----------------------------------------------------------*- c++ -*-\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPPER_CALLBACKS_H
#define ZYPPER_CALLBACKS_H

#include <iostream>
#include <string>

#include "zypp/Resolvable.h"
#include "zypp/base/Exception.h"

#include "zypper.h"

/*
enum Error {
    NO_ERROR,
    NOT_FOUND,
    IO,
    INVALID,
};
*/
void display_progress ( const std::string &id, std::ostream & out, const std::string& s, int percent);
void display_tick ( const std::string &id, std::ostream & out, const std::string& s);
void display_done ( const std::string &id, ostream & out, const std::string& s);
// newline if normal progress is on single line
void display_done ( const std::string &id, ostream & out);

//! \todo this must be rewritten as the error enums are different for some zypp callbacks
template<typename Error>
void display_error (Error error, const std::string& reason) {
  if (error != 0 /*NO_ERROR*/)
  {
    static const char * error_s[] = {
      // TranslatorExplanation These are reasons for various failures.
      "", _("Not found"), _("I/O error"), _("Invalid object")
    };
    ostream& stm = std::cerr;

    // error type:
    if (error < 3)
      stm << error_s[error];
    else
      stm << _("Error");
    // description
    if (!reason.empty())
	stm << ": " << reason;
    stm << std::endl;
  }
}

/**
 * Abort, Retry, Ignore stdin prompt.
 * \param default_action Answer to be returned in non-interactive mode. If none
 *    is specified, 0 (ABORT) is returned. In interactive mode, this parameter
 *    is ignored.
 */
int read_action_ari (int default_action = -1);

/**
 * Prompt for y/n answer (localized) from stdin.
 *
 * \param question Question to be printed on prompt.
 * \param default_answer Value to be returned in non-interactive mode or on
 *      input failure.
 */
bool read_bool_answer(const string & question, bool default_answer);

/**
 * Returns string representation of a resolvable.
 */
std::string to_string (zypp::Resolvable::constPtr resolvable);

/**
 * Prints exception message. If there is exception history available for the
 * exception, this method prints the root cause or the whole history (if
 * verbosity level is >2). Otherwise it just prist the e.asUserString() of the
 * exception passed.
 */
void report_zypp_exception(const zypp::Exception & e);

/**
 * Prints the problem description caused by an exception, its cause and,
 * optionaly, a hint for the user.
 * 
 * \param e Exception which caused the problem.
 * \param Problem description for the user.
 * \param Hint for the user how to cope with the problem.
 */
void report_problem(const zypp::Exception & e,
                    const string & problem_desc,
                    const string & hint = "");

/**
 * 
 */
void report_too_many_arguments(const string & specific_help);

#endif
