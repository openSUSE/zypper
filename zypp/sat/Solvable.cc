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
#include "zypp/sat/Repo.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    const Solvable Solvable::nosolvable;

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
      NO_SOLVABLE_RETURN( nosolvable );
      for ( detail::SolvableIdType next = _id+1; next < unsigned(_solvable->repo->end); ++next )
      {
        ::_Solvable * nextS( myPool().getSolvable( next ) );
        if ( nextS && nextS->repo == _solvable->repo )
        {
          return Solvable( next );
        }
      }
      return nosolvable;
    }

    Repo Solvable::repo() const
    {
      NO_SOLVABLE_RETURN( Repo::norepo );
      return Repo( _solvable->repo );
    }

    bool Solvable::isSystem() const
    { return repo().isSystemRepo(); }

    IdString Solvable::ident() const
    {
      NO_SOLVABLE_RETURN( IdString() );
      return IdString( _solvable->name );
    }

    std::string Solvable::lookupStrAttribute( const SolvAttr &attr ) const
    {
      const char *s = repo_lookup_str(this->get(), attr.idStr().id());
      return s ? s : std::string();
    }

    unsigned Solvable::lookupNumAttribute( const SolvAttr &attr ) const
    {
      return repo_lookup_num(this->get(), attr.idStr().id());
    }

    bool Solvable::lookupBoolAttribute( const SolvAttr &attr ) const
    {
      return repo_lookup_num(this->get(), attr.idStr().id()) > 0;
    }

    struct LocCallback
    {
      unsigned medianr;
      const char *mediadir;
      const char *mediafile;
      int trivial;
    };

    static int
    location_cb (void *vcbdata, ::Solvable *s, ::Repodata *data, ::Repokey *key, ::KeyValue *kv)
    {
      LocCallback *lc = (LocCallback *)vcbdata;
      switch (key->type)
      {
        case TYPE_ID:
        if (key->name == SolvAttr::mediadir.idStr().id())
        {
          if (data->localpool)
            lc->mediadir = stringpool_id2str(&data->spool, kv->id);
          else
            lc->mediadir = id2str(data->repo->pool, kv->id);
        }
        break;
        case TYPE_STR:
        if (key->name == SolvAttr::mediafile.idStr().id())
          lc->mediafile = kv->str;
        break;
          case TYPE_VOID:
        if (key->name == SolvAttr::mediafile.idStr().id())
          lc->trivial = 1;
        break;
          case TYPE_CONSTANT:
        if (key->name == SolvAttr::medianr.idStr().id())
          lc->medianr = kv->num;
        break;
      }
      /* continue walking */
      return 0;
    }

    std::string Solvable::lookupLocation(unsigned &medianr) const
    {
      NO_SOLVABLE_RETURN( std::string() );
      ::Repo *repo = _solvable->repo;
      ::Pool *pool = repo->pool;
      Id sid = _solvable - pool->solvables;
      ::Repodata *data;
      unsigned i;
      LocCallback lc;
      lc.medianr = 1;
      lc.mediadir = 0;
      lc.mediafile = 0;
      lc.trivial = 0;
      for (i = 0, data = repo->repodata; i < repo->nrepodata; i++, data++)
      {
        if (data->state == REPODATA_STUB || data->state == REPODATA_ERROR)
          continue;
        if (sid < data->start || sid >= data->end)
          continue;
        repodata_search(data, sid - data->start, 0, location_cb, &lc);
      }
      medianr = lc.medianr;
      std::string ret;

      if (!lc.trivial)
      {
        if (lc.mediafile)
          ret += lc.mediafile;
        return ret;
      }

      if (lc.mediadir)
      {
        ret += std::string( lc.mediadir ) + "/";
      }
      else
      {
        /* If we haven't seen an explicit dirname, then prepend the arch as
           directory.  */
        ret += "suse/";
        ret += IdString(_solvable->arch).asString() + "/";
      }
      /* Trivial means that we can construct the rpm name from our
         solvable data, as name-evr.arch.rpm .  */
      ret += IdString(_solvable->name).asString();
      ret += '-';
      ret += IdString(_solvable->evr).asString();
      ret += '.';
      ret += IdString(_solvable->arch).asString();
      ret += ".rpm";
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
          case 'i': OUTS( script, 6 );      break;
          case 'k': OUTS( package, 7 );     break;
          case 'm': OUTS( atom, 4 );        break;
          case 'p': OUTS( srcpackage, 10 ); break;
          case 's': OUTS( message, 7 );     break;
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
      if ( kind_r == ResKind::srcpackage )
      {
        return( _solvable->arch == ARCH_SRC || _solvable->arch == ARCH_NOSRC );
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
        case Dep::FRESHENS_e:    return freshens();    break;
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
    Capabilities Solvable::freshens() const
    {
      NO_SOLVABLE_RETURN( Capabilities() );
      return _getCapabilities( _solvable->repo->idarraydata, _solvable->freshens );
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
        return str << "sat::solvable()";

      return str << "(" << obj.id() << ")"
          << ( obj.isKind( ResKind::srcpackage ) ? "srcpackage:" : "" ) << obj.ident()
          << '-' << obj.edition() << '.' << obj.arch() << "("
          << obj.repo().name() << ")";
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
        OUTS(FRESHENS);
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
