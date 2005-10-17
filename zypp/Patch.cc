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
#include "zypp/detail/ResolvableImpl.h"
#include "zypp//Resolvable.h"

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
  //	METHOD NAME : Patch::interactive
  //	Check whether patch can be applied only interactivly
  //
  bool Patch::interactive ()
  {
    return _pimpl->interactive ();
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Patch::category
  //	Get the category of the patch
  //
  std::string Patch::category ()
  {
    return _pimpl->_category;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Patch::summary
  //	Get the patch summary
  //
  std::string Patch::summary ()
  {
    return _pimpl->_summary["en"];
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Patch::description
  //	Get the patch description
  //
  std::string Patch::description ()
  {
    return _pimpl->_description["en"];
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
