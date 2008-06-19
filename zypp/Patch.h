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

#include "zypp/sat/SolvAttr.h"
#include "zypp/ResObject.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////


  DEFINE_PTR_TYPE(Patch);


  /**
   * Class representing a patch.
   *
   * A patch represents a specific problem that
   * can be fixed by pulling in the patch dependencies.
   *
   * Patches can be marked for installation but their
   * installation is a no-op.
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

      enum Category {
        CAT_OTHER,
        CAT_YAST,
        CAT_SECURITY,
        CAT_RECOMMENDED,
        CAT_OPTIONAL,
        CAT_DOCUMENT
      };

    public:
      /**
       * Issue date time. For now it is the same as
       * \ref buildtime().
       */
      Date timestamp() const
      { return buildtime(); }

      /**
       * Patch category (recommended, security,...)
       */
      std::string category() const;

      /** Patch category as enum of wellknown categories.
       * Unknown values are mapped to \ref CAT_OTHER.
       */
      Category categoryEnum() const;

      /**
       * Does the system need to reboot to finish the update process?
       */
      bool rebootSuggested() const;

      /**
       * Does the patch affect the package manager itself?
       * restart is suggested then
       */
      bool restartSuggested() const;

      /**
       * \short Information or warning to be displayed to the user
       */
      std::string message( const Locale & lang_r = Locale() ) const;

      /**
       * Use \ref rebootSuggested()
       */
      ZYPP_DEPRECATED bool reboot_needed() const
      { return rebootSuggested(); }

      /**
       * Use \ref restartSuggested()
       */
      ZYPP_DEPRECATED bool affects_pkg_manager() const
      { return restartSuggested(); }

      /**
       * Is the patch installation interactive? (does it need user input?)
       */
      bool interactive() const;

    public:
      /**
       * The collection of packages associated with this patch.
       */
      Contents contents() const;

    public:

      /** Query class for Patch issue references */
      class ReferenceIterator;
      /**
       * Get an iterator to the beginning of the patch
       * references. \see Patch::ReferenceIterator
       */
      ReferenceIterator referencesBegin() const;
      /**
       * Get an iterator to the end of the patch
       * references. \see Patch::ReferenceIterator
       */
      ReferenceIterator referencesEnd() const;


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


  /**
   * Query class for Patch issue references
   * like bugzilla and security issues the
   * patch is supposed to fix.
   *
   * The iterator does not provide a dereference
   * operator so you can do * on it, but you can
   * access the attributes of each patch issue reference
   * directly from the iterator.
   *
   * \code
   * for ( Patch::ReferenceIterator it = patch->referencesBegin();
   *       it != patch->referencesEnd();
   *       ++it )
   * {
   *   cout << it.href() << endl;
   * }
   * \endcode
   *
   */
  class Patch::ReferenceIterator : public boost::iterator_adaptor<
      Patch::ReferenceIterator           // Derived
      , sat::LookupAttr::iterator        // Base
      , int                              // Value
      , boost::forward_traversal_tag     // CategoryOrTraversal
      , int
  >
  {
    public:
      ReferenceIterator() {}
      explicit ReferenceIterator( const sat::Solvable & val_r );

      /**
       * The id of the reference. For bugzilla entries
       * this is the bug number as a string.
       */
      std::string id() const;
      /**
       * Url or pointer where to find more information
       */
      std::string href() const;
      /**
       * Title describing the issue
       */
      std::string title() const;
      /**
       * Type of the reference. For example
       * "bugzilla"
       */
      std::string type() const;
    private:
      friend class boost::iterator_core_access;

      int dereference() const { return 0; }
      void increment();
    private:
      sat::LookupAttr::iterator _hrefit;
      sat::LookupAttr::iterator _titleit;
      sat::LookupAttr::iterator _typeit;
  };

  inline Patch::ReferenceIterator Patch::referencesBegin() const
  { return ReferenceIterator(satSolvable()); }

  inline Patch::ReferenceIterator Patch::referencesEnd() const
  { return ReferenceIterator(); }

  /////////////////////////////////////////////////////////////////

} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PATCH_H
