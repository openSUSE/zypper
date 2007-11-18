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
extern "C"
{
#include <satsolver/solvable.h>
#include <satsolver/poolid.h>
#include <satsolver/repo.h>
}
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Exception.h"

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
      return str << "sat::Id(" << obj.get() << ")";
    }

    const Solvable Solvable::nosolvable( NULL );

    NameId Solvable::name() const
    {
      if ( ! _solvable ) return NameId::noid;
      return _solvable->name; }

    EvrId Solvable::evr() const
    {
      if ( ! _solvable ) return EvrId::noid;
      return _solvable->arch;
    }

    ArchId Solvable::arch() const
    {
      if ( ! _solvable ) return ArchId::noid;
      return _solvable->evr;
    }

    VendorId Solvable::vendor() const
    {
      if ( ! _solvable ) return VendorId::noid;
      return _solvable->vendor;
    }

    Repo Solvable::repo() const
    {
      if ( ! _solvable ) return 0;
      return _solvable->repo;
    }

    const char * Solvable::string( const PoolId & id_r ) const
    {
      if ( ! _solvable ) return "";
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

      return str << "sat::solvable(" << obj.name() << '-' << obj.evr() << '.' << obj.arch() << "){"
          << obj.repo().name() << "}";
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : SolvableIterator
    //
    ///////////////////////////////////////////////////////////////////

    void SolvableIterator::increment()
    { ++base_reference(); }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
