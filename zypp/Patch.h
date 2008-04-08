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
      typedef sat::SolvableSet Contents;

    public:
      /** Patch time stamp */
      Date timestamp() const
      { return buildtime(); }
      /** Patch category (recommended, security,...) */
      std::string category() const;
      /** Does the system need to reboot to finish the update process? */
      bool reboot_needed() const;
      /** Does the patch affect the package manager itself? */
      bool affects_pkg_manager() const;
      /** Is the patch installation interactive? (does it need user input?) */
      bool interactive() const;

    public:
      /** The collection of packages associated with this patch. */
      Contents contents() const;

    public:
      /** Patch ID
       * \deprecated Seems to be unsused autobuild interal data?
      */
      ZYPP_DEPRECATED std::string id() const
      { return std::string(); }

      /** The list of all atoms building the patch
       * \deprecated  Try contents().
      */
      typedef std::list<ResObject::Ptr> AtomList;
      ZYPP_DEPRECATED AtomList atoms() const
      { return AtomList(); }

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
