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

#include "zypp/base/Logger.h"

#include "Zypper.h"

#include "colors.h"

using namespace std;

static map<string, string> str2esc;

Color::Color(const string & color_str)
  : _value(parse(color_str))
{}

string Color::parse(const string & value)
{
  if (value.empty())
    return value;

  if (str2esc.empty())
  {
    str2esc["green"]          = COLOR_GREEN;
    str2esc["lightgreen"]     = COLOR_GREEN_LIGHT;
    str2esc["red"]            = COLOR_RED;
    str2esc["lightred"]       = COLOR_RED_LIGHT;
    str2esc["grey"]           = COLOR_WHITE;
    str2esc["white"]          = COLOR_WHITE_LIGHT;
    str2esc["brown"]          = COLOR_YELLOW;
    str2esc["yellow"]         = COLOR_YELLOW_LIGHT;
    str2esc["purple"]         = COLOR_PURPLE;
    str2esc["lightpurple"]    = COLOR_PURPLE_LIGHT;
    str2esc["blue"]           = COLOR_BLUE;
    str2esc["lightblue"]      = COLOR_BLUE_LIGHT;
    str2esc["cyan"]           = COLOR_CYAN;
    str2esc["lightcyan"]      = COLOR_CYAN_LIGHT;
    str2esc["black"]          = COLOR_BLACK;
    str2esc["darkgrey"]       = COLOR_GREY_DARK;

    str2esc["reset"]          = COLOR_RESET;
  }

  map<string, string>::const_iterator it = str2esc.find(value);
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

static const string get_color(const Config & conf, const ColorContext context)
{
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
  case COLOR_CONTEXT_POSTIVE:
    return conf.color_positive.value();
  case COLOR_CONTEXT_NEGATIVE:
    return conf.color_negative.value();
  case COLOR_CONTEXT_PROMPT_OPTION:
    return conf.color_promptOption.value();
  case COLOR_CONTEXT_HIGHLIGHT:
    return conf.color_highlight.value();
  default:
    return COLOR_RESET;
  }
}

const string get_color(const ColorContext context)
{
  return get_color(Zypper::instance()->config(), context);
}

void print_color(const std::string & s,
    const char * ansi_color_seq, const char * prev_color)
{
  fprint_color(cout, s, ansi_color_seq, prev_color);
}

void fprint_color(ostream & str, const std::string & s,
    const char * ansi_color_seq, const char * prev_color)
{
  if (Zypper::instance()->config().do_colors)
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

void fprint_color(ostream & str, const std::string & s,
    const ColorContext cc, const ColorContext prev_cc)
{
  const Config & conf = Zypper::instance()->config();
  fprint_color(str, s, get_color(conf, cc).c_str(), get_color(conf, prev_cc).c_str());
}

void print_color(const std::string & s,
    const ColorContext cc, const ColorContext prev_cc)
{
  fprint_color(cout, s, cc, prev_cc);
}
