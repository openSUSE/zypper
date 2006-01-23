/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/PatchImpl.h
 *
*/
#ifndef ZYPP_DETAIL_PATCHIMPL_H
#define ZYPP_DETAIL_PATCHIMPL_H

#include "zypp/detail/PatchImplIf.h"

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
    /** Class representing a patch
     * \todo Get rid of string based ResKind detection in e.g.
     * PatchImpl::interactive.
    */
    class PatchImpl : public PatchImplIf
    {
    public:
      /** Default ctor */
      PatchImpl();
      /** Dtor */
      ~PatchImpl();

    public:
      /** Patch ID */
      std::string id() const;
      /** Patch time stamp */
      unsigned int timestamp() const;
      /** Patch summary */
      std::string summary() const;
      /** Patch description */
      std::list<std::string> description() const;
      /** Patch category (recommended, security,...) */
      std::string category() const;
      /** Does the system need to reboot to finish the update process? */
      bool reboot_needed() const;
      /** Does the patch affect the package manager itself? */
      bool affects_pkg_manager() const;

      /** Is the patch installation interactive? (does it need user input?) */
      bool interactive() const;
      /** The list of all atoms building the patch */
      AtomList all_atoms() const;
      /** The list of those atoms which have not been installed */
      AtomList not_installed_atoms() const;

// TODO check necessarity of functions below
      bool any_atom_selected() const;
      void mark_atoms_to_freshen(bool freshen);
    protected:
      /** Patch ID */
      std::string _patch_id;
      /** Patch time stamp */
      int _timestamp;
      /** Patch summary */
      std::string _summary;
      /** Patch description */
      std::list<std::string> _description;
      /** Patch category (recommended, security,...) */
      std::string _category;
      /** Does the system need to reboot to finish the update process? */
      bool _reboot_needed;
      /** Does the patch affect the package manager itself? */
      bool _affects_pkg_manager;
      /** The list of all atoms building the patch */
      AtomList _atoms;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PATCHIMPL_H
