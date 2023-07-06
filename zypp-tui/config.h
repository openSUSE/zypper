/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* Strictly for internal use!
*/

#ifndef ZYPP_TUI_CONFIG_INCLUDED
#define ZYPP_TUI_CONFIG_INCLUDED

#include <zypp-tui/utils/ansi.h>

namespace ztui {

  class Config {
  public:

    Config ();

    /**
     * True unless output is a dumb tty or file. In this case we should not use
     * any ANSI Escape sequences (at least those moving the cursor; color may
     * be explicitly enabled 'zypper --color ..| less'
     */
    bool do_ttyout;

    /**
     * Whether to colorize the output. This is evaluated according to
     * color_useColors and do_ttyout.
     */
    bool do_colors;

    ansi::Color color_result;
    ansi::Color color_msgStatus;
    ansi::Color color_msgError;
    ansi::Color color_msgWarning;
    ansi::Color color_prompt;
    ansi::Color color_promptOption;
    ansi::Color color_positive;
    ansi::Color color_change;
    ansi::Color color_negative;
    ansi::Color color_highlight;
    ansi::Color color_lowlight;
    ansi::Color color_osdebug;

  };

}

#endif
