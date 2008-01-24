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

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(Patch);

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
    typedef Patch                    Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;

  public:
    typedef std::list<ResObject::Ptr> AtomList;

  public:
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
    /** The list of all atoms building the patch */
    AtomList atoms() const;
    /** Is the patch installation interactive? (does it need user input?) */
    bool interactive() const;

  protected:
    friend Ptr make<Self>( const sat::Solvable & solvable_r );
    /** Ctor */
    Patch( const sat::Solvable & solvable_r );
    /** Dtor */
    virtual ~Patch();
  };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PATCH_H
