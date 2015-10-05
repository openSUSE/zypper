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
{
  class ByteCount;
  class CheckSum;
  class CpeId;
  class Date;
  class OnMediaLocation;
  ///////////////////////////////////////////////////////////////////
  namespace sat
  {
    ///////////////////////////////////////////////////////////////////
    /// \class Solvable
    /// \brief  A \ref Solvable object within the sat \ref Pool.
    ///
    /// \note Unfortunately libsolv combines the objects kind and
    /// name in a single identifier \c "pattern:kde_multimedia",
    /// \b except for packages and source packes. They are not prefixed
    /// by any kind string. Instead the architecture is abused to store
    /// \c "src" and \c "nosrc" values.
    ///
    /// \ref Solvable will hide this inconsistency by treating source
    /// packages as an own kind of solvable and map their arch to
    /// \ref Arch_noarch.
    ///////////////////////////////////////////////////////////////////
    class Solvable : protected detail::PoolMember
    {
    public:
      typedef sat::detail::SolvableIdType IdType;

    public:
      /** Default ctor creates \ref noSolvable.*/
      Solvable()
      : _id( detail::noSolvableId )
      {}

      /** \ref PoolImpl ctor. */
      explicit Solvable( IdType id_r )
      : _id( id_r )
      {}

    public:
      /** Represents no \ref Solvable. */
      static const Solvable noSolvable;

      /** Evaluate \ref Solvable in a boolean context (\c != \c noSolvable). */
      explicit operator bool() const
      { return get(); }

    public:
      /** The identifier.
       * This is the solvables \ref name, \b except for packages and
       * source packes, prefixed by it's \ref kind.
       */
      IdString ident()const;

      /** The Solvables ResKind. */
      ResKind kind()const;

      /** Test whether a Solvable is of a certain \ref ResKind.
       * The test is far cheaper than actually retrieving and
       * comparing the \ref kind.
       */
      bool isKind( const ResKind & kind_r ) const;
      /** \overload */
      template<class TRes>
      bool isKind() const
      { return isKind( resKind<TRes>() ); }
      /** \overload Extend the test to a range of \ref ResKind. */
      template<class TIterator>
      bool isKind( TIterator begin, TIterator end ) const
      { for_( it, begin, end ) if ( isKind( *it ) ) return true; return false; }

      /** The name (without any ResKind prefix). */
      std::string name() const;

      /** The edition (version-release). */
      Edition edition() const;

      /** The architecture. */
      Arch arch() const;

      /** The vendor. */
      IdString vendor() const;

      /** The \ref Repository this \ref Solvable belongs to. */
      Repository repository() const;
      /** The repositories \ref RepoInfo. */
      RepoInfo repoInfo() const;

      /** Return whether this \ref Solvable belongs to the system repo.
       * \note This includes the otherwise hidden systemSolvable.
       */
      bool isSystem() const;

      /** Whether this is known to be installed on behalf of a user request.
       * \note This is a hint guessed by evaluating an available install history.
       * Returns \c false for non-system (uninstalled) solvables, or if no history
       * is available.
       */
      bool onSystemByUser() const;

      /** Whether different versions of this package can be installed at the same time.
       * Per default \c false. \see also \ref ZConfig::multiversion.
       */
      bool multiversionInstall() const;

      /** The items build time. */
      Date buildtime() const;

      /** The items install time (\c false if not installed). */
      Date installtime() const;

    public:
      /** String representation <tt>"ident-edition.arch"</tt> or \c "noSolvable"
       * \code
       *   product:openSUSE-11.1.x86_64
       *   autoyast2-2.16.19-0.1.src
       *   noSolvable
       * \endcode
       */
      std::string asString() const;

      /** String representation <tt>"ident-edition.arch(repo)"</tt> or \c "noSolvable" */
      std::string asUserString() const;

      /** Test whether two Solvables have the same content.
       * Basically the same name, edition, arch, vendor and buildtime.
       */
      bool identical( const Solvable & rhs ) const;

      /** Test for same name-version-release.arch */
      bool sameNVRA( const Solvable & rhs ) const
      { return( get() == rhs.get() || ( ident() == rhs.ident() && edition() == rhs.edition() && arch() == rhs.arch() ) ); }

    public:
      /** \name Access to the \ref Solvable dependencies.
       *
       * \note Prerequires are a subset of requires.
       */
      //@{
      Capabilities provides()    const;
      Capabilities requires()    const;
      Capabilities conflicts()   const;
      Capabilities obsoletes()   const;
      Capabilities recommends()  const;
      Capabilities suggests()    const;
      Capabilities enhances()    const;
      Capabilities supplements() const;
      Capabilities prerequires() const;

      /** Return \ref Capabilities selected by \ref Dep constant. */
      Capabilities dep( Dep which_r ) const
      {
	switch( which_r.inSwitch() )
	{
	  case Dep::PROVIDES_e:    return provides();    break;
	  case Dep::REQUIRES_e:    return requires();    break;
	  case Dep::CONFLICTS_e:   return conflicts();   break;
	  case Dep::OBSOLETES_e:   return obsoletes();   break;
	  case Dep::RECOMMENDS_e:  return recommends();  break;
	  case Dep::SUGGESTS_e:    return suggests();    break;
	  case Dep::ENHANCES_e:    return enhances();    break;
	  case Dep::SUPPLEMENTS_e: return supplements(); break;
	  case Dep::PREREQUIRES_e: return prerequires(); break;
	}
	return Capabilities();
      }
      /** \overload operator[] */
      Capabilities operator[]( Dep which_r ) const
      { return dep( which_r ); }


      /** Return the namespaced provides <tt>'namespace([value])[ op edition]'</tt> of this Solvable. */
      CapabilitySet providesNamespace( const std::string & namespace_r ) const;

      /** Return <tt>'value[ op edition]'</tt> for namespaced provides <tt>'namespace(value)[ op edition]'</tt>.
       * Similar to \ref providesNamespace, but the namespace is stripped from the
       * dependencies. This is convenient if the namespace denotes packages that
       * should be looked up. E.g. the \c weakremover namespace used in a products
       * release package denotes the packages that were dropped from the distribution.
       * \see \ref Product::droplist
       */
      CapabilitySet valuesOfNamespace( const std::string & namespace_r ) const;
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
      /** Return the supported locales. */
      LocaleSet getSupportedLocales() const;
      /** \overload Legacy return via arg \a locales_r */
      void getSupportedLocales( LocaleSet & locales_r ) const
      { locales_r = getSupportedLocales(); }
      //@}

    public:
      /** The solvables CpeId if available. */
      CpeId cpeId() const;

      /** Media number the solvable is located on (\c 0 if no media access required). */
      unsigned mediaNr() const;

      /** Installed (unpacked) size.
       * This is just a total number. Many objects provide even more detailed
       * disk usage data. You can use \ref DiskUsageCounter to find out
       * how objects data are distributed across partitions/directories.
       * \code
       *   // Load directory set into ducounter
       *   DiskUsageCounter ducounter( { "/", "/usr", "/var" } );
       *
       *   // see how noch space the packages use
       *   for ( const PoolItem & pi : pool )
       *   {
       *     cout << pi << ducounter.disk_usage( pi ) << endl;
       *     // I__s_(7)GeoIP-1.4.8-3.1.2.x86_64(@System) {
       *     // dir:[/] [ bs: 0 B ts: 0 B us: 0 B (+-: 1.0 KiB)]
       *     // dir:[/usr] [ bs: 0 B ts: 0 B us: 0 B (+-: 133.0 KiB)]
       *     // dir:[/var] [ bs: 0 B ts: 0 B us: 0 B (+-: 1.1 MiB)]
       *     // }
       *   }
       * \endcode
       * \see \ref DiskUsageCounter
       */
      ByteCount installSize() const;

      /** Download size. */
      ByteCount downloadSize() const;

      /** The distribution string. */
      std::string distribution() const;

      /** Short (singleline) text describing the solvable (opt. translated). */
      std::string summary( const Locale & lang_r = Locale() ) const;

      /** Long (multiline) text describing the solvable (opt. translated). */
      std::string description( const Locale & lang_r = Locale() ) const;

      /** UI hint text when selecting the solvable for install (opt. translated). */
      std::string insnotify( const Locale & lang_r = Locale() ) const;
      /** UI hint text when selecting the solvable for uninstall (opt. translated).*/
      std::string delnotify( const Locale & lang_r = Locale() ) const;

      /** License or agreement to accept before installing the solvable (opt. translated). */
      std::string licenseToConfirm( const Locale & lang_r = Locale() ) const;
      /** \c True except for well known exceptions (i.e show license but no need to accept it). */
      bool needToAcceptLicense() const;

    public:
      /** Helper that splits an identifier into kind and name or vice versa.
       * \note In case \c name_r is preceded by a well known kind spec, the
       * \c kind_r argument is ignored, and kind is derived from name.
       * \see \ref ident
       */
      class SplitIdent
      {
      public:
	SplitIdent() {}
	SplitIdent( IdString ident_r );
	SplitIdent( const char * ident_r );
	SplitIdent( const std::string & ident_r );
	SplitIdent( ResKind kind_r, IdString name_r );
	SplitIdent( ResKind kind_r, const C_Str & name_r );

	IdString ident() const { return _ident; }
	ResKind  kind()  const { return _kind; }
	IdString name()  const { return _name; }

      private:
	IdString  _ident;
	ResKind   _kind;
	IdString  _name;
      };

    public:
      /** \name Attribute lookup.
       * \see \ref LookupAttr and  \ref ArrayAttr providing a general, more
       * query like interface for attribute retrieval.
       */
      //@{
      /**
       * returns the string attribute value for \ref attr
       * or an empty string if it does not exists.
       */
      std::string lookupStrAttribute( const SolvAttr & attr ) const;
      /** \overload Trying to look up a translated string attribute.
       *
       * Returns the translation for \c lang_r.
       *
       * Passing an empty \ref Locale will return the string for the
       * current default locale (\see \ref ZConfig::TextLocale),
       * \b considering all fallback locales.
       *
       * Returns an empty string if no translation is available.
       */
      std::string lookupStrAttribute( const SolvAttr & attr, const Locale & lang_r ) const;

      /**
       * returns the numeric attribute value for \ref attr
       * or 0 if it does not exists.
       */
      unsigned long long lookupNumAttribute( const SolvAttr & attr ) const;

      /**
       * returns the boolean attribute value for \ref attr
       * or \c false if it does not exists.
       */
      bool lookupBoolAttribute( const SolvAttr & attr ) const;

      /**
       * returns the id attribute value for \ref attr
       * or \ref detail::noId if it does not exists.
       */
      detail::IdType lookupIdAttribute( const SolvAttr & attr ) const;

      /**
       * returns the CheckSum attribute value for \ref attr
       * or an empty CheckSum if ir does not exist.
       */
      CheckSum lookupCheckSumAttribute( const SolvAttr & attr ) const;

      /**
       * returns OnMediaLocation data: This is everything we need to
       * download e.g. an rpm (path, checksum, downloadsize, etc.).
       */
      OnMediaLocation lookupLocation() const;
      //@}

    public:
      /** Return next Solvable in \ref Pool (or \ref noSolvable). */
      Solvable nextInPool() const;
      /** Return next Solvable in \ref Repo (or \ref noSolvable). */
      Solvable nextInRepo() const;
      /** Expert backdoor. */
      detail::CSolvable * get() const;
      /** Expert backdoor. */
      IdType id() const { return _id; }

    private:
      IdType _id;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Solvable Stream output */
    std::ostream & operator<<( std::ostream & str, const Solvable & obj );

    /** \relates Solvable More verbose stream output including dependencies */
    std::ostream & dumpOn( std::ostream & str, const Solvable & obj );

    /** \relates Solvable XML output */
    std::ostream & dumpAsXmlOn( std::ostream & str, const Solvable & obj );

    /** \relates Solvable */
    inline bool operator==( const Solvable & lhs, const Solvable & rhs )
    { return lhs.get() == rhs.get(); }

    /** \relates Solvable */
    inline bool operator!=( const Solvable & lhs, const Solvable & rhs )
    { return lhs.get() != rhs.get(); }

    /** \relates Solvable */
    inline bool operator<( const Solvable & lhs, const Solvable & rhs )
    { return lhs.get() < rhs.get(); }

    /** \relates Solvable Test whether a \ref Solvable is of a certain Kind. */
    template<class TRes>
    inline bool isKind( const Solvable & solvable_r )
    { return solvable_r.isKind( ResTraits<TRes>::kind ); }

    /** \relates Solvable Test for same content. */
    inline bool identical( const Solvable & lhs, const Solvable & rhs )
    { return lhs.identical( rhs ); }

    /** \relates Solvable Test for same name version release and arch. */
    inline bool sameNVRA( const Solvable & lhs, const Solvable & rhs )
    { return lhs.sameNVRA( rhs ); }


    /** \relates Solvable Compare according to \a kind and \a name. */
    inline int compareByN( const Solvable & lhs, const Solvable & rhs )
    {
      int res = 0;
      if ( lhs != rhs )
      {
	if ( (res = lhs.kind().compare( rhs.kind() )) == 0 )
	  res = lhs.name().compare( rhs.name() );
      }
      return res;
    }

    /** \relates Solvable Compare according to \a kind, \a name and \a edition. */
    inline int compareByNVR( const Solvable & lhs, const Solvable & rhs )
    {
      int res = compareByN( lhs, rhs );
      if ( res == 0 )
	res = lhs.edition().compare( rhs.edition() );
      return res;
    }

    /** \relates Solvable Compare according to \a kind, \a name, \a edition and \a arch. */
    inline int compareByNVRA( const Solvable & lhs, const Solvable & rhs )
    {
      int res = compareByNVR( lhs, rhs );
      if ( res == 0 )
	res = lhs.arch().compare( rhs.arch() );
      return res;
    }

    ///////////////////////////////////////////////////////////////////
    namespace detail
    {
      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : SolvableIterator
      //
      /** */
      class SolvableIterator : public boost::iterator_adaptor<
          SolvableIterator                   // Derived
          , CSolvable*                       // Base
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
    } // namespace detail
    ///////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////

  class PoolItem;
  ///////////////////////////////////////////////////////////////////
  namespace sat
  {
    /** To Solvable transform functor.
     * \relates Solvable
     * \relates sat::SolvIterMixin
     */
    struct asSolvable
    {
      typedef Solvable result_type;

      Solvable operator()( const Solvable & solv_r ) const
      { return solv_r; }

      Solvable operator()( const PoolItem & pi_r ) const;

      Solvable operator()( const ResObject_constPtr & res_r ) const;
    };
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

ZYPP_DEFINE_ID_HASHABLE( ::zypp::sat::Solvable );

#endif // ZYPP_SAT_SOLVABLE_H
