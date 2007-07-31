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

    ByteCount PackageImplIf::downloadSize() const
    { return location().downloadSize(); }

    unsigned PackageImplIf::mediaNr() const
    { return location().medianr(); }

    /////////////////////////////////////////////////////////////////
    // Default implementation of PackageImplIf attributes,
    // as far as resonable.
    /////////////////////////////////////////////////////////////////

      std::string PackageImplIf::buildhost() const
      { return std::string(); }

      std::string PackageImplIf::distribution() const
      { return std::string(); }

      Label PackageImplIf::license() const
      { return Label(); }

      std::string PackageImplIf::packager() const
      { return std::string(); }

      PackageGroup PackageImplIf::group() const
      { return PackageGroup(); }

      PackageImplIf::Keywords PackageImplIf::keywords() const
      { return std::set<PackageKeyword>(); }

      Changelog PackageImplIf::changelog() const
      { return Changelog(); }

      std::string PackageImplIf::url() const
      { return std::string(); }

      std::string PackageImplIf::os() const
      { return std::string(); }

      Text PackageImplIf::prein() const
      { return Text(); }

      Text PackageImplIf::postin() const
      { return Text(); }

      Text PackageImplIf::preun() const
      { return Text(); }

      Text PackageImplIf::postun() const
      { return Text(); }

      ByteCount PackageImplIf::sourcesize() const
      { return ByteCount(); }

      OnMediaLocation PackageImplIf::location() const
      { return OnMediaLocation(); }

      DiskUsage PackageImplIf::diskusage() const
      { return DiskUsage(); }

      std::list<std::string> PackageImplIf::authors() const
      { return std::list<std::string>(); }

      std::list<std::string> PackageImplIf::filenames() const
      { return std::list<std::string>(); }

      std::list<PackageImplIf::DeltaRpm> PackageImplIf::deltaRpms() const
      { return std::list<DeltaRpm>(); }

      std::list<PackageImplIf::PatchRpm> PackageImplIf::patchRpms() const
      { return std::list<PatchRpm>(); }

      std::string PackageImplIf::sourcePkgName() const
      { return std::string(); }

      Edition PackageImplIf::sourcePkgEdition() const
      { return Edition(); }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
