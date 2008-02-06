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

    int Solvable::lookupNumAttribute( const SolvAttr &attr ) const
    {
      return repo_lookup_num(this->get(), attr.idStr().id());
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
#define OUTS(K,S) if ( ::strncmp( ident, ResKind::K.c_str(), S ) ) return ResKind::K
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
      NO_SOLVABLE_RETURN( Capabilities() );
      ::Offset offs = 0;
      switch( which_r.inSwitch() )
      {
        case Dep::PROVIDES_e:    offs = _solvable->provides;    break;
        case Dep::REQUIRES_e:    offs = _solvable->requires;    break;
        case Dep::CONFLICTS_e:   offs = _solvable->conflicts;   break;
        case Dep::OBSOLETES_e:   offs = _solvable->obsoletes;   break;
        case Dep::RECOMMENDS_e:  offs = _solvable->recommends;  break;
        case Dep::SUGGESTS_e:    offs = _solvable->suggests;    break;
        case Dep::FRESHENS_e:    offs = _solvable->freshens;    break;
        case Dep::ENHANCES_e:    offs = _solvable->enhances;    break;
        case Dep::SUPPLEMENTS_e: offs = _solvable->supplements; break;

        case Dep::PREREQUIRES_e:
          // prerequires are a subset of requires
          if ( (offs = _solvable->requires) )
            return Capabilities( _solvable->repo->idarraydata + offs, detail::solvablePrereqMarker );
          else
            return Capabilities();
          break;
      }

      return offs ? Capabilities( _solvable->repo->idarraydata + offs )
                  : Capabilities();
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

      return str << "sat::solvable(" << obj.id() << "|"
          << ( obj.isKind( ResKind::srcpackage ) ? "srcpackage:" : "" ) << obj.ident()
          << '-' << obj.edition() << '.' << obj.arch() << "){"
          << obj.repo().name() << "}";
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
