/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/pool/ByIdent.h
 *
*/
#ifndef ZYPP_POOL_BYIDENT_H
#define ZYPP_POOL_BYIDENT_H

#include "zypp/PoolItem.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace pool
  { /////////////////////////////////////////////////////////////////

    /** Main filter selecting PoolItems by \c name and \c kind.
     */
    class ByIdent
    {
      public:
        ByIdent()
        : _id( 0 )
        {}

        explicit ByIdent( sat::Solvable slv_r )
        : _id( makeIdent( slv_r ) )
        {}

        explicit ByIdent( IdString ident_r )
        : _id( ident_r.id() )
        {}

        ByIdent( ResKind kind_r, IdString name_r )
        : _id( makeIdent( kind_r, name_r ) )
        {}

        ByIdent( ResKind kind_r, const C_Str & name_r )
        : _id( makeIdent( kind_r, name_r ) )
        {}

      public:
        bool operator()( sat::Solvable slv_r ) const
        {
          return _id >= 0 ? ( slv_r.ident().id() == _id && ! slv_r.isKind( ResKind::srcpackage ) )
            : ( slv_r.ident().id() == -_id && slv_r.isKind( ResKind::srcpackage ) );
        }

        bool operator()( const PoolItem & pi_r ) const
        { return operator()( pi_r.satSolvable() ); }

        bool operator()( ResObject::constPtr p_r ) const
        { return p_r ? operator()( p_r->satSolvable() ) : !_id; }

      private:
        sat::detail::IdType makeIdent( sat::Solvable slv_r )
        {
          return slv_r.isKind( ResKind::srcpackage ) ? -slv_r.ident().id()
            : slv_r.ident().id();
        }

        sat::detail::IdType makeIdent( ResKind kind_r, IdString name_r )
        {
          if ( kind_r == ResKind::package )
            return name_r.id();
          else if ( kind_r == ResKind::srcpackage )
            return -name_r.id();
          return IdString( str::form( "%s:%s", kind_r.c_str(), name_r.c_str() ) ).id();
        }

        sat::detail::IdType makeIdent( ResKind kind_r, const C_Str & name_r )
        {
          if ( kind_r == ResKind::package )
            return IdString( name_r ).id();
          else if ( kind_r == ResKind::srcpackage )
            return -(IdString( name_r ).id());
          return IdString( str::form( "%s:%s", kind_r.c_str(), name_r.c_str() ) ).id();
        }

      public:
        sat::detail::IdType get() const { return _id; }

      private:
        /** negative \c _id for \c srcpackage, as they use the same \c ident
         * as \c package.
         */
        sat::detail::IdType _id;
    };

    /////////////////////////////////////////////////////////////////
  } // namespace pool
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_POOL_BYIDENT_H
