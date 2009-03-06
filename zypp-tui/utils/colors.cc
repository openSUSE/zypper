/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "colors.h"

using namespace std;

bool has_colors()
{
  if (::isatty(STDOUT_FILENO))
  {
    char *term = ::getenv("TERM");
    if (term && ::strcmp(term, "dumb"))
      return true;
  }
  return false;
}

void print_color(const std::string & s,
    const char * ansi_color_seq, const char * prev_color)
{
  if (prev_color)
    cout << COLOR_RESET;

  cout << ansi_color_seq << s;

  if (prev_color)
    cout << prev_color;
  else
    cout << COLOR_RESET;
}
