/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

// #include <unistd.h>
// #include <stdlib.h>
// #include <string.h>

#include <iostream>
#include <map>

#include <zypp/base/Logger.h>

#include "Zypper.h"
#include "colors.h"

bool has_colors()
{
  if (::isatty(STDOUT_FILENO))
  {
    char *term = ::getenv("TERM");
    if ( term && ::strcmp(term, "dumb") )
      return true;
  }
  return false;
}

bool do_colors()
{
  return Zypper::instance()->config().do_colors;
}

///////////////////////////////////////////////////////////////////

ansi::Color customColorCtor( ColorContext ctxt_r )
{
  const Config & conf( Zypper::instance()->config() );
  switch ( ctxt_r )
  {
    case ColorContext::RESULT:		return conf.color_result;
    case ColorContext::MSG_STATUS:	return conf.color_msgStatus;
    case ColorContext::MSG_WARNING:	return conf.color_msgWarning;
    case ColorContext::MSG_ERROR:	return conf.color_msgError;
    case ColorContext::POSITIVE:	return conf.color_positive;
    case ColorContext::NEGATIVE:	return conf.color_negative;
    case ColorContext::PROMPT_OPTION:	return conf.color_promptOption;
    case ColorContext::HIGHLIGHT:	return conf.color_highlight;
    case ColorContext::LOWLIGHT:	return conf.color_lowlight;
    case ColorContext::OSDEBUG:		return conf.color_osdebug;

    case ColorContext::DEFAULT:
      break;			// use default...
  }
  return ansi::Color::Default;	// default
}
