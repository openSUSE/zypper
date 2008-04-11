/*---------------------------------------------------------------------\
 |                          ____ _   __ __ ___                          |
 |                         |__  / \ / / . \ . \                         |
 |                           / / \ V /|  _/  _/                         |
 |                          / /__ | | | | | |                           |
 |                         /_____||_| |_| |_|                           |
 |                                                                      |
 \---------------------------------------------------------------------*/
/** \file	zypp/ui/PatternContents.h
 *
*/
#ifndef ZYPP_UI_PATTERN_CONTENTS_H
#define ZYPP_UI_PATTERN_CONTENTS_H

#include "zypp/base/PtrTypes.h"
#include "zypp/Pattern.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace ui
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //  CLASS NAME : PatternContents
    //
    /**  Helper class that computes a patterns expanded install_packages set.
     *
     * Joins the install_packages sets of this and all included or extending
     * pattens.
     *
     * \deprecated use Pattern::contents method instead.
    */
    class ZYPP_DEPRECATED PatternContents
    {
    public:
      class Impl;

    public:
      /** Ctor taking the Pattern to expand. */
      PatternContents( const Pattern::constPtr & pattern ) ZYPP_DEPRECATED;

      /** Return the expanded Patterns install_packages. */
      std::set<std::string> install_packages() const ZYPP_DEPRECATED;

    private:
      RW_pointer<Impl> _pimpl;
    };

    ///////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_UI_PATTERN_CONTENTS_H
