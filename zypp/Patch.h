/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Patch.h
 *
*/
#ifndef ZYPP_PATCH_H
#define ZYPP_PATCH_H

#include "zypp/ResObject.h"
#include "zypp/detail/PatchImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Patch
  //
  /** Class representing a patch.
   * \todo Patch::atoms can't be const, if Impl does not
   * provide a const method. Check it.
  */
  class Patch : public ResObject
  {
  public:
    typedef detail::PatchImplIf      Impl;
    typedef Patch                    Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;

  public:
    typedef Impl::AtomList AtomList;

  public:
    /** Patch ID */
    std::string id() const;
    /** Patch time stamp */
    unsigned int timestamp() const;
    /** Patch category (recommended, security,...) */
    std::string category() const;
    /** Does the system need to reboot to finish the update process? */
    bool reboot_needed() const;
    /** Does the patch affect the package manager itself? */
    bool affects_pkg_manager() const;
    /** The list of all atoms building the patch */
    AtomList atoms();
    /** Is the patch installation interactive? (does it need user input?) */
    bool interactive();
    // TODO add comments and reevaluate the need for functions below
    void mark_atoms_to_freshen(bool freshen);
    bool any_atom_selected();
    void select(); // TODO parameter to specify select/unselect or another function

  protected:
    /** Ctor */
    Patch( const NVRAD & nvrad_r );
    /** Dtor */
    virtual ~Patch();

  private:
    /** Access implementation */
    virtual Impl & pimpl() = 0;
    /** Access implementation */
    virtual const Impl & pimpl() const = 0;
  };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PATCH_H
