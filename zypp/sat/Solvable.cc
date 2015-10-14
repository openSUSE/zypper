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
#include "zypp/base/Xml.h"

#include "zypp/sat/detail/PoolImpl.h"
#include "zypp/sat/Solvable.h"
#include "zypp/sat/Pool.h"
#include "zypp/sat/LookupAttr.h"

#include "zypp/Repository.h"
#include "zypp/OnMediaLocation.h"
#include "zypp/ZConfig.h"

#include "zypp/ui/Selectable.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace sat
  {
    ///////////////////////////////////////////////////////////////////
    namespace
    {
      void _doSplit( IdString & _ident, ResKind & _kind, IdString & _name )
      {
        if ( ! _ident )
          return;

	ResKind explicitKind = ResKind::explicitBuiltin( _ident.c_str() );
	// NOTE: kind package and srcpackage do not have namespaced ident!
	if ( ! explicitKind  )
	{
          _name = _ident;
	  // No kind defaults to package
	  if ( !_kind )
	    _kind = ResKind::package;
	  else if ( ! ( _kind == ResKind::package || _kind == ResKind::srcpackage ) )
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
    } // namespace
    ///////////////////////////////////////////////////////////////////

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

    /////////////////////////////////////////////////////////////////
    //	class Solvable
    /////////////////////////////////////////////////////////////////

    const Solvable Solvable::noSolvable;

    /////////////////////////////////////////////////////////////////

    detail::CSolvable * Solvable::get() const
    { return myPool().getSolvable( _id ); }

#define NO_SOLVABLE_RETURN( VAL ) \
    detail::CSolvable * _solvable( get() ); \
    if ( ! _solvable ) return VAL

    Solvable Solvable::nextInPool() const
    { return Solvable( myPool().getNextId( _id ) ); }

    Solvable Solvable::nextInRepo() const
    {
      NO_SOLVABLE_RETURN( noSolvable );
      for ( detail::SolvableIdType next = _id+1; next < unsigned(_solvable->repo->end); ++next )
      {
        detail::CSolvable * nextS( myPool().getSolvable( next ) );
        if ( nextS && nextS->repo == _solvable->repo )
        {
          return Solvable( next );
        }
      }
      return noSolvable;
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
      if ( !lang_r )
      {
        s = ::solvable_lookup_str_poollang( _solvable, attr.id() );
      }
      else
      {
	for ( Locale l( lang_r ); l; l = l.fallback() )
	  if ( (s = ::solvable_lookup_str_lang( _solvable, attr.id(), l.c_str(), 0 )) )
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
        case REPOKEY_TYPE_SHA224: return CheckSum::sha224( s );
        case REPOKEY_TYPE_SHA256: return CheckSum::sha256( s );
        case REPOKEY_TYPE_SHA384: return CheckSum::sha384( s );
        case REPOKEY_TYPE_SHA512: return CheckSum::sha512( s );
      }
      return CheckSum( std::string(), s ); // try to autodetect
    }

    ///////////////////////////////////////////////////////////////////
    namespace
    {
      inline Pathname lookupDatadirIn( Repository repor_r )
      {
        static const SolvAttr susetagsDatadir( "susetags:datadir" );
        Pathname ret;
        // First look for repo attribute "susetags:datadir". If not found,
        // look into the solvables as Code11 libsolv placed it there.
        LookupRepoAttr datadir( susetagsDatadir, repor_r );
        if ( ! datadir.empty() )
          ret = datadir.begin().asString();
        else
        {
          LookupAttr datadir( susetagsDatadir, repor_r );
          if ( ! datadir.empty() )
            ret = datadir.begin().asString();
        }
        return ret;
      }
    } // namespace
    ///////////////////////////////////////////////////////////////////

    OnMediaLocation Solvable::lookupLocation() const
    {
      NO_SOLVABLE_RETURN( OnMediaLocation() );
      // medianumber and path
      unsigned medianr;
      const char * file = ::solvable_lookup_location( _solvable, &medianr );
      if ( ! file )
        return OnMediaLocation();
      if ( ! medianr )
	medianr = 1;

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


    IdString Solvable::ident() const
    {
      NO_SOLVABLE_RETURN( IdString() );
      return IdString( _solvable->name );
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

      // either explicitly prefixed...
      const char * ident = IdString( _solvable->name ).c_str();
      ResKind knownKind( ResKind::explicitBuiltin( ident ) );
      if ( knownKind )
	return knownKind;

      // ...or no ':' in package names (hopefully)...
      const char * sep = ::strchr( ident, ':' );
      if ( ! sep )
	return ResKind::package;

      // ...or something unknown.
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

    IdString Solvable::vendor() const
    {
      NO_SOLVABLE_RETURN( IdString() );
      return IdString( _solvable->vendor );
    }

   Repository Solvable::repository() const
    {
      NO_SOLVABLE_RETURN( Repository::noRepository );
      return Repository( _solvable->repo );
    }

    RepoInfo Solvable::repoInfo() const
    { return repository().info(); }


    bool Solvable::isSystem() const
    {
      NO_SOLVABLE_RETURN( _id == detail::systemSolvableId );
      return myPool().isSystemRepo( _solvable->repo );
    }

    bool Solvable::onSystemByUser() const
    {
      return isSystem() && myPool().isOnSystemByUser( ident() );
    }

    bool Solvable::multiversionInstall() const
    {
      NO_SOLVABLE_RETURN( false );
      return myPool().isMultiversion( *this );
    }

    Date Solvable::buildtime() const
    {
      NO_SOLVABLE_RETURN( Date() );
      return Date( lookupNumAttribute( SolvAttr::buildtime ) );
    }

    Date Solvable::installtime() const
    {
      NO_SOLVABLE_RETURN( Date() );
      return Date( lookupNumAttribute( SolvAttr::installtime ) );
    }

    std::string Solvable::asString() const
    {
      NO_SOLVABLE_RETURN( (_id == detail::systemSolvableId ? "systemSolvable" : "noSolvable") );
      return str::form( "%s-%s.%s",
                        IdString( _solvable->name ).c_str(),
                        IdString( _solvable->evr ).c_str(),
                        IdString( _solvable->arch ).c_str() );
    }

    std::string Solvable::asUserString() const\
    {
      NO_SOLVABLE_RETURN( (_id == detail::systemSolvableId ? "systemSolvable" : "noSolvable") );
      return str::form( "%s-%s.%s (%s)",
                        IdString( _solvable->name ).c_str(),
                        IdString( _solvable->evr ).c_str(),
                        IdString( _solvable->arch ).c_str(),
                        repository().asUserString().c_str() );
    }

    bool Solvable::identical( const Solvable & rhs ) const
    {
      NO_SOLVABLE_RETURN( ! rhs.get() );
      detail::CSolvable * rhssolvable( rhs.get() );
      return rhssolvable && ( _solvable == rhssolvable || ::solvable_identical( _solvable, rhssolvable ) );
    }

    ///////////////////////////////////////////////////////////////////
    namespace
    {
      inline Capabilities _getCapabilities( detail::IdType * idarraydata_r, ::Offset offs_r )
      {
	return offs_r ? Capabilities( idarraydata_r + offs_r ) : Capabilities();
      }
    } // namespace
    ///////////////////////////////////////////////////////////////////

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

    ///////////////////////////////////////////////////////////////////
    namespace
    {
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
      inline int invokeOnEachSupportedLocale( Capabilities cap_r, function<bool (Locale)> fnc_r )
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
    } // namespace
    ///////////////////////////////////////////////////////////////////

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

    LocaleSet Solvable::getSupportedLocales() const
    {
      LocaleSet ret;
      invokeOnEachSupportedLocale( supplements(), functor::collector( std::inserter( ret, ret.begin() ) ) );
      return ret;
    }

    CpeId Solvable::cpeId() const
    {
      NO_SOLVABLE_RETURN( CpeId() );
      return CpeId( lookupStrAttribute( SolvAttr::cpeid ), CpeId::noThrow );
    }

    unsigned Solvable::mediaNr() const
    {
      NO_SOLVABLE_RETURN( 0U );
      return lookupNumAttribute( SolvAttr::medianr );
    }

    ByteCount Solvable::installSize() const
    {
      NO_SOLVABLE_RETURN( ByteCount() );
      return ByteCount( lookupNumAttribute( SolvAttr::installsize ) );
    }

    ByteCount Solvable::downloadSize() const
    {
      NO_SOLVABLE_RETURN( ByteCount() );
      return ByteCount( lookupNumAttribute( SolvAttr::downloadsize ) );
    }

    std::string Solvable::distribution() const
    {
      NO_SOLVABLE_RETURN( std::string() );
      return lookupStrAttribute( SolvAttr::distribution );
    }

    std::string	Solvable::summary( const Locale & lang_r ) const
    {
      NO_SOLVABLE_RETURN( std::string() );
      return lookupStrAttribute( SolvAttr::summary, lang_r );
    }

    std::string	Solvable::description( const Locale & lang_r ) const
    {
      NO_SOLVABLE_RETURN( std::string() );
      return lookupStrAttribute( SolvAttr::description, lang_r );
    }

    std::string	Solvable::insnotify( const Locale & lang_r ) const
    {
      NO_SOLVABLE_RETURN( std::string() );
      return lookupStrAttribute( SolvAttr::insnotify, lang_r );
    }

    std::string	Solvable::delnotify( const Locale & lang_r ) const
    {
      NO_SOLVABLE_RETURN( std::string() );
      return lookupStrAttribute( SolvAttr::delnotify, lang_r );
    }

    std::string	Solvable::licenseToConfirm( const Locale & lang_r ) const
    {
      NO_SOLVABLE_RETURN( std::string() );
      std::string ret = lookupStrAttribute( SolvAttr::eula, lang_r );
      if ( ret.empty() && isKind<Product>() )
      {
	const RepoInfo & ri( repoInfo() );
	if ( ri.needToAcceptLicense() || ! ui::Selectable::get( *this )->hasInstalledObj() )
	  ret = ri.getLicense( lang_r ); // bnc#908976: suppress informal license upon update
      }
      return ret;
    }

    bool Solvable::needToAcceptLicense() const
    {
      NO_SOLVABLE_RETURN( false );
      return ( isKind<Product>() ? repoInfo().needToAcceptLicense() : true );
    }


    std::ostream & operator<<( std::ostream & str, const Solvable & obj )
    {
      if ( ! obj )
        return str << (obj.isSystem() ? "systemSolvable" : "noSolvable" );

      return str << "(" << obj.id() << ")"
          << ( obj.isKind( ResKind::srcpackage ) ? "srcpackage:" : "" ) << obj.ident()
          << '-' << obj.edition() << '.' << obj.arch() << "("
          << obj.repository().alias() << ")";
    }

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

    std::ostream & dumpAsXmlOn( std::ostream & str, const Solvable & obj )
    {
      xmlout::Node guard( str, "solvable" );

      dumpAsXmlOn( *guard, obj.kind() );
      *xmlout::Node( *guard, "name" ) << obj.name();
      dumpAsXmlOn( *guard, obj.edition() );
      dumpAsXmlOn( *guard, obj.arch() );
      dumpAsXmlOn( *guard, obj.repository() );
      return str;
    }

  } // namespace sat
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
