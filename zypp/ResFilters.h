/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ResFilters.h
 *
*/
#ifndef ZYPP_RESFILTERS_H
#define ZYPP_RESFILTERS_H

#include <iosfwd>

#include "zypp/Resolvable.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace resfilter
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    // Predefined filters
    //
    ///////////////////////////////////////////////////////////////////

    struct True
    {
      bool operator()( ResObject::Ptr ) const
      {
        return true;
      }
    };

    True true_c()
    { return True(); }

    ///////////////////////////////////////////////////////////////////

    struct False
    {
      bool operator()( ResObject::Ptr ) const
      {
        return false;
      }
    };

    False false_c()
    { return False(); }

    ///////////////////////////////////////////////////////////////////

    template<class _Condition>
      struct Not
      {
        Not( _Condition cond_r )
        : _cond( cond_r )
        {}
        bool operator()( ResObject::Ptr p ) const
        {
          return ! _cond( p );
        }
        _Condition _cond;
      };

    template<class _Condition>
      Not<_Condition> not_c( _Condition cond_r )
      {
        return Not<_Condition>( cond_r );
      }

    ///////////////////////////////////////////////////////////////////

    template<class _ACondition, class _BCondition>
      struct Chain
      {
        Chain( _ACondition conda_r, _BCondition condb_r )
        : _conda( conda_r )
        , _condb( condb_r )
        {}
        bool operator()( ResObject::Ptr p ) const
        {
          return _conda( p ) && _condb( p );
        }
        _ACondition _conda;
        _BCondition _condb;
      };

    template<class _ACondition, class _BCondition>
      Chain<_ACondition, _BCondition> chain( _ACondition conda_r, _BCondition condb_r )
      {
        return Chain<_ACondition, _BCondition>( conda_r, condb_r );
      }

    ///////////////////////////////////////////////////////////////////
    //
    // Now some Resolvable attributes
    //
    ///////////////////////////////////////////////////////////////////

    struct ByKind
    {
      ByKind( const ResObject::Kind & kind_r )
      : _kind( kind_r )
      {}
      bool operator()( ResObject::Ptr p ) const
      {
        return p->kind() == _kind;
      }
      ResObject::Kind _kind;
    };

    struct ByName
    {
      ByName( const std::string & name_r )
      : _name( name_r )
      {}
      bool operator()( ResObject::Ptr p ) const
      {
        return p->name() == _name;
      }
      std::string _name;
    };


    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace resfilter
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESFILTERS_H
