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
#include "zypp/Package.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////
    IMPL_PTR_TYPE(PatchImpl)

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PatchImpl
    //
    ///////////////////////////////////////////////////////////////////

    /** Default ctor */
    PatchImpl::PatchImpl( const std::string & name_r,
			  const Edition & edition_r,
			  const Arch & arch_r )
    : ResolvableImpl (ResKind ("patch"),
		      name_r,
		      Edition (edition_r),
		      Arch (arch_r))
    {
    }

    /** Dtor */
    PatchImpl::~PatchImpl()
    {
    }

    std::string PatchImpl::id () const
    {
      return _patch_id;
    }
    unsigned int PatchImpl::timestamp () const
    {
      return _timestamp;
    }

    std::string PatchImpl::summary () const
    {
      return _summary;
    }

    std::list<std::string> PatchImpl::description () const
    {
      return _description;
    }

    std::string PatchImpl::category () const
    {
      return _category;
    }

    bool PatchImpl::reboot_needed () const
    {
      return _reboot_needed;
    }

    bool PatchImpl::affects_pkg_manager () const
    {
      return _affects_pkg_manager;
    }

    bool PatchImpl::interactive () {
      if (_reboot_needed)
      {
	DBG << "Patch needs reboot" << endl;
	return true;
      }
      atom_list not_installed = not_installed_atoms ();
      for (atom_list::iterator it = not_installed.begin ();
	it != not_installed.end ();
	it++)
      {
	if ((std::string)((*it)->kind ()) == "message")
	{
//	  DBG << "Patch contains a message" << endl;
	  return true;
	}
	if ((std::string)((*it)->kind ()) == "package")
	{
				 // Resolvable*
				  // Resolvable
				   // ResolvablePtr
	  Package* p = (Package*)&**it;
	  // FIXME use the condition
//	  if (p->licenseToConfirm() != "")
	  if (false)
	  {
//	    DBG << "Package has a license to be shown to user" << endl;
	    return true;
	  }
	}
      }
      return false;
    }

    atom_list PatchImpl::all_atoms () {
      return _atoms;
    }

    atom_list PatchImpl::not_installed_atoms () {
      atom_list ret;
      for (atom_list::iterator it = _atoms.begin ();
	it != _atoms.end ();
	it++)
      {
	if (true) // FIXME check if atom/resolvable is not installed
	{
	  ret.push_back (*it);
	}
      }
      return ret;
    }

// TODO check necessarity of functions below

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
