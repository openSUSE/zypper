/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* Strictly for internal use!
*/

/** \file console.h
 * Miscellaneous console utilities.
 */

#ifndef ZYPP_TUI_UTILS_CONSOLE_H_
#define ZYPP_TUI_UTILS_CONSOLE_H_

#include <string>

namespace ztui {

/** Use readline to get line of input. */
std::string readline_getline();

/**
 * Reads COLUMNS environment variable or gets the screen width from readline,
 * in that order. Falls back to 80 if all that fails.
 *
 * \NOTE In case stdout is not connected to a terminal max. unsigned
 * is returned. This should prevent clipping when output is redirected.
 */
unsigned get_screen_width();

/**
 *  Clear the keyboard buffer.
 *  Useful before showing the user prompt message to catch any unwanted <enter>
 *  key hits (bnc #649248).
 *
 *  \NOTE This will not clear characters typed after the last \n
 */
void clear_keyboard_buffer();

}

#endif /* ZYPP_TUI_UTILS_CONSOLE_H_ */
