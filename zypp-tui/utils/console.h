/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

/** \file console.h
 * Miscellaneous console utilities.
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_

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


#endif /* CONSOLE_H_ */
