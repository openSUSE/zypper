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
#include "zypp/Package.h"

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
    PatchImpl::PatchImpl()
    {}

    /** Dtor */
    PatchImpl::~PatchImpl()
    {}

    std::string PatchImpl::id() const
    {
      return _patch_id;
    }
    unsigned int PatchImpl::timestamp() const
    {
      return _timestamp;
    }

    std::string PatchImpl::summary() const
    {
      return _summary;
    }

    std::list<std::string> PatchImpl::description() const
    {
      return _description;
    }

    std::string PatchImpl::category() const
    {
      return _category;
    }

    bool PatchImpl::reboot_needed() const
    {
      return _reboot_needed;
    }

    bool PatchImpl::affects_pkg_manager() const
    {
      return _affects_pkg_manager;
    }

    bool PatchImpl::interactive() {
      if (_reboot_needed)
      {
	DBG << "Patch needs reboot" << endl;
	return true;
      }
      AtomList not_installed = not_installed_atoms();
      for (AtomList::iterator it = not_installed.begin();
	it != not_installed.end();
	it++)
      {
	if ((std::string)((*it)->kind()) == "message")
	{
//	  DBG << "Patch contains a message" << endl;
	  return true;
	}
	if ((std::string)((*it)->kind()) == "package")
	{
				 // Resolvable*
				  // Resolvable
				   // ResolvablePtr


          // <ma> never do somthing like this!!!
//	  Package* p = (Package*)&**it;
          //
          // (*it) is a ResolvablePtr




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

    PatchImpl::AtomList PatchImpl::all_atoms() {
      return _atoms;
    }

    PatchImpl::AtomList PatchImpl::not_installed_atoms() {
      AtomList ret;
      for (AtomList::iterator it = _atoms.begin();
	it != _atoms.end();
	it++)
      {
	if (true) // FIXME check if atom/resolvable is not installed
	{
	  ret.push_back(*it);
	}
      }
      return ret;
    }

// TODO check necessarity of functions below

    bool PatchImpl::any_atom_selected() {
      for (AtomList::iterator it = _atoms.begin();
	it != _atoms.end();
	it++)
      {
	if (false) // FIXME check if atom/resolvable is selected
	{
	  return true;
	}
      }
      return false;
    }

    void PatchImpl::mark_atoms_to_freshen( bool freshen ) {
      for (AtomList::iterator it = _atoms.begin();
	it != _atoms.end();
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
