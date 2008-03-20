/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/Solvable.h
 *
*/
#ifndef ZYPP_SAT_SOLVABLE_H
#define ZYPP_SAT_SOLVABLE_H

#include <iosfwd>

#include "zypp/base/SafeBool.h"

#include "zypp/sat/detail/PoolMember.h"
#include "zypp/sat/SolvAttr.h"
#include "zypp/ResTraits.h"
#include "zypp/IdString.h"
#include "zypp/Edition.h"
#include "zypp/Arch.h"
#include "zypp/Dep.h"
#include "zypp/Capabilities.h"
#include "zypp/Capability.h"
#include "zypp/Locale.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Solvable
    //
    /** A \ref Solvable object within the sat \ref Pool.
     *
     * \note Unfortunately libsatsolver combines the objects kind and
     * name in a single identifier \c "pattern:kde_multimedia",
     * \b except for packages and source packes. They are not prefixed
     * by any kind string. Instead the architecture is abused to store
     * \c "src" and \c "nosrc" values.
     *
     * \ref Solvable will hide this inconsistency by treating source
     * packages as an own kind of solvable and map their arch to
     * \ref Arch_noarch.
     *
     *
     */
    class Solvable : protected detail::PoolMember,
                     private base::SafeBool<Solvable>
    {
      public:
        typedef sat::detail::SolvableIdType IdType;

      public:
        /** Default ctor creates \ref nosolvable.*/
        Solvable()
        : _id( detail::noSolvableId ) {}

        /** \ref PoolImpl ctor. */
        explicit Solvable( IdType id_r )
        : _id( id_r ) {}

      public:
        /** Represents no \ref Solvable. */
        static const Solvable nosolvable;

        /** Evaluate \ref Solvable in a boolean context (\c != \c nosolvable). */
        using base::SafeBool<Solvable>::operator bool_type;

        /** Return whether this \ref Solvable belongs to the system repo. */
        bool isSystem() const;

        /** The \ref Repository this \ref Solvable belongs to. */
        Repository repository() const;

      public:

        /**
         * returns the string attribute value for \ref attr
         * or an empty string if it does not exists.
         */
        std::string lookupStrAttribute( const SolvAttr & attr ) const;
        /** \overload Trying to look up a translated string attribute.
         *
         * Passing an empty \ref Locale will return the string for the
         * current default locale (\see \ref ZConfig::defaultTextLocale).
         *
         * Returns the translation for \c lang_r considering all fallback
         * locales. Returns an empty string if no translation is available.
        */
        std::string lookupStrAttribute( const SolvAttr & attr, const Locale & lang_r ) const;

        /**
         * returns the numeric attribute value for \ref attr
         * or 0 if it does not exists.
         */
        unsigned lookupNumAttribute( const SolvAttr & attr ) const;

        /**
         * returns the boolean attribute value for \ref attr
         * or \c false if it does not exists.
         */
        bool lookupBoolAttribute( const SolvAttr & attr ) const;

        /**
         * returns the media location: media number in \ref medianr,
         * file name as return value.  The file name is possibly prepended
         * with a subdirectory.
         */
        std::string lookupLocation( unsigned & medianr ) const;

      public:
        /** The identifier.
         * This is the solvables \ref name, \b except for packages and
         * source packes, prefixed by it's \ref kind.
         */
        IdString     ident()    const;

        ResKind      kind()     const;
        /** Test whether a Solvable is of a certain \ref ResKind. */
        bool         isKind( const ResKind & kind_r ) const;

        std::string  name()     const;
        Edition      edition()  const;
        Arch         arch()     const;

        IdString     vendor()   const;

      public:

        /** \name Access to the \ref Solvable dependencies.
         *
         * \note Prerequires are a subset of requires.
         */
        //@{
        Capabilities operator[]( Dep which_r ) const;

        Capabilities provides()    const;
        Capabilities requires()    const;
        Capabilities conflicts()   const;
        Capabilities obsoletes()   const;
        Capabilities recommends()  const;
        Capabilities suggests()    const;
        Capabilities freshens()    const;
        Capabilities enhances()    const;
        Capabilities supplements() const;
        Capabilities prerequires() const;
        //@}

      public:
        /** \name Locale support. */
        //@{
        /** Whether this \c Solvable claims to support locales. */
        bool supportsLocales() const;
        /** Whether this \c Solvable supports a specific \ref Locale. */
        bool supportsLocale( const Locale & locale_r ) const;
        /** Whether this \c Solvable supports at least one of the specified locales. */
        bool supportsLocale( const LocaleSet & locales_r ) const;
        /** Whether this \c Solvable supports at least one requested locale.
         * \see \ref Pool::setRequestedLocales
        */
        bool supportsRequestedLocales() const;
        /** Return the supported locales via locales_r. */
        void getSupportedLocales( LocaleSet & locales_r ) const;
        /** \overload */
        LocaleSet getSupportedLocales() const
        { LocaleSet ret; getSupportedLocales( ret ); return ret; }
        //@}

      public:
        /** Return next Solvable in \ref Pool (or \ref nosolvable). */
        Solvable nextInPool() const;
        /** Return next Solvable in \ref Repo (or \ref nosolvable). */
        Solvable nextInRepo() const;

      public:
        /** Expert backdoor. */
        ::_Solvable * get() const;
        /** Expert backdoor. */
        IdType id() const { return _id; }
      private:
        friend base::SafeBool<Solvable>::operator bool_type() const;
        bool boolTest() const { return get(); }
      private:
        IdType _id;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Solvable Stream output */
    std::ostream & operator<<( std::ostream & str, const Solvable & obj );

    /** \relates Solvable More verbose stream output including dependencies */
    std::ostream & dumpOn( std::ostream & str, const Solvable & obj );

    /** \relates Solvable */
    inline bool operator==( const Solvable & lhs, const Solvable & rhs )
    { return lhs.get() == rhs.get(); }

    /** \relates Solvable */
    inline bool operator!=( const Solvable & lhs, const Solvable & rhs )
    { return lhs.get() != rhs.get(); }

    /** \relates Solvable */
    inline bool operator<( const Solvable & lhs, const Solvable & rhs )
    { return lhs.get() < rhs.get(); }

    ///////////////////////////////////////////////////////////////////
    namespace detail
    { /////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : SolvableIterator
      //
      /** */
      class SolvableIterator : public boost::iterator_adaptor<
          SolvableIterator                   // Derived
          , ::_Solvable*                     // Base
          , const Solvable                   // Value
          , boost::forward_traversal_tag     // CategoryOrTraversal
          , const Solvable                   // Reference
          >
      {
        public:
          SolvableIterator()
          : SolvableIterator::iterator_adaptor_( 0 )
          {}

          explicit SolvableIterator( const Solvable & val_r )
          : SolvableIterator::iterator_adaptor_( 0 )
          { assignVal( val_r ); }

          explicit SolvableIterator( SolvableIdType id_r )
          : SolvableIterator::iterator_adaptor_( 0 )
          { assignVal( Solvable( id_r ) ); }

        private:
          friend class boost::iterator_core_access;

          Solvable dereference() const
          { return _val; }

          void increment()
          { assignVal( _val.nextInPool() ); }

        private:
          void assignVal( const Solvable & val_r )
          { _val = val_r; base_reference() = _val.get(); }

          Solvable _val;
      };
      ///////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////
    } // namespace detail
    ///////////////////////////////////////////////////////////////////

   /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////

  /** \relates sat::Solvable Test whether a \ref sat::Solvable is of a certain Kind. */
  template<class _Res>
  inline bool isKind( const sat::Solvable & solvable_r )
  { return solvable_r.isKind( ResTraits<_Res>::kind ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_SOLVABLE_H
