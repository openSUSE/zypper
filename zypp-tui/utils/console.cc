/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

/** \file console.cc
 * Miscellaneous console utilities.
 */
#include <unistd.h>

#include <string>
#include <fstream>
#include <iostream>
#include <readline/readline.h>
#include <readline/history.h>
#include <cstdlib>

using namespace std;

// ----------------------------------------------------------------------------

// Read a string. "\004" (^D) on EOF.
string readline_getline()
{
  // A static variable for holding the line.
  static char *line_read = NULL;

  /* If the buffer has already been allocated,
     return the memory to the free pool. */
  if (line_read) {
    free (line_read);
    line_read = NULL;
  }

  //::rl_catch_signals = 0;
  /* Get a line from the user. */
  line_read = ::readline ("zypper> ");

  /* If the line has any text in it,
     save it on the history. */
  if (line_read && *line_read)
    ::add_history (line_read);

  if (line_read)
    return line_read;
  else
    return "\004";
}

// ----------------------------------------------------------------------------

unsigned get_screen_width()
{
  if (!::isatty(STDOUT_FILENO))
    return -1; // no clipping

  int width = 80;

  const char *cols_env = getenv("COLUMNS");
  if (cols_env)
    width  = ::atoi (cols_env);
  else
  {
    ::rl_initialize();
    //::rl_reset_screen_size();
    ::rl_get_screen_size (NULL, &width);
  }

  // safe default
  if (!width)
    width = 80;

  return width;
}

// ----------------------------------------------------------------------------

void clear_keyboard_buffer()
{
  // note: this will not clear characters typed after the last \n
  ifstream stm("/dev/tty", ifstream::in);
  char s[8];
  while (stm.good() && stm.readsome(s, 8));
}
