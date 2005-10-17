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

#include <iosfwd>

#include "zypp/detail/PatchImpl.h"
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

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Patch
  //
  /** */
  class Patch : public Resolvable
  {
  public:
    /** Default ctor */
    Patch( detail::PatchImplPtr impl_r );
    /** Dtor */
    ~Patch();
  public:
    bool interactive ();
    std::string do_script ();
    std::string undo_script ();
    bool undo_available ();
    std::string category ();
    std::string description ();
    std::string summary ();
    void mark_atoms_to_freshen (bool freshen);
    bool any_atom_selected ();
    void select (); // TODO parameter to specify select/unselect or another function
  private:
    /** Pointer to implementation */
    detail::PatchImplPtr _pimpl;
  };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PATCH_H
