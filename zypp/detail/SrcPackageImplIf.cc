/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/SrcPackageImplIf.cc
 *
*/
#include "zypp/detail/SrcPackageImplIf.h"
#include <iostream>

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ByteCount SrcPackageImplIf::downloadSize() const
    { return location().downloadSize(); }

    unsigned SrcPackageImplIf::mediaNr() const
    { return location().medianr(); }

    /////////////////////////////////////////////////////////////////
    // Default implementation of SrcPackageImplIf attributes,
    // as far as resonable.
    /////////////////////////////////////////////////////////////////

    OnMediaLocation SrcPackageImplIf::location() const
    { return OnMediaLocation(); }

    DiskUsage SrcPackageImplIf::diskusage() const
    { return DiskUsage(); }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
