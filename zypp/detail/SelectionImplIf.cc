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

    TranslatedText SelectionImplIf::summary() const
    { return TranslatedText::notext; }

    TranslatedText SelectionImplIf::description() const
    { return TranslatedText::notext; }

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
