/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/PackageImpl.cc
 *
*/
#include <iostream>

#include "zypp/detail/PackageImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : PackageImpl::PackageImpl
    //	METHOD TYPE : Ctor
    //
    PackageImpl::PackageImpl( const ResName & name_r,
                              const Edition & edition_r,
                              const Arch & arch_r )
    : ResolvableImpl( ResKind("package"), name_r, edition_r, arch_r )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : PackageImpl::~PackageImpl
    //	METHOD TYPE : Dtor
    //
    PackageImpl::~PackageImpl()
    {}

    std::string PackageImpl::summary() const
    { return std::string(); }

    std::list<std::string> PackageImpl::description() const
    { return std::list<std::string>(); }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
