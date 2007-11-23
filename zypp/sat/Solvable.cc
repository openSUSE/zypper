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

    const PoolId PoolId::noid;

    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const PoolId & obj )
    {
      return str << ::id2str( detail::PoolMember::myPool().getPool(), obj.get() );
      return str << "sat::Id(" << obj.get() << ")";
    }

    /////////////////////////////////////////////////////////////////

    const Solvable Solvable::nosolvable;

    /////////////////////////////////////////////////////////////////

    ::_Solvable * Solvable::get() const
    { return myPool().getSolvable( _id ); }

    Solvable Solvable::nextInPool() const
    { return Solvable( myPool().getNextId( _id ) ); }

#define NO_SOLVABLE_RETURN( VAL ) \
    ::_Solvable * _solvable( get() ); \
    if ( ! _solvable ) return VAL

    NameId Solvable::name() const
    {
      NO_SOLVABLE_RETURN( NameId::noid );
      return _solvable->name; }

    EvrId Solvable::evr() const
    {
      NO_SOLVABLE_RETURN( EvrId::noid );
      return _solvable->arch;
    }

    ArchId Solvable::arch() const
    {
      NO_SOLVABLE_RETURN( ArchId::noid );
      return _solvable->evr;
    }

    VendorId Solvable::vendor() const
    {
      NO_SOLVABLE_RETURN( VendorId::noid );
      return _solvable->vendor;
    }

    Repo Solvable::repo() const
    {
      NO_SOLVABLE_RETURN( Repo::norepo );
      return Repo( _solvable->repo );
    }

    const char * Solvable::string( const PoolId & id_r ) const
    {
      NO_SOLVABLE_RETURN( "" );
      SEC << id_r<< endl;
      SEC << "   n " << ::id2str( _solvable->repo->pool, id_r.get() ) << endl;
      SEC << "   e " << ::dep2str( _solvable->repo->pool, id_r.get() )<< endl;
      SEC << "   a " << ::id2rel( _solvable->repo->pool, id_r.get() )<< endl;
      SEC << "   v " << ::id2evr( _solvable->repo->pool, id_r.get() )<< endl;
      return ::id2str( _solvable->repo->pool, id_r.get() );
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
