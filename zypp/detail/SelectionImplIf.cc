/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/SelectionImplIf.cc
 *
*/
#include "zypp/detail/SelectionImplIf.h"
#include <iostream>

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
    // Default implementation of SelectionImplIf attributes,
    // as far as resonable.
    /////////////////////////////////////////////////////////////////

    const TranslatedText & SelectionImplIf::summary() const
    { return TranslatedText(); }

    const TranslatedText & SelectionImplIf::description() const
    { return TranslatedText(); }

    Label SelectionImplIf::category() const
    { return Label(); }

    bool SelectionImplIf::visible() const
    { return false; }

    Label SelectionImplIf::order() const
    { return Label(); }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
