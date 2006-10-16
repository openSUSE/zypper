/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/store/xml/XMLPatchImpl.cc
 *
*/
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/target/store/xml/XMLPatchImpl.h"
#include "zypp/Package.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace storage
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : XMLPatchImpl
    //
    ///////////////////////////////////////////////////////////////////

    /** Default ctor */
    XMLPatchImpl::XMLPatchImpl()
    {}

    /** Dtor */
    XMLPatchImpl::~XMLPatchImpl()
    {}

    std::string XMLPatchImpl::id() const
    {
      return _patch_id;
    }
    Date XMLPatchImpl::timestamp() const
    {
      return _timestamp;
    }

    std::string XMLPatchImpl::category() const
    {
      return _category;
    }

    bool XMLPatchImpl::reboot_needed() const
    {
      return _reboot_needed;
    }

    bool XMLPatchImpl::affects_pkg_manager() const
    {
      return _affects_pkg_manager;
    }

    bool XMLPatchImpl::interactive() const {
      if (_reboot_needed)
      {
	DBG << "Patch needs reboot" << endl;
	return true;
      }
      AtomList not_installed = not_installed_atoms();
      for (AtomList::const_iterator it = not_installed.begin();
	it != not_installed.end();
	it++)
      {
	if ((*it)->kind() == "Message")
	{
//	  DBG << "Patch contains a message" << endl;
	  return true;
	}
	if ((*it)->kind() == "Package")
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

    XMLPatchImpl::AtomList XMLPatchImpl::all_atoms() const {
      return _atoms;
    }

    XMLPatchImpl::AtomList XMLPatchImpl::not_installed_atoms() const {
      AtomList ret;
      for (AtomList::const_iterator it = _atoms.begin();
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

    bool XMLPatchImpl::any_atom_selected() const {
      for (AtomList::const_iterator it = _atoms.begin();
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

    void XMLPatchImpl::mark_atoms_to_freshen( bool freshen ) {
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
