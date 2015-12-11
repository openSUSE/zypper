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
#include <cstdlib>

#include <string>
#include <fstream>
#include <iostream>
#include <readline/readline.h>
#include <readline/history.h>

// ----------------------------------------------------------------------------

// Read a string. "\004" (^D) on EOF.
std::string readline_getline()
{
  std::string ret;

  //::rl_catch_signals = 0;
  /* Get a line from the user. */
  if ( char * line_read = ::readline( "zypper> " ) )
  {
    ret = line_read;
    /* If the line has any text in it, save it on the history. */
    if ( *line_read )
      ::add_history( line_read );
    ::free( line_read );
  }
  else
    ret = "\004";

  return ret;
}

// ----------------------------------------------------------------------------

unsigned get_screen_width()
{
  if ( !::isatty(STDOUT_FILENO) )
    return -1; // no clipping

  int width = 80;

  const char *cols_env = getenv("COLUMNS");
  if ( cols_env )
    width  = ::atoi( cols_env );
  else
  {
    ::rl_initialize();
    //::rl_reset_screen_size();
    ::rl_get_screen_size( NULL, &width );
  }

  // safe default
  if ( !width )
    width = 80;

  return width;
}

// ----------------------------------------------------------------------------

void clear_keyboard_buffer()
{
  // note: this will not clear characters typed after the last \n
  std::ifstream stm( "/dev/tty" );
  char s[8];
  while (stm.good() && stm.readsome(s, 8));
}
