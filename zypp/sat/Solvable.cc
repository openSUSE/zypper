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
#include "zypp/sat/IdStr.h"
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

    IdStr Solvable::ident() const
    {
      NO_SOLVABLE_RETURN( IdStr() );
      return IdStr( _solvable->name );
    }

    ResKind Solvable::kind() const
    {
      NO_SOLVABLE_RETURN( ResKind() );
      if ( _kind.empty() )
      {
        switch ( _solvable->arch )
        {
          case ARCH_SRC:
          case ARCH_NOSRC:
            _kind = resKind<SrcPackage>();
            break;

          default:
#warning FIX KindId calc or index
            break;
        }
      }
      return _kind;
    }

    std::string Solvable::name() const
    {
#warning FIX Name skip kind or own Id
      NO_SOLVABLE_RETURN( std::string() );
      return ident().string();
    }

    EvrId Solvable::edition() const
    {
      NO_SOLVABLE_RETURN( EvrId() );
      return EvrId( _solvable->evr );
    }

    ArchId Solvable::arch() const
    {
      NO_SOLVABLE_RETURN( ArchId() );
      switch ( _solvable->arch )
      {
        case ARCH_SRC:
        case ARCH_NOSRC:
          return ArchId( ARCH_NOARCH );
          break;
      }
      return ArchId( _solvable->arch );
    }

    VendorId Solvable::vendor() const
    {
      NO_SOLVABLE_RETURN( VendorId() );
      return VendorId( _solvable->vendor );
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
          << obj.ident() << '-' << obj.edition() << '.' << obj.arch() << "){"
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
