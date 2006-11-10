/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/PatchImplIf.cc
 *
*/
#include "zypp/detail/PatchImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
    // Default implementation of PatchImplIf attributes,
    // as far as resonable.
    /////////////////////////////////////////////////////////////////

    std::string PatchImplIf::id() const
    { return std::string(); }

    Date PatchImplIf::timestamp() const
    { return Date(); }

    std::string PatchImplIf::category() const
    { return std::string(); }

    bool PatchImplIf::reboot_needed() const
    { return false; }

    bool PatchImplIf::affects_pkg_manager() const
    { return false; }

    ByteCount PatchImplIf::size() const
    { return ByteCount(); }

    bool PatchImplIf::interactive() const
    { return false; }

    PatchImplIf::AtomList PatchImplIf::all_atoms() const
    { return AtomList(); }

    PatchImplIf::AtomList PatchImplIf::not_installed_atoms() const
    { return AtomList(); }

    void PatchImplIf::mark_atoms_to_freshen(bool)
    { return; }

    bool PatchImplIf::any_atom_selected() const
    { return false; }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
