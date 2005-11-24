/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/PackageImplIf.cc
 *
*/
#include "zypp/detail/PackageImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
    // Default implementation of PackageImplIf attributes,
    // as far as resonable.
    /////////////////////////////////////////////////////////////////

      Date PackageImplIf::buildtime() const
      { return Date(); }

      std::string PackageImplIf::buildhost() const
      { return std::string(); }

      Date PackageImplIf::installtime() const
      { return Date(); }

      std::string PackageImplIf::distribution() const
      { return std::string(); }

      Vendor PackageImplIf::vendor() const
      { return Vendor(); }

      Label PackageImplIf::license() const
      { return Label(); }

      std::string PackageImplIf::packager() const
      { return std::string(); }

      PackageGroup PackageImplIf::group() const
      { return PackageGroup(); }

      Text PackageImplIf::changelog() const
      { return Text(); }

      std::string PackageImplIf::url() const
      { return std::string(); }

      std::string PackageImplIf::os() const
      { return std::string(); }

      std::list<std::string> PackageImplIf::prein() const
      { return std::list<std::string>(); }

      std::list<std::string> PackageImplIf::postin() const
      { return std::list<std::string>(); }

      std::list<std::string> PackageImplIf::preun() const
      { return std::list<std::string>(); }

      std::list<std::string> PackageImplIf::postun() const
      { return std::list<std::string>(); }

      FSize PackageImplIf::sourcesize() const
      { return FSize(); }

      FSize PackageImplIf::archivesize() const
      { return FSize(); }

      std::list<std::string> PackageImplIf::authors() const
      { return std::list<std::string>(); }

      std::list<std::string> PackageImplIf::filenames() const
      { return std::list<std::string>(); }

      License PackageImplIf::licenseToConfirm() const
      { return License(); }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
