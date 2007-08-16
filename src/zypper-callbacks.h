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

#include <string>
#include <zypp/Resolvable.h>

// trying to compile for 10.1 where the callbacks had different signatures
#ifdef LIBZYPP_1xx
typedef std::string cbstring;
typedef zypp::Url cbUrl;
#else
typedef const std::string& cbstring;
typedef const zypp::Url& cbUrl;
#endif

/*
enum Error {
    NO_ERROR,
    NOT_FOUND,
    IO,
    INVALID,
};
*/
void display_progress (const std::string& s, int percent);
// newline if normal progress is on single line
void display_done ();

template<typename Error>
void display_error (Error error, const std::string& reason) {
  if (error != 0 /*NO_ERROR*/) {
    static const char * error_s[] = {
      "", "Not found", "I/O error", "Invalid object"
    };
    ostream& stm = std::cerr;
    stm << error_s[error];
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
 * Prompt for Yes/No answer from stdin.
 *
 * \param question Question to be printed on prompt.
 * \param default_answer Value to be returned in non-interactive mode or on
 *      input failure.
 *
 * \todo make this localized
 */
bool read_bool_answer(const string & question, bool default_answer);

std::string to_string (zypp::Resolvable::constPtr resolvable);
#endif
