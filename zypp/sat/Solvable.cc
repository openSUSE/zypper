/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/Solvable.cc
 *
*/
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Exception.h"
#include "zypp/base/Functional.h"
#include "zypp/base/Collector.h"

#include "zypp/sat/detail/PoolImpl.h"
#include "zypp/sat/Solvable.h"
#include "zypp/sat/Pool.h"
#include "zypp/sat/LookupAttr.h"

#include "zypp/Repository.h"
#include "zypp/OnMediaLocation.h"
#include "zypp/ZConfig.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    namespace
    {
      void _doSplit( IdString & _ident, ResKind & _kind, IdString & _name )
      {
        if ( ! _ident )
          return;

	ResKind explicitKind = Solvable::SplitIdent::explicitKind( _ident.c_str() );
	// NOTE: kind package and srcpackage do not have namespaced ident!
	if ( ! explicitKind  )
	{
          _name = _ident;
	  // No kind defaults to package
	  if ( !_kind )
	    _kind = ResKind::package;
	  if ( ! ( _kind == ResKind::package || _kind == ResKind::srcpackage ) )
	    _ident = IdString( str::form( "%s:%s", _kind.c_str(), _ident.c_str() ) );
	}
	else
	{
	  // strip kind spec from name
	  _name = IdString( ::strchr( _ident.c_str(), ':' )+1 );
	  _kind = explicitKind;
	  if ( _kind == ResKind::package || _kind == ResKind::srcpackage )
	    _ident = _name;
	}
	return;
      }
    }

    Solvable::SplitIdent::SplitIdent( IdString ident_r )
    : _ident( ident_r )
    { _doSplit( _ident, _kind, _name ); }

    Solvable::SplitIdent::SplitIdent( const char * ident_r )
    : _ident( ident_r )
    { _doSplit( _ident, _kind, _name ); }

    Solvable::SplitIdent::SplitIdent( const std::string & ident_r )
    : _ident( ident_r )
    { _doSplit( _ident, _kind, _name ); }

    Solvable::SplitIdent::SplitIdent( ResKind kind_r, IdString name_r )
    : _ident( name_r )
    , _kind( kind_r )
    { _doSplit( _ident, _kind, _name ); }

    Solvable::SplitIdent::SplitIdent( ResKind kind_r, const C_Str & name_r )
    : _ident( name_r )
    , _kind( kind_r )
    { _doSplit( _ident, _kind, _name ); }

    ResKind Solvable::SplitIdent::explicitKind( const char * ident_r )
    {
      if ( ! ident_r )
	return ResKind();

      const char * sep = ::strchr( ident_r, ':' );
      if ( ! sep )
	return ResKind();

      ResKind ret;
      if ( sep-ident_r >= 4 )
      {
	switch ( ident_r[3] )
	{
	  #define OUTS(K,S) if ( !::strncmp( ident_r, ResKind::K.c_str(), S ) && ident_r[S] == ':' ) ret = ResKind::K
	  //             ----v
	  case 'c': OUTS( patch, 5 );       break;
	  case 'd': OUTS( product, 7 );     break;
	  case 'k': OUTS( package, 7 );     break;
	  case 'p': OUTS( srcpackage, 10 ); break;
	  case 't': OUTS( pattern, 7 );     break;
	  #undef OUTS
	}
      }
      return ret;
    }

    /////////////////////////////////////////////////////////////////

    const Solvable Solvable::noSolvable;

    /////////////////////////////////////////////////////////////////

    ::_Solvable * Solvable::get() const
    { return myPool().getSolvable( _id ); }

#define NO_SOLVABLE_RETURN( VAL ) \
    ::_Solvable * _solvable( get() ); \
    if ( ! _solvable ) return VAL

    Solvable Solvable::nextInPool() const
    { return Solvable( myPool().getNextId( _id ) ); }

    Solvable Solvable::nextInRepo() const
    {
      NO_SOLVABLE_RETURN( noSolvable );
      for ( detail::SolvableIdType next = _id+1; next < unsigned(_solvable->repo->end); ++next )
      {
        ::_Solvable * nextS( myPool().getSolvable( next ) );
        if ( nextS && nextS->repo == _solvable->repo )
        {
          return Solvable( next );
        }
      }
      return noSolvable;
    }

    Repository Solvable::repository() const
    {
      NO_SOLVABLE_RETURN( Repository::noRepository );
      return Repository( _solvable->repo );
    }

    bool Solvable::isSystem() const
    {
      NO_SOLVABLE_RETURN( _id == detail::systemSolvableId );
      return myPool().isSystemRepo( _solvable->repo );
    }

    bool Solvable::onSystemByUser() const
    {
      return isSystem() && myPool().isOnSystemByUser( ident() );
    }

    IdString Solvable::ident() const
    {
      NO_SOLVABLE_RETURN( IdString() );
      return IdString( _solvable->name );
    }

    std::string Solvable::lookupStrAttribute( const SolvAttr & attr ) const
    {
      NO_SOLVABLE_RETURN( std::string() );
      const char * s = ::solvable_lookup_str( _solvable, attr.id() );
      return s ? s : std::string();
    }

    std::string Solvable::lookupStrAttribute( const SolvAttr & attr, const Locale & lang_r ) const
    {
      NO_SOLVABLE_RETURN( std::string() );
      const char * s = 0;
      if ( lang_r == Locale::noCode )
      {
        s = ::solvable_lookup_str_poollang( _solvable, attr.id() );
      }
      else
      {
	for ( Locale l( lang_r ); l != Locale::noCode; l = l.fallback() )
	  if ( (s = ::solvable_lookup_str_lang( _solvable, attr.id(), l.code().c_str(), 0 )) )
	    return s;
	  // here: no matching locale, so use default
	  s = ::solvable_lookup_str_lang( _solvable, attr.id(), 0, 0 );
      }
      return s ? s : std::string();
   }

    unsigned long long Solvable::lookupNumAttribute( const SolvAttr & attr ) const
    {
      NO_SOLVABLE_RETURN( 0 );
      return ::solvable_lookup_num( _solvable, attr.id(), 0 );
    }

    bool Solvable::lookupBoolAttribute( const SolvAttr & attr ) const
    {
      NO_SOLVABLE_RETURN( false );
      return ::solvable_lookup_bool( _solvable, attr.id() );
    }

    detail::IdType Solvable::lookupIdAttribute( const SolvAttr & attr ) const
    {
      NO_SOLVABLE_RETURN( detail::noId );
      return ::solvable_lookup_id( _solvable, attr.id() );
    }

    CheckSum Solvable::lookupCheckSumAttribute( const SolvAttr & attr ) const
    {
      NO_SOLVABLE_RETURN( CheckSum() );
      detail::IdType chksumtype = 0;
      const char * s = ::solvable_lookup_checksum( _solvable, attr.id(), &chksumtype );
      if ( ! s )
        return CheckSum();
      switch ( chksumtype )
      {
        case REPOKEY_TYPE_MD5:    return CheckSum::md5( s );
        case REPOKEY_TYPE_SHA1:   return CheckSum::sha1( s );
        case REPOKEY_TYPE_SHA256: return CheckSum::sha256( s );
      }
      return CheckSum( std::string(), s ); // try to autodetect
    }

    ///////////////////////////////////////////////////////////////////
    namespace
    {
      inline Pathname lookupDatadirIn( Repository repor_r )
      {
        static const sat::SolvAttr susetagsDatadir( "susetags:datadir" );
        Pathname ret;
        // First look for repo attribute "susetags:datadir". If not found,
        // look into the solvables as Code11 libsolv placed it there.
        sat::LookupRepoAttr datadir( susetagsDatadir, repor_r );
        if ( ! datadir.empty() )
          ret = datadir.begin().asString();
        else
        {
          sat::LookupAttr datadir( susetagsDatadir, repor_r );
          if ( ! datadir.empty() )
            ret = datadir.begin().asString();
        }
        return ret;
      }
    }
    ///////////////////////////////////////////////////////////////////

    OnMediaLocation Solvable::lookupLocation() const
    {
      NO_SOLVABLE_RETURN( OnMediaLocation() );
      // medianumber and path
      unsigned medianr;
      const char * file = ::solvable_get_location( _solvable, &medianr );
      if ( ! file )
        return OnMediaLocation();

      OnMediaLocation ret;

      Pathname path;
      switch ( repository().info().type().toEnum() )
      {
        case repo::RepoType::NONE_e:
        {
          path = lookupDatadirIn( repository() );
          if ( ! path.empty() )
            repository().info().setProbedType( repo::RepoType::YAST2_e );
        }
        break;

        case repo::RepoType::YAST2_e:
        {
          path = lookupDatadirIn( repository() );
          if ( path.empty() )
            path = "suse";
        }
        break;

        default:
          break;
      }
      ret.setLocation    ( path/file, medianr );
      ret.setDownloadSize( ByteCount( lookupNumAttribute( SolvAttr::downloadsize ) ) );
      ret.setChecksum    ( lookupCheckSumAttribute( SolvAttr::checksum ) );
      // Not needed/available for solvables?
      //ret.setOpenSize    ( ByteCount( lookupNumAttribute( SolvAttr::opensize ) ) );
      //ret.setOpenChecksum( lookupCheckSumAttribute( SolvAttr::openchecksum ) );
      return ret;
    }

    ResKind Solvable::kind() const
    {
      NO_SOLVABLE_RETURN( ResKind() );
      // detect srcpackages by 'arch'
      switch ( _solvable->arch )
      {
        case ARCH_SRC:
        case ARCH_NOSRC:
          return ResKind::srcpackage;
          break;
      }

      const char * ident = IdString( _solvable->name ).c_str();
      const char * sep = ::strchr( ident, ':' );

      // no ':' in package names (hopefully)
      if ( ! sep )
        return ResKind::package;

      // quick check for well known kinds
      if ( sep-ident >= 4 )
      {
        switch ( ident[3] )
        {
#define OUTS(K,S) if ( !::strncmp( ident, ResKind::K.c_str(), S ) ) return ResKind::K
          //             ----v
          case 'c': OUTS( patch, 5 );       break;
          case 'd': OUTS( product, 7 );     break;
          case 'k': OUTS( package, 7 );     break;
          case 'p': OUTS( srcpackage, 10 ); break;
          case 't': OUTS( pattern, 7 );     break;
#undef OUTS
        }
      }

      // an unknown kind
      return ResKind( std::string( ident, sep-ident ) );
    }

    bool Solvable::isKind( const ResKind & kind_r ) const
    {
      NO_SOLVABLE_RETURN( false );

      // detect srcpackages by 'arch'
      switch ( _solvable->arch )
      {
        case ARCH_SRC:
        case ARCH_NOSRC:
          return( kind_r == ResKind::srcpackage );
          break;
      }

      // no ':' in package names (hopefully)
      const char * ident = IdString( _solvable->name ).c_str();
      if ( kind_r == ResKind::package )
      {
        return( ::strchr( ident, ':' ) == 0 );
      }

      // look for a 'kind:' prefix
      const char * kind = kind_r.c_str();
      unsigned     ksize = ::strlen( kind );
      return( ::strncmp( ident, kind, ksize ) == 0
              && ident[ksize] == ':' );
    }

    std::string Solvable::name() const
    {
      NO_SOLVABLE_RETURN( std::string() );
      const char * ident = IdString( _solvable->name ).c_str();
      const char * sep = ::strchr( ident, ':' );
      return( sep ? sep+1 : ident );
    }

    Edition Solvable::edition() const
    {
      NO_SOLVABLE_RETURN( Edition() );
      return Edition( _solvable->evr );
    }

    Arch Solvable::arch() const
    {
      NO_SOLVABLE_RETURN( Arch_noarch ); //ArchId() );
      switch ( _solvable->arch )
      {
        case ARCH_SRC:
        case ARCH_NOSRC:
          return Arch_noarch; //ArchId( ARCH_NOARCH );
          break;
      }
      return Arch( IdString(_solvable->arch).asString() );
      //return ArchId( _solvable->arch );
    }

    bool Solvable::multiversionInstall() const
    {
      return myPool().isMultiversion( ident() );
    }

    IdString Solvable::vendor() const
    {
      NO_SOLVABLE_RETURN( IdString() );
      return IdString( _solvable->vendor );
    }

    Capabilities Solvable::operator[]( Dep which_r ) const
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

    inline Capabilities _getCapabilities( detail::IdType * idarraydata_r, ::Offset offs_r )
    {
      return offs_r ? Capabilities( idarraydata_r + offs_r ) : Capabilities();
    }
    Capabilities Solvable::provides() const
    {
      NO_SOLVABLE_RETURN( Capabilities() );
      return _getCapabilities( _solvable->repo->idarraydata, _solvable->provides );
    }
    Capabilities Solvable::requires() const
    {
      NO_SOLVABLE_RETURN( Capabilities() );
      return _getCapabilities( _solvable->repo->idarraydata, _solvable->requires );
    }
    Capabilities Solvable::conflicts() const
    {
      NO_SOLVABLE_RETURN( Capabilities() );
      return _getCapabilities( _solvable->repo->idarraydata, _solvable->conflicts );
    }
    Capabilities Solvable::obsoletes() const
    {
      NO_SOLVABLE_RETURN( Capabilities() );
      return _getCapabilities( _solvable->repo->idarraydata, _solvable->obsoletes );
    }
    Capabilities Solvable::recommends() const
    {
      NO_SOLVABLE_RETURN( Capabilities() );
      return _getCapabilities( _solvable->repo->idarraydata, _solvable->recommends );
    }
    Capabilities Solvable::suggests() const
    {
      NO_SOLVABLE_RETURN( Capabilities() );
      return _getCapabilities( _solvable->repo->idarraydata, _solvable->suggests );
    }
    Capabilities Solvable::enhances() const
    {
      NO_SOLVABLE_RETURN( Capabilities() );
      return _getCapabilities( _solvable->repo->idarraydata, _solvable->enhances );
    }
    Capabilities Solvable::supplements() const
    {
      NO_SOLVABLE_RETURN( Capabilities() );
      return _getCapabilities( _solvable->repo->idarraydata, _solvable->supplements );
    }
    Capabilities Solvable::prerequires() const
    {
      NO_SOLVABLE_RETURN( Capabilities() );
      // prerequires are a subset of requires
       ::Offset offs = _solvable->requires;
       return offs ? Capabilities( _solvable->repo->idarraydata + offs, detail::solvablePrereqMarker )
                   : Capabilities();
    }

    CapabilitySet Solvable::providesNamespace( const std::string & namespace_r ) const
    {
      NO_SOLVABLE_RETURN( CapabilitySet() );
      CapabilitySet ret;
      Capabilities caps( provides() );
      for_( it, caps.begin(), caps.end() )
      {
        CapDetail caprep( it->detail() );
        if ( str::hasPrefix( caprep.name().c_str(), namespace_r ) && *(caprep.name().c_str()+namespace_r.size()) == '(' )
          ret.insert( *it );
      }
      return ret;
   }

    CapabilitySet Solvable::valuesOfNamespace( const std::string & namespace_r ) const
    {
      NO_SOLVABLE_RETURN( CapabilitySet() );
      CapabilitySet ret;
      Capabilities caps( provides() );
      for_( it, caps.begin(), caps.end() )
      {
        CapDetail caprep( it->detail() );
        if ( str::hasPrefix( caprep.name().c_str(), namespace_r ) && *(caprep.name().c_str()+namespace_r.size()) == '(' )
        {
          std::string value( caprep.name().c_str()+namespace_r.size()+1 );
          value[value.size()-1] = '\0'; // erase the trailing ')'
          ret.insert( Capability( value, caprep.op(), caprep.ed() ) );
        }
      }
      return ret;
    }


    std::string Solvable::asString() const
    {
      NO_SOLVABLE_RETURN( (_id == detail::systemSolvableId ? "systemSolvable" : "noSolvable") );
      return str::form( "%s-%s.%s",
                        IdString( _solvable->name ).c_str(),
                        IdString( _solvable->evr ).c_str(),
                        IdString( _solvable->arch ).c_str() );
    }

    bool Solvable::identical( Solvable rhs ) const
    {
      NO_SOLVABLE_RETURN( ! rhs.get() );
      ::_Solvable * rhssolvable( rhs.get() );
      return rhssolvable && ( _solvable == rhssolvable || ::solvable_identical( _solvable, rhssolvable ) );
    }

    ///////////////////////////////////////////////////////////////////
    namespace
    { /////////////////////////////////////////////////////////////////
      /** Expand \ref Capability and call \c fnc_r for each namescpace:language
       * dependency. Return #invocations of fnc_r, negative if fnc_r returned
       * false to indicate abort.
       */
      int invokeOnEachSupportedLocale( Capability cap_r, function<bool (const Locale &)> fnc_r )
      {
        CapDetail detail( cap_r );
        if ( detail.kind() == CapDetail::EXPRESSION )
        {
          switch ( detail.capRel() )
          {
            case CapDetail::CAP_AND:
            case CapDetail::CAP_OR:
                // expand
              {
                int res = invokeOnEachSupportedLocale( detail.lhs(), fnc_r );
                if ( res < 0 )
                  return res; // negative on abort.
                int res2 = invokeOnEachSupportedLocale( detail.rhs(), fnc_r );
                if ( res2 < 0 )
                  return -res + res2; // negative on abort.
                return res + res2;
              }
              break;

            case CapDetail::CAP_NAMESPACE:
              if ( detail.lhs().id() == NAMESPACE_LANGUAGE )
              {
                return ( !fnc_r || fnc_r( Locale( IdString(detail.rhs().id()) ) ) ) ? 1 : -1; // negative on abort.
              }
              break;

            case CapDetail::REL_NONE:
            case CapDetail::CAP_WITH:
            case CapDetail::CAP_ARCH:
              break; // unwanted
          }
        }
        return 0;
      }

       /** Expand \ref Capability and call \c fnc_r for each namescpace:language
       * dependency. Return #invocations of fnc_r, negative if fnc_r returned
       * false to indicate abort.
       */
      inline int invokeOnEachSupportedLocale( Capabilities cap_r, function<bool (const Locale &)> fnc_r )
      {
        int cnt = 0;
        for_( cit, cap_r.begin(), cap_r.end() )
        {
          int res = invokeOnEachSupportedLocale( *cit, fnc_r );
          if ( res < 0 )
            return -cnt + res; // negative on abort.
          cnt += res;
        }
        return cnt;
      }
      //@}

      // Functor returning false if a Locale is in the set.
      struct NoMatchIn
      {
        NoMatchIn( const LocaleSet & locales_r ) : _locales( locales_r ) {}

        bool operator()( const Locale & locale_r ) const
        {
          return _locales.find( locale_r ) == _locales.end();
        }

        const LocaleSet & _locales;
      };

    } /////////////////////////////////////////////////////////////////

    bool Solvable::supportsLocales() const
    {
      // false_c stops on 1st Locale.
      return invokeOnEachSupportedLocale( supplements(), functor::false_c() ) < 0;
    }

    bool Solvable::supportsLocale( const Locale & locale_r ) const
    {
      // not_equal_to stops on == Locale.
      return invokeOnEachSupportedLocale( supplements(), bind( std::not_equal_to<Locale>(), locale_r, _1 ) ) < 0;
    }

    bool Solvable::supportsLocale( const LocaleSet & locales_r ) const
    {
      if ( locales_r.empty() )
        return false;
      // NoMatchIn stops if Locale is included.
      return invokeOnEachSupportedLocale( supplements(), NoMatchIn(locales_r) ) < 0;
    }

    bool Solvable::supportsRequestedLocales() const
    { return supportsLocale( myPool().getRequestedLocales() ); }

    void Solvable::getSupportedLocales( LocaleSet & locales_r ) const
    {
      invokeOnEachSupportedLocale( supplements(),
                                   functor::Collector( std::inserter( locales_r, locales_r.begin() ) ) );
    }

    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const Solvable & obj )
    {
      if ( ! obj )
        return str << (obj.isSystem() ? "systemSolvable" : "noSolvable" );

      return str << "(" << obj.id() << ")"
          << ( obj.isKind( ResKind::srcpackage ) ? "srcpackage:" : "" ) << obj.ident()
          << '-' << obj.edition() << '.' << obj.arch() << "("
          << obj.repository().alias() << ")";
    }

    /******************************************************************
    **
    **	FUNCTION NAME : dumpOn
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & dumpOn( std::ostream & str, const Solvable & obj )
    {
      str << obj;
      if ( obj )
      {
#define OUTS(X) if ( ! obj[Dep::X].empty() ) str << endl << " " #X " " << obj[Dep::X]
        OUTS(PROVIDES);
        OUTS(PREREQUIRES);
        OUTS(REQUIRES);
        OUTS(CONFLICTS);
        OUTS(OBSOLETES);
        OUTS(RECOMMENDS);
        OUTS(SUGGESTS);
        OUTS(ENHANCES);
        OUTS(SUPPLEMENTS);
#undef OUTS
      }
      return str;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
