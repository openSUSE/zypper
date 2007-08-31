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
      virtual std::string id() const;
      /** Patch time stamp */
      virtual Date timestamp() const;
      /** Patch summary */
      virtual TranslatedText summary() const;
      /** Patch description */
      virtual TranslatedText description() const;
      /** Patch category (recommended, security,...) */
      virtual std::string category() const;
      /** Does the system need to reboot to finish the update process? */
      virtual bool reboot_needed() const;
      /** Does the patch affect the package manager itself? */
      virtual bool affects_pkg_manager() const;

    protected:
      /** Patch ID */
      std::string _patch_id;
      /** Patch time stamp */
      Date _timestamp;
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
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PATCHIMPL_H
