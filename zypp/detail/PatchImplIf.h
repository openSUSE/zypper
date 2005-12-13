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
#include "zypp/Resolvable.h"

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
    /** Abstact Patch implementation interface.
     */
    class PatchImplIf : public ResObjectImplIf
    {
    public:
      typedef Patch ResType;

    public:
      typedef std::list<Resolvable::Ptr> AtomList;

    public:
      /** Patch ID */
      virtual std::string id() const = 0;
      /** Patch time stamp */
      virtual unsigned int timestamp() const = 0;
      /** Patch category (recommended, security,...) */
      virtual std::string category() const = 0;
      /** Does the system need to reboot to finish the update process? */
      virtual bool reboot_needed() const = 0;
      /** Does the patch affect the package manager itself? */
      virtual bool affects_pkg_manager() const = 0;
      /** */
      virtual ByteCount size() const;

      /** Is the patch installation interactive? (does it need user input?) */
      virtual bool interactive() = 0;
      /** The list of all atoms building the patch */
      virtual AtomList all_atoms() = 0;
      /** The list of those atoms which have not been installed */
      virtual AtomList not_installed_atoms() = 0;

// TODO check necessarity of functions below
      virtual void mark_atoms_to_freshen(bool freshen) = 0;
      virtual bool any_atom_selected() = 0;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PATCHIMPLIF_H
