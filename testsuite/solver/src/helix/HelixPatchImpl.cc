/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/solver/temporary/HelixPatchImpl.cc
 *
*/

#include "HelixPatchImpl.h"
#include "zypp/base/String.h"
#include "zypp/base/Logger.h"

using namespace std;
using namespace zypp::detail;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : HelixPatchImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
HelixPatchImpl::HelixPatchImpl (const zypp::HelixParser & parsed)
    : _size_installed(parsed.installedSize)
{
}

ByteCount HelixPatchImpl::size() const
{ return _size_installed; }

      /** Patch ID */
std::string HelixPatchImpl::id() const 
{ return ""; }

      /** Patch time stamp */
unsigned int HelixPatchImpl::timestamp() const 
{ return 0; }

      /** Patch category (recommended, security,...) */
std::string HelixPatchImpl::category() const 
{ return ""; }

      /** Does the system need to reboot to finish the update process? */
bool HelixPatchImpl::reboot_needed() const 
{ return false; }

      /** Does the patch affect the package manager itself? */
bool HelixPatchImpl::affects_pkg_manager() const 
{ return false; }

      /** Is the patch installation interactive? (does it need user input?) */
bool HelixPatchImpl::interactive() const 
{ return false; }

      /** The list of all atoms building the patch */
PatchImplIf::AtomList HelixPatchImpl::all_atoms() const
{ return AtomList(); }

      /** The list of those atoms which have not been installed */
PatchImplIf::AtomList HelixPatchImpl::not_installed_atoms() const
{ return AtomList(); }


// TODO check necessarity of functions below
void HelixPatchImpl::mark_atoms_to_freshen(bool freshen) 
{ return; }

bool HelixPatchImpl::any_atom_selected() const
{ return false; }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
