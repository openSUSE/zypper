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
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/Patch.h"
#include "zypp/detail/PatchImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Patch::Patch
  //	METHOD TYPE : Ctor
  //
  Patch::Patch( detail::PatchImplPtr impl_r )
  : Resolvable (impl_r)
  , _pimpl( impl_r )
  {
  }


  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Patch::~Patch
  //	METHOD TYPE : Dtor
  //
  Patch::~Patch()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Patch::id
  //	Get the patch id
  //
  std::string Patch::id () const
  {
    return _pimpl->id ();
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Patch::id
  //	Get the patch id
  //
  unsigned int Patch::timestamp () const
  {
    return _pimpl->timestamp ();
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Patch::summary
  //	Get the patch summary
  //
  std::string Patch::summary () const
  {
    return _pimpl->summary ();
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Patch::description
  //	Get the patch description
  //
  std::list<std::string> Patch::description () const
  {
    return _pimpl->description ();
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Patch::category
  //	Get the category of the patch
  //
  std::string Patch::category () const
  {
    return _pimpl->category ();
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Patch::reboot_needed
  //	Check whether reboot is needed to finish the patch installation
  //
  bool Patch::reboot_needed () const {
    return _pimpl->reboot_needed ();
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Patch::affects_pkg_manager
  //	Check whether the patch installation affects package manager
  //    (and it should be restarted after patch installation)
  //
  bool Patch::affects_pkg_manager () const {
    return _pimpl->affects_pkg_manager ();
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Patch::atoms
  //    Get the list of all atoms building the patch
  //
  atom_list Patch::atoms () const {
    return _pimpl->all_atoms ();
  }
  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Patch::interactive
  //	Check whether patch can be applied only interactivly
  //
  bool Patch::interactive ()
  {
    return _pimpl->interactive ();
  }

  void Patch::mark_atoms_to_freshen (bool freshen)
  {
    _pimpl->mark_atoms_to_freshen (freshen);
  }
  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Patch::any_atom_selected
  //	Check whether there is at least one atom of the solution selected
  //
  bool Patch::any_atom_selected ()
  {
    return _pimpl->any_atom_selected ();
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Patch::select
  //	Mark all atoms to be updated (set them as freshen) and mark
  //    the patch itself as selected for being installed/updated
  //
  // FIXME to be changed to inherit Resolvable's method
  void Patch::select ()
//  : Resolvable::select ()
  {
    mark_atoms_to_freshen (true); // FIXME
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
