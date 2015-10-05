/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/SolvableType.h
 */
#ifndef ZYPP_SAT_SOLVABLETYPE_H
#define ZYPP_SAT_SOLVABLETYPE_H

#include <iosfwd>

#include "zypp/sat/Solvable.h"
#include "zypp/Repository.h"
#include "zypp/OnMediaLocation.h"
#include "zypp/ByteCount.h"
#include "zypp/CheckSum.h"
#include "zypp/CpeId.h"
#include "zypp/Date.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace sat
  {
    ///////////////////////////////////////////////////////////////////
    /// \class SolvableType
    /// \brief Base class for creating \ref Solvable based types.
    /// \ingroup g_CRTP
    ///
    /// Derive from this and offer explicit conversion to \ref Solvable
    /// to make the \ref Solvable properties directly accessible.
    ///
    /// Different SolvableTypes are comparable based on the underlying
    /// \ref Solvable.
    ///
    /// \see \ref Solvable
    ///
    /// \code
    ///   class MySolvable : public SolvableType<MySolvable>
    ///   {
    ///     ......
    ///   public:
    ///     explicit operator sat::Solvable() const;
    ///
    ///   };
    /// \endcode
    ///////////////////////////////////////////////////////////////////
    template <class Derived>
    struct SolvableType
    {
      /** Return the corresponding \ref sat::Solvable. */
      Solvable satSolvable() const { return Solvable(static_cast<const Derived&>(*this)); }

      explicit operator bool() const				{ return bool(satSolvable()); }

      IdString		ident() const				{ return satSolvable().ident(); }

      ResKind 		kind() const				{ return satSolvable().kind(); }
      bool		isKind( const ResKind & kind_r ) const	{ return satSolvable().isKind( kind_r ); }
      template<class TRes>
      bool		isKind() const				{ return satSolvable().isKind<TRes>(); }
      template<class TIterator>
      bool isKind( TIterator begin, TIterator end ) const	{ return satSolvable().isKind( begin, end ); }

      std::string	name() const				{ return satSolvable().name(); }
      Edition		edition() const				{ return satSolvable().edition(); }
      Arch		arch() const				{ return satSolvable().arch(); }
      IdString		vendor() const				{ return satSolvable().vendor(); }

      Repository	repository() const			{ return satSolvable().repository(); }
      RepoInfo		repoInfo() const			{ return satSolvable().repoInfo(); }

      bool		isSystem() const			{ return satSolvable().isSystem(); }
      bool		onSystemByUser() const			{ return satSolvable().onSystemByUser(); }
      bool		multiversionInstall() const		{ return satSolvable().multiversionInstall(); }

      Date		buildtime() const			{ return satSolvable().buildtime(); }
      Date		installtime() const			{ return satSolvable().installtime(); }

      std::string	asString() const			{ return satSolvable().asString(); }
      std::string	asUserString() const			{ return satSolvable().asUserString(); }

      bool		identical( const Solvable & rhs ) const	{ return satSolvable().identical( rhs ); }
      template <class RDerived>
      bool		identical( const SolvableType<RDerived> & rhs ) const	{ return satSolvable().identical( rhs.satSolvable() ); }

      bool		sameNVRA( const Solvable &rhs ) const	{ return satSolvable().sameNVRA( rhs ); }
      template <class RDerived>
      bool		sameNVRA( const SolvableType<RDerived> & rhs ) const	{ return satSolvable().sameNVRA( rhs.satSolvable() ); }

      Capabilities	provides() const			{ return satSolvable().provides(); }
      Capabilities	requires() const			{ return satSolvable().requires(); }
      Capabilities	conflicts() const			{ return satSolvable().conflicts(); }
      Capabilities	obsoletes() const			{ return satSolvable().obsoletes(); }
      Capabilities	recommends() const			{ return satSolvable().recommends(); }
      Capabilities	suggests() const			{ return satSolvable().suggests(); }
      Capabilities	enhances() const			{ return satSolvable().enhances(); }
      Capabilities	supplements() const			{ return satSolvable().supplements(); }
      Capabilities	prerequires() const			{ return satSolvable().prerequires(); }
      Capabilities 	dep( Dep which_r ) const		{ return satSolvable().dep(which_r); }
      Capabilities 	operator[]( Dep which_r ) const		{ return satSolvable()[which_r]; }

      CapabilitySet	providesNamespace( const std::string & namespace_r ) const	{ return satSolvable().providesNamespace( namespace_r ); }
      CapabilitySet	valuesOfNamespace( const std::string & namespace_r ) const	{ return satSolvable().valuesOfNamespace( namespace_r ); }

      bool		supportsLocales() const			{ return satSolvable().supportsLocales(); }
      bool		supportsLocale( const Locale & locale_r ) const	{ return satSolvable().supportsLocale( locale_r ); }
      bool		supportsLocale( const LocaleSet & locales_r ) const	{ return satSolvable().supportsLocale( locales_r ); }
      bool		supportsRequestedLocales() const	{ return satSolvable().supportsRequestedLocales(); }
      LocaleSet		getSupportedLocales() const		{ return satSolvable().getSupportedLocales(); }

      CpeId		cpeId() const				{ return satSolvable().cpeId(); }
      unsigned		mediaNr() const				{ return satSolvable().mediaNr(); }
      ByteCount		installSize() const			{ return satSolvable().installSize(); }
      ByteCount		downloadSize() const			{ return satSolvable().downloadSize(); }
      std::string	distribution() const			{ return satSolvable().distribution(); }

      std::string	summary( const Locale & lang_r = Locale() ) const	{ return satSolvable().summary( lang_r ); }
      std::string	description( const Locale & lang_r = Locale() ) const	{ return satSolvable().description( lang_r ); }
      std::string	insnotify( const Locale & lang_r = Locale() ) const	{ return satSolvable().insnotify( lang_r ); }
      std::string	delnotify( const Locale & lang_r = Locale() ) const	{ return satSolvable().delnotify( lang_r ); }
      std::string	licenseToConfirm( const Locale & lang_r = Locale() ) const	{ return satSolvable().licenseToConfirm( lang_r ); }
      bool		needToAcceptLicense() const		{ return satSolvable().needToAcceptLicense(); }

    public:
      std::string	lookupStrAttribute( const SolvAttr & attr ) const	{ return satSolvable().lookupStrAttribute( attr ); }
      std::string	lookupStrAttribute( const SolvAttr & attr, const Locale & lang_r ) const	{ return satSolvable().lookupStrAttribute( attr, lang_r ); }
      bool		lookupBoolAttribute( const SolvAttr & attr ) const	{ return satSolvable().lookupBoolAttribute( attr ); }
      detail::IdType	lookupIdAttribute( const SolvAttr & attr ) const	{ return satSolvable().lookupIdAttribute( attr ); }
      unsigned long long lookupNumAttribute( const SolvAttr & attr ) const	{ return satSolvable().lookupNumAttribute( attr ); }
      CheckSum		lookupCheckSumAttribute( const SolvAttr & attr ) const	{ return satSolvable().lookupCheckSumAttribute( attr ); }
      OnMediaLocation	lookupLocation() const			{ return satSolvable().lookupLocation(); }
      Solvable::IdType	id() const 					{ return satSolvable().id(); }

    protected:
      SolvableType() {}
      SolvableType( const SolvableType & ) {}
      void operator=( const SolvableType & ) {}
#ifndef SWIG
      SolvableType( SolvableType && ) {}
      void operator=( SolvableType && ) {}
#endif
      ~SolvableType() {}
    };

    /** \relates SolvableType Stream output */
    template <class Derived>
    inline std::ostream & operator<<( std::ostream & str, const SolvableType<Derived> & obj )
    { return str << obj.satSolvable(); }

    /** \relates SolvableType More verbose stream output including dependencies */
    template <class Derived>
    inline std::ostream & dumpOn( std::ostream & str, const SolvableType<Derived> & obj )
    { return dumpOn( str, obj.satSolvable() ); }

    /** \relates SolvableType Equal*/
    template <class LDerived, class RDerived>
    inline bool operator==( const SolvableType<LDerived> & lhs, const SolvableType<RDerived> & rhs )
    { return lhs.satSolvable() == rhs.satSolvable(); }
    /** \overload */
    template <class Derived>
    inline bool operator==( const SolvableType<Derived> & lhs, const Solvable & rhs )
    { return lhs.satSolvable() == rhs; }
    /** \overload */
    template <class Derived>
    inline bool operator==( const Solvable & lhs, const SolvableType<Derived> & rhs )
    { return lhs == rhs.satSolvable(); }

    /** \relates SolvableType NotEqual */
    template <class LDerived, class RDerived>
    inline bool operator!=( const SolvableType<LDerived> & lhs, const SolvableType<RDerived> & rhs )
    { return lhs.satSolvable() != rhs.satSolvable(); }
    /** \overload */
    template <class Derived>
    inline bool operator!=( const SolvableType<Derived> & lhs, const Solvable & rhs )
    { return lhs.satSolvable() != rhs; }
    /** \overload */
    template <class Derived>
    inline bool operator!=( const Solvable & lhs, const SolvableType<Derived> & rhs )
    { return lhs != rhs.satSolvable(); }

    /** \relates SolvableType Less*/
    template <class LDerived, class RDerived>
    inline bool operator<( const SolvableType<LDerived> & lhs, const SolvableType<RDerived> & rhs )
    { return lhs.satSolvable() < rhs.satSolvable(); }
    /** \overload */
    template <class Derived>
    inline bool operator<( const SolvableType<Derived> & lhs, const Solvable & rhs )
    { return lhs.satSolvable() < rhs; }
    /** \overload */
    template <class Derived>
    inline bool operator<( const Solvable & lhs, const SolvableType<Derived> & rhs )
    { return lhs < rhs.satSolvable(); }

    /** \relates SolvableType Test whether the \ref Solvable is of a certain \ref ResKind. */
    template<class TRes, class Derived>
    inline bool isKind( const SolvableType<Derived> & solvable_r )
    { return isKind<TRes>( solvable_r.satSolvable() ); }

    /** \relates SolvableType Test for same content. */
    template <class LDerived, class RDerived>
    inline bool identical( const SolvableType<LDerived> & lhs, const SolvableType<RDerived> & rhs )
    { return identical( lhs.satSolvable(), rhs.satSolvable() ); }
    /** \overload */
    template <class Derived>
    inline bool identical( const SolvableType<Derived> & lhs, const Solvable & rhs )
    { return identical( lhs.satSolvable(), rhs ); }
    /** \overload */
    template <class Derived>
    inline bool identical( const Solvable & lhs, const SolvableType<Derived> & rhs )
    { return identical( lhs, rhs.satSolvable() ); }

    /** \relates SolvableType Test for same name version release and arch. */
    template <class LDerived, class RDerived>
    inline bool sameNVRA( const SolvableType<LDerived> & lhs, const SolvableType<RDerived> & rhs )
    { return sameNVRA( lhs.satSolvable(), rhs.satSolvable() ); }
    /** \overload */
    template <class Derived>
    inline bool sameNVRA( const SolvableType<Derived> & lhs, const Solvable & rhs )
    { return sameNVRA( lhs.satSolvable(), rhs ); }
    /** \overload */
    template <class Derived>
    inline bool sameNVRA( const Solvable & lhs, const SolvableType<Derived> & rhs )
    { return sameNVRA( lhs, rhs.satSolvable() ); }


    /** \relates SolvableType Compare according to \a kind and \a name. */
    template <class LDerived, class RDerived>
    inline int compareByN( const SolvableType<LDerived> & lhs, const SolvableType<RDerived> & rhs )
    { return compareByN( lhs.satSolvable(), rhs.satSolvable() ); }
    /** \overload */
    template <class Derived>
    inline bool compareByN( const SolvableType<Derived> & lhs, const Solvable & rhs )
    { return compareByN( lhs.satSolvable(), rhs ); }
    /** \overload */
    template <class Derived>
    inline bool compareByN( const Solvable & lhs, const SolvableType<Derived> & rhs )
    { return compareByN( lhs, rhs.satSolvable() ); }


    /** \relates SolvableType Compare according to \a kind, \a name and \a edition. */
    template <class LDerived, class RDerived>
    inline int compareByNVR( const SolvableType<LDerived> & lhs, const SolvableType<RDerived> & rhs )
    { return compareByNVR( lhs.satSolvable(), rhs.satSolvable() ); }
    /** \overload */
    template <class Derived>
    inline bool compareByNVR( const SolvableType<Derived> & lhs, const Solvable & rhs )
    { return compareByNVR( lhs.satSolvable(), rhs ); }
    /** \overload */
    template <class Derived>
    inline bool compareByNVR( const Solvable & lhs, const SolvableType<Derived> & rhs )
    { return compareByNVR( lhs, rhs.satSolvable() ); }

    /** \relates SolvableType Compare according to \a kind, \a name, \a edition and \a arch. */
    template <class LDerived, class RDerived>
    inline int compareByNVRA( const SolvableType<LDerived> & lhs, const SolvableType<RDerived> & rhs )
    { return compareByNVRA( lhs.satSolvable(), rhs.satSolvable() ); }
    /** \overload */
    template <class Derived>
    inline bool compareByNVRA( const SolvableType<Derived> & lhs, const Solvable & rhs )
    { return compareByNVRA( lhs.satSolvable(), rhs ); }
    /** \overload */
    template <class Derived>
    inline bool compareByNVRA( const Solvable & lhs, const SolvableType<Derived> & rhs )
    { return compareByNVRA( lhs, rhs.satSolvable() ); }

  } // namespace sat
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_SOLVABLETYPE_H
