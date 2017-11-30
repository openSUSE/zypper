/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>

#include <zypp/base/Logger.h>

#include "Zypper.h"
#include "colors.h"

///////////////////////////////////////////////////////////////////
// from ansi.h

bool do_ttyout()
{
  return Zypper::instance().config().do_ttyout;
}

bool do_colors()
{
  return Zypper::instance().config().do_colors;
}

namespace ansi
{
  namespace tty
  {
    const EscapeSequence clearLN	( "\033[2K\r", "\n" );
    const EscapeSequence cursorUP	( "\033[1A" );
    const EscapeSequence cursorDOWN	( "\033[1B" );
    const EscapeSequence cursorRIGHT	( "\033[1C" );
    const EscapeSequence cursorLEFT	( "\033[1D" );
  } // namespace tty
} // namespace tty
// from ansi.h
///////////////////////////////////////////////////////////////////

ansi::Color customColorCtor( ColorContext ctxt_r )
{
  const Config & conf( Zypper::instance().config() );
  switch ( ctxt_r )
  {
    case ColorContext::RESULT:		return conf.color_result;
    case ColorContext::MSG_STATUS:	return conf.color_msgStatus;
    case ColorContext::MSG_WARNING:	return conf.color_msgWarning;
    case ColorContext::MSG_ERROR:	return conf.color_msgError;
    case ColorContext::PROMPT:		return conf.color_prompt;
    case ColorContext::PROMPT_OPTION:	return conf.color_promptOption;
    case ColorContext::POSITIVE:	return conf.color_positive;
    case ColorContext::CHANGE:		return conf.color_change;
    case ColorContext::NEGATIVE:	return conf.color_negative;
    case ColorContext::HIGHLIGHT:	return conf.color_highlight;
    case ColorContext::LOWLIGHT:	return conf.color_lowlight;
    case ColorContext::OSDEBUG:		return conf.color_osdebug;

    case ColorContext::DEFAULT:
      break;			// use default...
  }
  return ansi::Color::Default;	// default
}
