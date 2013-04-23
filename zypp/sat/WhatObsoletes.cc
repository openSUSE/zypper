/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/WhatObsoletes.cc
 *
*/
#include <iostream>

#include "zypp/base/LogTools.h"
#include "zypp/base/Tr1hash.h"
#include "zypp/sat/WhatObsoletes.h"
#include "zypp/sat/detail/PoolImpl.h"
#include "zypp/PoolItem.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    // Obsoletes may either match against provides, or names.
    // Configuration depends on the behaviour of rpm.
#ifdef _RPM_5
    bool obsoleteUsesProvides = true;
#else
    bool obsoleteUsesProvides = false;
#endif

    ///////////////////////////////////////////////////////////////////
    namespace
    { /////////////////////////////////////////////////////////////////

      typedef std::tr1::unordered_set<detail::IdType> set_type;
      typedef std::vector<sat::detail::IdType>        vector_type;

      /////////////////////////////////////////////////////////////////
    } // namespace
    ///////////////////////////////////////////////////////////////////

    WhatObsoletes::WhatObsoletes( Solvable item_r )
    : _begin( 0 )
    {
      ctorAdd( item_r );
      ctorDone();
    }

    WhatObsoletes::WhatObsoletes( const PoolItem & item_r )
    : _begin( 0 )
    {
      ctorAdd( item_r );
      ctorDone();
    }

    WhatObsoletes::WhatObsoletes( const ResObject::constPtr item_r )
    : _begin( 0 )
    {
      if ( item_r )
      {
        ctorAdd( item_r->satSolvable() );
        ctorDone();
      }
    }

    void WhatObsoletes::ctorAdd( const PoolItem & item_r )
    { ctorAdd( item_r->satSolvable() ); }

    void WhatObsoletes::ctorAdd( ResObject_constPtr item_r )
    { if ( item_r ) ctorAdd( item_r->satSolvable() ); }


    namespace
    {
      /** Add item to the set created on demand. */
      inline void addToSet( Solvable item, set_type *& pdata, shared_ptr<void>& _private )
      {
        if ( ! pdata )
        {
          _private.reset( (pdata = new set_type) );
        }
        pdata->insert( item.id() );
      }
    }

    void WhatObsoletes::ctorAdd( Solvable item_r )
    {
      if ( item_r.multiversionInstall() )
        return; // multiversion (rpm -i) does not evaluate any obsoletes

      if ( obsoleteUsesProvides )
      {
        WhatProvides obsoleted( item_r.obsoletes() );
        if ( obsoleted.empty() )
          return;

        // use allocated private data to collect the results
        set_type * pdata = ( _private ? reinterpret_cast<set_type*>( _private.get() ) : 0 );
        for_( it, obsoleted.begin(), obsoleted.end() )
        {
          if ( it->isSystem() )
            addToSet( *it, pdata, _private );
        }
      }
      else // Obsoletes match names
      {
        Capabilities obsoletes( item_r.obsoletes() );
        if ( obsoletes.empty() )
          return;

        // use allocated private data to collect the results
        set_type * pdata = ( _private ? reinterpret_cast<set_type*>( _private.get() ) : 0 );
        for_( it, obsoletes.begin(), obsoletes.end() )
        {
          // For each obsoletes find providers, but with the same name
          IdString ident( it->detail().name() );
          WhatProvides obsoleted( *it );
          for_( iit, obsoleted.begin(), obsoleted.end() )
          {
            if ( iit->isSystem() && iit->ident() == ident )
              addToSet( *iit, pdata, _private );
          }
        }
      }
    }

    void WhatObsoletes::ctorDone()
    {
      if ( _private )
      {
        // copy set to vector and terminate _private
        set_type * sdata = reinterpret_cast<set_type*>( _private.get() );

        vector_type * pdata = new vector_type( sdata->begin(), sdata->end() );
        pdata->push_back( sat::detail::noId );
        _begin = &pdata->front();

        _private.reset( pdata );
      }
    }

    WhatObsoletes::size_type WhatObsoletes::size() const
    {
      if ( ! _begin )
        return 0;

      Capabilities::size_type ret = 0;
      for ( const sat::detail::IdType * end = _begin; *end; ++end )
      {
        ++ret;
      }
      return ret;
    }

    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const WhatObsoletes & obj )
    {
      return dumpRange( str << "(" << obj.size() << ")", obj.begin(), obj.end() );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
