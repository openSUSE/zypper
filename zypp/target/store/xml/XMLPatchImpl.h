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

      virtual TranslatedText summary() const
      { return _summary; }
      virtual TranslatedText description() const
      { return _description; }
      virtual TranslatedText insnotify() const
      { return _install_notify; }    
      virtual TranslatedText delnotify() const
      { return _delete_notify; }    
      virtual TranslatedText licenseToConfirm() const
      { return _license_to_confirm; }    
      virtual Vendor vendor() const
      { return _vendor; }    
      virtual ByteCount size() const
      { return _size; }    
      virtual ByteCount archivesize() const
      { return _archive_size; }    
      virtual unsigned sourceMediaNr() const
      { return 0; }    
      virtual bool installOnly() const
      { return _install_only; }    
      virtual Date buildtime() const
      { return _build_time; }    
      virtual Date installtime() const
      { return _install_time; }    
      
      /** Patch ID */
      std::string id() const;
      /** Patch time stamp */
      Date timestamp() const;
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
      Date _timestamp;
      
      TranslatedText _summary;
      TranslatedText _description;
      
      TranslatedText _install_notify;
      TranslatedText _delete_notify;
      TranslatedText _license_to_confirm;
      std::string _vendor;
      ByteCount _size;
      ByteCount _archive_size;
      bool _install_only;
      Date _build_time;
      Date _install_time;
      
      
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
