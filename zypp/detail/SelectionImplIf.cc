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

    std::set<std::string> SelectionImplIf::suggests() const
    { return std::set<std::string>(); }

    std::set<std::string> SelectionImplIf::recommends() const
    { return std::set<std::string>(); }

    std::set<std::string> SelectionImplIf::install_packages( const Locale & lang) const
    { return std::set<std::string>(); }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
