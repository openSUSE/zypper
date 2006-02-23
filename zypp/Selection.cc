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
#include <iostream>

#include "zypp/Selection.h"
#include "zypp/TranslatedText.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE(Selection);
  
  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Selection::Selection
  //	METHOD TYPE : Ctor
  //
  Selection::Selection( const NVRAD & nvrad_r )
  : ResObject( TraitsType::kind, nvrad_r )
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

    /** selection summary (FIXME: localized) */
    Label Selection::summary() const
    {
	return pimpl().summary().text();    
    }

    /** */
    Text Selection::description() const
    {
	return pimpl().description().text();    
    }

    /** selection category */
    Label Selection::category() const
    {
	return pimpl().category();    
    }

    /** selection visibility (for hidden selections) */
    bool Selection::visible() const
    {
	return pimpl().visible();    
    }

    /** selection presentation order */
    Label Selection::order() const
    {
	return pimpl().order();    
    }

    const std::set<std::string> Selection::install_packages( const Locale & lang) const
    {
      return pimpl().install_packages();
    }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
