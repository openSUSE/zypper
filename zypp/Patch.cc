/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Patch.cc
 *
*/
#include "zypp/Patch.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Patch::Patch
  //	METHOD TYPE : Ctor
  //
  Patch::Patch( const std::string & name_r,
                const Edition & edition_r,
                const Arch & arch_r )
  : ResObject( ResKind("Patch"), name_r, edition_r, arch_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Patch::~Patch
  //	METHOD TYPE : Dtor
  //
  Patch::~Patch()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	Patch interface forwarded to implementation
  //
  ///////////////////////////////////////////////////////////////////

  std::string Patch::id() const
  { return pimpl().id(); }

  unsigned int Patch::timestamp() const
  { return pimpl().timestamp(); }

  std::string Patch::category() const
  { return pimpl().category(); }

  bool Patch::reboot_needed() const
  { return pimpl().reboot_needed(); }

  bool Patch::affects_pkg_manager() const
  { return pimpl().affects_pkg_manager(); }

  Patch::AtomList Patch::atoms()
  { return pimpl().all_atoms(); }

  bool Patch::interactive()
  { return pimpl().interactive(); }

  void Patch::mark_atoms_to_freshen(bool freshen)
  { pimpl().mark_atoms_to_freshen(freshen); }

  bool Patch::any_atom_selected()
  { return pimpl().any_atom_selected(); }

  void Patch::select()
  { pimpl().mark_atoms_to_freshen(true); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
