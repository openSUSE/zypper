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

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

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
	return pimpl().summary();    
    }

    /** */
    Text Selection::description() const
    {
	return pimpl().description();    
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

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
