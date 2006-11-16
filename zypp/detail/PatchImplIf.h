/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/PatchImplIf.h
 *
*/
#ifndef ZYPP_DETAIL_PATCHIMPLIF_H
#define ZYPP_DETAIL_PATCHIMPLIF_H

#include "zypp/detail/ResObjectImplIf.h"
#include "zypp/ResObject.h"


///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Patch;

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PatchImplIf
    //
    /** Abstract Patch implementation interface.
     */
    class PatchImplIf : public ResObjectImplIf
    {
    public:
      typedef Patch ResType;

    public:
      typedef std::list<ResObject::Ptr> AtomList;

    public:
      /** Patch ID */
      virtual std::string id() const PURE_VIRTUAL;
      /** Patch time stamp */
      virtual Date timestamp() const PURE_VIRTUAL;
      /** Patch category (recommended, security,...) */
      virtual std::string category() const PURE_VIRTUAL;
      /** Does the system need to reboot to finish the update process? */
      virtual bool reboot_needed() const PURE_VIRTUAL;
      /** Does the patch affect the package manager itself? */
      virtual bool affects_pkg_manager() const PURE_VIRTUAL;
      /** */
      virtual ByteCount size() const;
      /** Is the patch installation interactive? (does it need user input?) */
      virtual bool interactive() const PURE_VIRTUAL;
      /** The list of all atoms building the patch */
      virtual AtomList all_atoms() const PURE_VIRTUAL;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PATCHIMPLIF_H
