/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/WhatProvides.cc
 *
*/
#include <iostream>

#include "zypp/base/LogTools.h"
#include "zypp/sat/WhatProvides.h"
#include "zypp/sat/detail/PoolImpl.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : WhatProvides::Impl
    //
    /** WhatProvides implementation date.
     * Stores the offset into a O terminated Id array. Per default
     * libsolvs whatprovidesdata, otherwise private data.
     *
     * As libsolvs whatprovidesdata might be realocated
     * while iterating a result, the iterator takes an
     * <tt>const IdType *const*</tt>. Thats why we explicitly
     * provide _private and pass its adress to the iterator,
     * even if private data are not reallocated.
     */
    class WhatProvides::Impl : protected detail::PoolMember
    {
      public:
        Impl()
        : _offset( 0 ), _private( 0 )
        {}

        Impl( unsigned offset_r )
        : _offset( offset_r ), _private( 0 )
        {}

        Impl( const std::unordered_set<detail::IdType> & ids_r )
        : _offset( 0 ), _private( 0 )
        {
           // use private data to store the result (incl. trailing NULL)
          _pdata.reserve( ids_r.size()+1 );
          _pdata.insert( _pdata.begin(), ids_r.begin(), ids_r.end() );
          _pdata.push_back( detail::noId );

          _private = &_pdata.front(); // ptr to 1st element
        }

      public:
        unsigned                         _offset;
        const detail::IdType *           _private;

      private:
        std::vector<sat::detail::IdType> _pdata;
    };
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace
    { /////////////////////////////////////////////////////////////////

      /** WhatProvides ctor helper collecting providers from Capabilies. */
      template <class Iterator>
      void collectProviders( Iterator begin_r, Iterator end_r, std::unordered_set<detail::IdType> & collect_r )
      {
        for_( it, begin_r, end_r )
        {
          WhatProvides providers( *it );
          for_( prv, providers.begin(), providers.end() )
          {
            collect_r.insert( prv->id() );
          }
        }
      }

      /////////////////////////////////////////////////////////////////
    } //namespace
    ///////////////////////////////////////////////////////////////////

    WhatProvides::WhatProvides()
    {}

    WhatProvides::WhatProvides( Capability cap_r )
    {
      unsigned res( myPool().whatProvides( cap_r ) );
      if ( myPool().whatProvidesData( res ) )
      {
        _pimpl.reset( new Impl( res ) );
      }
      // else: no Impl for empty result.
    }

    WhatProvides::WhatProvides( Capabilities caps_r )
    {
      std::unordered_set<detail::IdType> ids;
      collectProviders( caps_r.begin(), caps_r.end(), ids );
      if ( ! ids.empty() )
      {
        _pimpl.reset( new Impl( ids ) );
      }
      // else: no Impl for empty result.
   }

    WhatProvides::WhatProvides( const CapabilitySet & caps_r )
    {
      std::unordered_set<detail::IdType> ids;
      collectProviders( caps_r.begin(), caps_r.end(), ids );
      if ( ! ids.empty() )
      {
        _pimpl.reset( new Impl( ids ) );
      }
      // else: no Impl for empty result.
   }

    bool WhatProvides::empty() const
    {
      return !_pimpl; // Ctor asserts no Impl for empty result.
    }

    WhatProvides::size_type WhatProvides::size() const
    {
      if ( !_pimpl )
        return 0;

      size_type count = 0;
      for_( it, begin(), end() )
        ++count;
      return count;
    }

    WhatProvides::const_iterator WhatProvides::begin() const
    {
      if ( !_pimpl )
        return const_iterator();

      if ( _pimpl->_private )
        return const_iterator( _pimpl->_private );

      // for libsolvs index use one more indirection, as it might get relocated.
      return const_iterator( &myPool().getPool()->whatprovidesdata, _pimpl->_offset );
    }

    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const WhatProvides & obj )
    {
      return dumpRange( str << "(" << obj.size() << ")", obj.begin(), obj.end() );
    }

    ///////////////////////////////////////////////////////////////////
    namespace detail
    { /////////////////////////////////////////////////////////////////

      std::ostream & operator<<( std::ostream & str, const WhatProvidesIterator & obj )
      {
        str << str::form( "[%5u]", obj._offset );
        str << str::form( "<%p(%p)>", obj.base_reference(), &obj.base_reference() );
        str << str::form( "<%p(%p)>", obj._baseRef, (obj._baseRef ? *obj._baseRef : 0) );
        return str;
      }

      /////////////////////////////////////////////////////////////////
    } //namespace detail
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
