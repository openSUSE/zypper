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

#include <zypp/base/Logger.h>

#include "Zypper.h"

#include "colors.h"

using namespace std;


Color::Color(const string & color_str)
  : _value(parse(color_str))
{}

string Color::parse(const string & value)
{
  static map<string, string> str2esc = {
    { "",		""			},	// no color
    { "green",		COLOR_GREEN		},
    { "lightgreen",	COLOR_GREEN_LIGHT	},
    { "red",		COLOR_RED		},
    { "lightred",	COLOR_RED_LIGHT		},
    { "grey",		COLOR_WHITE		},
    { "white",		COLOR_WHITE_LIGHT	},
    { "brown",		COLOR_YELLOW		},
    { "yellow",		COLOR_YELLOW_LIGHT	},
    { "purple",		COLOR_PURPLE		},
    { "lightpurple",	COLOR_PURPLE_LIGHT	},
    { "blue",		COLOR_BLUE		},
    { "lightblue",	COLOR_BLUE_LIGHT	},
    { "cyan",		COLOR_CYAN		},
    { "lightcyan",	COLOR_CYAN_LIGHT	},
    { "black",		COLOR_BLACK		},
    { "darkgrey",	COLOR_GREY_DARK		},

    { "reset",		COLOR_RESET		}
  };

  auto it = str2esc.find(value);
  if (it == str2esc.end())
  {
    ERR << "Unknown color '" << value << "'" << endl;
    return string();
  }
  return it->second;
}

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

bool do_colors()
{
  return Zypper::instance()->config().do_colors;
}

const string get_color( const ColorContext context )
{
  const Config & conf( Zypper::instance()->config() );
  switch (context)
  {
  case COLOR_CONTEXT_RESULT:
    return conf.color_result.value();
  case COLOR_CONTEXT_MSG_STATUS:
    return conf.color_msgStatus.value();
  case COLOR_CONTEXT_MSG_WARNING:
    return conf.color_msgWarning.value();
  case COLOR_CONTEXT_MSG_ERROR:
    return conf.color_msgError.value();
  case COLOR_CONTEXT_POSITIVE:
    return conf.color_positive.value();
  case COLOR_CONTEXT_NEGATIVE:
    return conf.color_negative.value();
  case COLOR_CONTEXT_PROMPT_OPTION:
    return conf.color_promptOption.value();
  case COLOR_CONTEXT_HIGHLIGHT:
    return conf.color_highlight.value();
  case COLOR_CONTEXT_OSDEBUG:
    return Color("brown").value();
  default:
    return COLOR_RESET;
  }
}

void print_color( ostream & str, const std::string & s, const char * ansi_color_seq, const char * prev_color )
{
  if ( do_colors() )
  {
    if (prev_color)
      str << COLOR_RESET;

    str << ansi_color_seq << s << COLOR_RESET;

    if (prev_color)
      str << prev_color;
  }
  else
    str << s;
}
