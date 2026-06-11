/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef CONSOLE_H_
#define CONSOLE_H_

#include <zypp-core/Globals.h>
#include <zypp-tui/utils/console.h>

#if LEGACY(173814)  // ztui::readline_getline has no custom prompt in older libzypp
#include <readline/readline.h>
#include <readline/history.h>
inline std::string readline_getline( std::string prompt_r = " >" )
{
  std::string ret;

  //::rl_catch_signals = 0;
  /* Get a line from the user. */
  if ( char * line_read = ::readline( prompt_r.c_str() ) )
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
#else
using ztui::readline_getline;
#endif
using ztui::get_screen_width;
using ztui::clear_keyboard_buffer;

#endif
