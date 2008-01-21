/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Selection.cc
 *
*/
#include "zypp/Selection.h"
#include "zypp/TranslatedText.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE(Selection);

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Selection::Selection
  //	METHOD TYPE : Ctor
  //
  Selection::Selection( const sat::Solvable & solvable_r )
  : ResObject( solvable_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Selection::~Selection
  //	METHOD TYPE : Dtor
  //
  Selection::~Selection()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	Selection interface forwarded to implementation
  //
  ///////////////////////////////////////////////////////////////////

    /** selection category */
    Label Selection::category() const
    { return Label(); }

    /** selection visibility (for hidden selections) */
    bool Selection::visible() const
    { return true; }

    /** selection presentation order */
    Label Selection::order() const
    { return Label(); }

    const std::set<std::string> Selection::install_packages( const Locale & lang) const
    { return std::set<std::string>(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
