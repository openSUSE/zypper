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

/*
enum Error {
    NO_ERROR,
    NOT_FOUND,
    IO,
    INVALID,
};
*/
void display_progress (const std::string& s, int percent);
void display_tick (const std::string& s);
void display_done (const std::string& s);
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
 * 		is specified, 0 (ABORT) is returned. In interactive mode, this parameter
 *    is ignored.
 */
int read_action_ari (int default_action = -1);

/**
 * Prompt for Yes/No answer from stdin.
 * 
 * \todo work with default
 * \todo non-interactive mode
 * \todo make this localized
 */
bool readBoolAnswer();

std::string to_string (zypp::Resolvable::constPtr resolvable);
#endif
