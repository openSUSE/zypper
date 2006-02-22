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

    /////////////////////////////////////////////////////////////////
    // Default implementation of SrcPackageImplIf attributes,
    // as far as resonable.
    /////////////////////////////////////////////////////////////////

      ByteCount SrcPackageImplIf::archivesize() const
      { return ByteCount(); }

      DiskUsage SrcPackageImplIf::diskusage() const
      { return DiskUsage(); }      

      Pathname SrcPackageImplIf::location() const
      { return Pathname(); }

      unsigned SrcPackageImplIf::mediaId() const
      { return 1; }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
