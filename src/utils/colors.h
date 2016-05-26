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

// ColorString types
template <ColorContext _ctxt>
struct CCString : public ColorString
{
  CCString()						: ColorString( _ctxt ) {}
  explicit CCString( const std::string & str_r )	: ColorString( _ctxt, str_r ) {}
  explicit CCString( std::string && str_r )		: ColorString( _ctxt, std::move(str_r) ) {}
};

typedef CCString<ColorContext::DEFAULT>		DEFAULTString;

typedef CCString<ColorContext::MSG_STATUS>	MSG_STATUSString;
typedef CCString<ColorContext::MSG_ERROR>	MSG_ERRORString;
typedef CCString<ColorContext::MSG_WARNING>	MSG_WARNINGString;

typedef CCString<ColorContext::POSITIVE>	POSITIVEString;
typedef CCString<ColorContext::CHANGE>		CHANGEString;
typedef CCString<ColorContext::NEGATIVE>	NEGATIVEString;

typedef CCString<ColorContext::HIGHLIGHT>	HIGHLIGHTString;
typedef CCString<ColorContext::LOWLIGHT>	LOWLIGHTString;

#endif /* UTILS_COLORS_H_ */
