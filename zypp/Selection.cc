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
  Selection::Selection( const std::string & name_r,
                        const Edition & edition_r,
                        const Arch & arch_r )
  : ResObject( ResTraits<Self>::_kind, name_r, edition_r, arch_r )
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


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
