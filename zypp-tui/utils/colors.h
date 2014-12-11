/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef UTILS_COLORS_H_
#define UTILS_COLORS_H_

#include <iostream>
#include <string>

#include "ansi.h"

/** Simple check whether stdout can handle colors */
bool has_colors();

/** If output is done in colors (depends on config) */
bool do_colors();

///////////////////////////////////////////////////////////////////

enum class ColorContext
{
  DEFAULT,

  RESULT,
  MSG_STATUS,
  MSG_ERROR,
  MSG_WARNING,
  PROMPT,
  PROMPT_OPTION,
  POSITIVE,
  CHANGE,
  NEGATIVE,
  HIGHLIGHT,
  LOWLIGHT,

  OSDEBUG
};

/** \relates ColorContext map to \ref ansi::Color */
ansi::Color customColorCtor( ColorContext ctxt_r );

namespace ansi
{
  // Enable using ColorContext as ansi::SGRSequence
  template<>
  struct ColorTraits<::ColorContext>
  { enum { customColorCtor = true }; };
}

#endif /* UTILS_COLORS_H_ */
