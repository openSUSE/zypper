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

#ifndef ZYPP_TUI_UTILS_COLORS_H_
#define ZYPP_TUI_UTILS_COLORS_H_

#include <iostream>
#include <string>

#include "ansi.h"

namespace ztui {

/** True unless output is a dumb tty or file.
 * In this case we should not use any ANSI Escape sequences moving the cursor.
 */
bool do_ttyout();

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
  struct ColorTraits<ztui::ColorContext>
  { enum { customColorCtor = true }; };
}

// ColorString types
template <ColorContext _ctxt>
struct CCString : public ColorString
{
  CCString()						: ColorString( _ctxt ) {}
  explicit CCString( const std::string & str_r )	: ColorString( _ctxt, str_r ) {}
  explicit CCString( std::string && str_r )		: ColorString( _ctxt, std::move(str_r) ) {}
};

template <ColorContext _ctxt>
inline ansi::ColorStream & operator<<( ansi::ColorStream & cstr_r, const CCString<_ctxt> & cstring_r )
{ return cstr_r << (const ColorString &)cstring_r; }

typedef CCString<ColorContext::DEFAULT>		DEFAULTString;

typedef CCString<ColorContext::MSG_STATUS>	MSG_STATUSString;
typedef CCString<ColorContext::MSG_ERROR>	MSG_ERRORString;
typedef CCString<ColorContext::MSG_WARNING>	MSG_WARNINGString;

typedef CCString<ColorContext::POSITIVE>	POSITIVEString;
typedef CCString<ColorContext::CHANGE>		CHANGEString;
typedef CCString<ColorContext::NEGATIVE>	NEGATIVEString;

typedef CCString<ColorContext::HIGHLIGHT>	HIGHLIGHTString;
typedef CCString<ColorContext::LOWLIGHT>	LOWLIGHTString;

}

#endif /* ZYPP_TUI_UTILS_COLORS_H_ */
