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

// abort, retry, ignore
int read_action_ari ();
std::string to_string (zypp::Resolvable::constPtr resolvable);
#endif

