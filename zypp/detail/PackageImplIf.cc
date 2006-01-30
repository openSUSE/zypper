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
#include <iostream>

using namespace std;

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

      Changelog PackageImplIf::changelog() const
      { return Changelog(); }

      Pathname PackageImplIf::location() const
      { return Pathname(); }

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

      ByteCount PackageImplIf::sourcesize() const
      { return ByteCount(); }

      ByteCount PackageImplIf::archivesize() const
      { return ByteCount(); }

      std::list<std::string> PackageImplIf::authors() const
      { return std::list<std::string>(); }

      std::list<std::string> PackageImplIf::filenames() const
      { return std::list<std::string>(); }

      License PackageImplIf::licenseToConfirm() const
      { return License(); }

      std::list<DeltaRpm> PackageImplIf::deltaRpms() const
      { return std::list<DeltaRpm>(); }

      std::list<PatchRpm> PackageImplIf::patchRpms() const
      { return std::list<PatchRpm>(); }

      bool PackageImplIf::installOnly() const
      { return false; }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
