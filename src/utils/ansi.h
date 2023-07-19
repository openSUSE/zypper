/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_UTILS_ANSI_H
#define ZYPPER_UTILS_ANSI_H

#include <zypp-tui/utils/ansi.h>

using ztui::mayUseANSIEscapes;
using ztui::hasANSIColor;
using ztui::do_ttyout;
using ztui::do_colors;

namespace ansi {
  using ztui::ansi::ColorTraits;
  using ztui::ansi::Color;
  using ztui::ansi::ColorString;
  using ztui::ansi::ColorStream;

  namespace tty {
    using ztui::ansi::tty::EscapeSequence;
    using ztui::ansi::tty::clearLN;
    using ztui::ansi::tty::cursorDOWN;
    using ztui::ansi::tty::cursorLEFT;
    using ztui::ansi::tty::cursorRIGHT;
    using ztui::ansi::tty::cursorUP;
    using ztui::ansi::tty::operator<<;
  }
}

using ansi::ColorString;
using ansi::ColorStream;

#endif // ZYPPER_UTILS_ANSI_H
using ansi::ColorString;
using ansi::ColorStream;
