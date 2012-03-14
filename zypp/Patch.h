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

#include "zypp/base/Flags.h"
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

      /**
       * Flags defining if and why this
       * patch is interactive.
       */
      enum InteractiveFlag {
        NoFlags = 0x0000,
        Reboot  = 0x0001,
        Message = 0x0002,
        License = 0x0004
      };
      ZYPP_DECLARE_FLAGS(InteractiveFlags, InteractiveFlag);

      /**
       * \brief Possible severity levels for (security) patches.
       * Metadata string values are mapped to this enum to ease
       * computations. For a string representation call
       * \ref asSring( const Patch::SeverityFlag & ).
       */
      enum SeverityFlag {
	SEV_NONE	= 0,	//!< no value specified
	SEV_OTHER	= 1,	//!< unknown value specified
	SEV_LOW		= 1<<1,	//!< Low
	SEV_MODERATE	= 1<<2,	//!< Moderate
	SEV_IMPORTANT	= 1<<3,	//!< Important
	SEV_CRITICAL	= 1<<4	//!< Critical
      };
      ZYPP_DECLARE_FLAGS(SeverityFlags, SeverityFlag);

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
       * Severity string as specified in metadata.
       * For use in computaions see \ref severityFlag.
       */
      std::string severity() const;

      /**
       * Severity string mapped to an enum.
       * Unknown string values are mapped to \ref SEV_OTHER
       */
      SeverityFlag severityFlag() const;

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
       * Does the patch needs the user to relogin to take effect?
       * relogin is suggested then
       */
      bool reloginSuggested() const;

      /**
       * \short Information or warning to be displayed to the user
       */
      std::string message( const Locale & lang_r = Locale() ) const;

      /**
       * Get the InteractiveFlags of this Patch
       */
      InteractiveFlags interactiveFlags() const;

      /**
       * Is the patch still interactive when ignoring this flags?
       */
      bool interactiveWhenIgnoring( InteractiveFlags flags_r = NoFlags ) const;

      /**
       * Is the patch installation interactive? (does it need user input?)
       *
       * For security reasons patches requiring a reboot are not
       * installed in an unattended mode. They are considered to be
       * \c interactive so the user gets informed about the need for
       * reboot. \a ignoreRebootFlag_r may be used to explicitly turn
       * off this behavior and include those patches (unless they actually
       * contain interactive components as well, like messages or licenses).
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

    protected:
      friend Ptr make<Self>( const sat::Solvable & solvable_r );
      /** Ctor */
      Patch( const sat::Solvable & solvable_r );
      /** Dtor */
      virtual ~Patch();
  };
  ZYPP_DECLARE_OPERATORS_FOR_FLAGS(Patch::InteractiveFlags);
  ZYPP_DECLARE_OPERATORS_FOR_FLAGS(Patch::SeverityFlags);

  /** \relates Patch::SeverityFlag string representation.*/
  std::string asString( const Patch::SeverityFlag & obj );

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
      , int                              // Reference
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
  };

  inline Patch::ReferenceIterator Patch::referencesBegin() const
  { return ReferenceIterator(satSolvable()); }

  inline Patch::ReferenceIterator Patch::referencesEnd() const
  { return ReferenceIterator(); }

  /////////////////////////////////////////////////////////////////

} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PATCH_H
