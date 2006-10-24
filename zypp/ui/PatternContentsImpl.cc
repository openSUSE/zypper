/*---------------------------------------------------------------------\
 |                          ____ _   __ __ ___                          |
 |                         |__  / \ / / . \ . \                         |
 |                           / / \ V /|  _/  _/                         |
 |                          / /__ | | | | | |                           |
 |                         /_____||_| |_| |_|                           |
 |                                                                      |
 \---------------------------------------------------------------------*/
/** \file	zypp/ui/PatternContentsImpl.cc
 *
*/

#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/ui/PatternContentsImpl.h"
#include <zypp/ZYppFactory.h>
#include <zypp/ResPool.h>

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace ui
  { /////////////////////////////////////////////////////////////////

    PatternContents::Impl::Impl( const Pattern::constPtr & pattern )
    : _pattern( pattern )
    {}

    std::set<std::string> PatternContents::Impl::install_packages( const Locale & lang ) const
    {
      return _pattern->install_packages( lang );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
