/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/PatternImplIf.h
 *
*/
#ifndef ZYPP_DETAIL_PATTERNIMPLIF_H
#define ZYPP_DETAIL_PATTERNIMPLIF_H

#include "zypp/detail/ResObjectImplIf.h"
#include "zypp/Pathname.h"
#include <string>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Pattern;

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PatternImplIf
    //
    /** Abstact Pattern implementation interface.
    */
    class PatternImplIf : public ResObjectImplIf
    {
    public:
      typedef Pattern ResType;

    public:
      virtual bool isDefault() const PURE_VIRTUAL;

      virtual bool userVisible() const PURE_VIRTUAL;

      virtual std::string category() const PURE_VIRTUAL;

      virtual Pathname icon() const PURE_VIRTUAL;

      virtual Pathname script() const PURE_VIRTUAL;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PATTERNIMPLIF_H
