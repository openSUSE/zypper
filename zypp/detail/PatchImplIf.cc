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

    bool PatchImplIf::interactive() const
    {
      if ( reboot_needed()
           || ! licenseToConfirm().empty() )
        {
          return true;
        }

      AtomList atoms = all_atoms();
      for ( AtomList::const_iterator it = atoms.begin(); it != atoms.end(); it++)
        {
          if (    isKind<Message>( *it )
               || ! licenseToConfirm().empty() )
            {
              return true;
            }
        }

      return false;
    }

    PatchImplIf::AtomList PatchImplIf::all_atoms() const
    { return AtomList(); }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
