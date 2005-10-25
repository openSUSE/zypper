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

#include <list>

#include "zypp/Resolvable.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////
    DEFINE_PTR_TYPE(PatchImpl)
    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  DEFINE_PTR_TYPE(Patch)

  #define atom_list std::list<ResolvablePtr>

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Patch
  //
  /** Class representing a patch */
  class Patch : public Resolvable
  {
  public:
    /** Default ctor */
    Patch( detail::PatchImplPtr impl_r );
    /** Dtor */
    ~Patch();
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
    /** The list of all atoms building the patch */
    atom_list atoms() const;
    /** Is the patch installation interactive? (does it need user input?) */
    bool interactive();
    // TODO add comments and reevaluate the need for functions below
    void mark_atoms_to_freshen(bool freshen);
    bool any_atom_selected();
    void select(); // TODO parameter to specify select/unselect or another function
  private:
    /** Pointer to implementation */
    detail::PatchImplPtr _pimpl;
  };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PATCH_H
