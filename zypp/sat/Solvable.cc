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
        if ( nextS &&  nextS->repo == _solvable->repo )
        {
          return Solvable( next );
        }
      }
      return nosolvable;
    }

    NameId Solvable::name() const
    {
      NO_SOLVABLE_RETURN( NameId() );
      return NameId( _solvable->name ); }

    EvrId Solvable::evr() const
    {
      NO_SOLVABLE_RETURN( EvrId() );
      return EvrId( _solvable->evr );
    }

    ArchId Solvable::arch() const
    {
      NO_SOLVABLE_RETURN( ArchId() );
      return ArchId( _solvable->arch );
    }

    VendorId Solvable::vendor() const
    {
      NO_SOLVABLE_RETURN( VendorId() );
      return VendorId( _solvable->vendor );
    }

    Repo Solvable::repo() const
    {
      NO_SOLVABLE_RETURN( Repo::norepo );
      return Repo( _solvable->repo );
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

      return str << "sat::solvable(" << obj.id() << "|" << obj.name() << '-' << obj.evr() << '.' << obj.arch() << "){"
          << obj.repo().name() << "}";
    }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
