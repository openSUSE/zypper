/*---------------------------------------------------------------------\
 |                          ____ _   __ __ ___                          |
 |                         |__  / \ / / . \ . \                         |
 |                           / / \ V /|  _/  _/                         |
 |                          / /__ | | | | | |                           |
 |                         /_____||_| |_| |_|                           |
 |                                                                      |
 \---------------------------------------------------------------------*/
/** \file	zypp/ui/PatternContentsImpl.h
 *
*/
#ifndef ZYPP_UI_PATTERN_CONTENTS_IMPL_H
#define ZYPP_UI_PATTERN_CONTENTS_IMPL_H

#include "zypp/base/PtrTypes.h"
#include "zypp/ui/PatternContents.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace ui
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //  CLASS NAME : PatternContents::Impl
    //
    /** */
    class PatternContents::Impl
    {
    public:
      Impl( const Pattern::constPtr & pattern );

      std::set<std::string> install_packages( const Locale & lang = Locale("") ) const;

    private:
      Pattern::constPtr _pattern;
    };

    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_UI_PATTERN_CONTENTS_IMPL_H
