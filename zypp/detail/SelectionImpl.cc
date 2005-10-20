/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/SelectionImpl.cc
 *
*/
#include <iostream>

#include "zypp/detail/SelectionImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : SelectionImpl::SelectionImpl
    //	METHOD TYPE : Ctor
    //
    SelectionImpl::SelectionImpl( const std::string & name_r,
                                  const Edition & edition_r,
                                  const Arch & arch_r )
    : ResolvableImpl( ResKind("selection"), name_r, edition_r, arch_r )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : SelectionImpl::~SelectionImpl
    //	METHOD TYPE : Dtor
    //
    SelectionImpl::~SelectionImpl()
    {}

    std::string SelectionImpl::summary() const
    { return std::string(); }

    std::list<std::string> SelectionImpl::description() const
    { return std::list<std::string>(); }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
