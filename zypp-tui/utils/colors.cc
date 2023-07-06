/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------*/

#include <iostream>

#include <zypp/base/Logger.h>
#include <zypp-tui/Application>
#include "colors.h"

namespace ztui {

namespace env
{
  inline bool NO_COLOR()
  { return ::getenv("NO_COLOR"); }
} // namespace env

///////////////////////////////////////////////////////////////////
// from ansi.h

bool do_ttyout()
{
  return Application::instance().config().do_ttyout;
}

bool do_colors()
{
  return Application::instance().config().do_colors;
}

bool mayUseANSIEscapes()
{
  constexpr auto detectAnsiEscapes = [](){
    if ( ::isatty(STDOUT_FILENO) )
    {
      char *term = ::getenv("TERM");
      if ( term && ::strcmp( term, "dumb" ) )
        return true;
    }
    return false;
  };

  static bool mayUse = detectAnsiEscapes();
  return mayUse;
}

bool hasANSIColor()
{ return mayUseANSIEscapes() && not env::NO_COLOR(); }

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


  Color Color::fromString(const std::string &colorName)
  {
    static const std::map<std::string, ansi::Color> _def =  {
      { "black",	ansi::Color::Black		},
      { "darkgrey",	ansi::Color::BrightBlack	},
      { "red",		ansi::Color::Red		},
      { "green",	ansi::Color::Green		},
      { "brown",	ansi::Color::Yellow		},
      { "yellow",	ansi::Color::BrightYellow	},
      { "blue",		ansi::Color::Blue		},
      { "magenta",	ansi::Color::Magenta		},
      { "purple",	ansi::Color::Magenta		},
      { "cyan",		ansi::Color::Cyan		},
      { "grey",		ansi::Color::White		},
      { "white",	ansi::Color::BrightWhite	},
      { "default",	ansi::Color::Default		},
      { "",		ansi::Color::Default		},  // matches "bold" "light" "bright" NOT ""
    };

    ansi::Color ret = ansi::Color::nocolor();

    if ( colorName.empty() )	// "" when undefined in config file
      return ret;

    std::string name_r = zypp::str::toLower( colorName );

    if ( zypp::str::hasPrefix( name_r, "bold" ) ) {
      name_r.erase( 0, 4 );
      ret <= ansi::Color::Attr::Bright;

    } else if ( zypp::str::hasPrefix( name_r, "light" ) ) {
      name_r.erase( 0, 5 );
      ret <= ansi::Color::Attr::Bright;

    } else if ( zypp::str::hasPrefix( name_r, "bright" ) ) {
      name_r.erase( 0, 6 );
      ret <= ansi::Color::Attr::Bright;
    }

    auto && it = _def.find( name_r );
    if ( it == _def.end() )
    {
      ERR << "Unknown color name '" << name_r << "'" << std::endl;
      ret = ansi::Color::Default;
    }
    else
    {
      ret = ( it->second < ret );
    }
    return ret;
  }


} // namespace tty
// from ansi.h
///////////////////////////////////////////////////////////////////

ansi::Color customColorCtor( ColorContext ctxt_r )
{
  const ztui::Config & conf( Application::instance().config() );
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

}
