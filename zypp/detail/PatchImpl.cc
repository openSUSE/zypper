/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/PatchImpl.cc
 *
*/
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/detail/PatchImpl.h"
#include "zypp/Patch.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PatchImpl
    //
    ///////////////////////////////////////////////////////////////////

    /** Default ctor */
    PatchImpl::PatchImpl( const ResName & name_r,
			  const Edition & edition_r,
			  const Arch & arch_r )
    : ResolvableImpl (ResKind ("patch"),
		      ResName (name_r),
		      Edition (edition_r),
		      Arch (arch_r))
    {
    }

    /** Dtor */
    PatchImpl::~PatchImpl()
    {
    }

    bool PatchImpl::interactive () {
      if (_reboot_needed)
	return true;
      atom_list not_installed = not_installed_atoms ();
      for (atom_list::iterator it = not_installed.begin ();
	it != not_installed.end ();
	it++)
      {
	if ((std::string)((*it)->kind ()) == "message")
	// FIXME package with license
	{
	  return true;
	}
      }
      return false;
    }

    atom_list PatchImpl::not_installed_atoms () {
      atom_list ret;
      for (atom_list::iterator it = _atoms.begin ();
	it != _atoms.end ();
	it++)
      {
	if (false) // FIXME check if atom/resolvable is not installed
	{
	  ret.push_back (*it);
	}
      }
      return ret;
    }

    bool PatchImpl::any_atom_selected () {
      for (atom_list::iterator it = _atoms.begin ();
	it != _atoms.end ();
	it++)
      {
	if (false) // FIXME check if atom/resolvable is selected
	{
	  return true;
	}
      }
      return false;
    }

    void PatchImpl::mark_atoms_to_freshen (bool freshen) {
      for (atom_list::iterator it = _atoms.begin ();
	it != _atoms.end ();
	it++)
      {
	// TODO mark the resolvable to be or not to be freshed
      }
    }
    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
