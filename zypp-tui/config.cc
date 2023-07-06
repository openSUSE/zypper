/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------*/
#include "config.h"
#include <cstdlib>
#include <cstring>
#include <unistd.h>

namespace ztui {
  Config::Config()
    : do_ttyout         ( mayUseANSIEscapes() )
    , do_colors         ( hasANSIColor() )
    , color_result      (ansi::Color::fromString("default"))
    , color_msgStatus	(ansi::Color::fromString("default"))
    , color_msgError	(ansi::Color::fromString("red"))
    , color_msgWarning	(ansi::Color::fromString("purple"))
    , color_prompt      (ansi::Color::fromString("bold"))
    , color_promptOption(ansi::Color::nocolor())	// follow color_prompt
    , color_positive	(ansi::Color::fromString("green"))
    , color_change      (ansi::Color::fromString("brown"))
    , color_negative	(ansi::Color::fromString("red"))
    , color_highlight	(ansi::Color::fromString("cyan"))
    , color_lowlight	(ansi::Color::fromString("brown"))
    , color_osdebug     (ansi::Color::fromString("default") < ansi::Color::Attr::Reverse)
  { }

}
