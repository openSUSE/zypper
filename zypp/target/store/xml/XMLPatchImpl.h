/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/XMLPatchImpl.h
 *
*/
#ifndef ZYPP_STORE_XMLPATCHIMPL_H
#define ZYPP_STORE_XMLPATCHIMPL_H

#include "zypp/detail/PatchImplIf.h"

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
    struct XMLPatchImpl : public zypp::detail::PatchImplIf
    {
      XMLPatchImpl();
      ~XMLPatchImpl();

      /** Patch ID */
      std::string id() const;
      /** Patch time stamp */
      unsigned int timestamp() const;
      /** Patch summary */
      TranslatedText summary() const;
      /** Patch description */
      TranslatedText description() const;
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
    

      /** Patch ID */
      std::string _patch_id;
      /** Patch time stamp */
      int _timestamp;
      /** Patch summary */
      TranslatedText _summary;
      /** Patch description */
      TranslatedText _description;
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
  } // namespace storage
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PATCHIMPL_H
