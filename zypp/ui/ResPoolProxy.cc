/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ui/ResPoolProxy.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp/base/Iterator.h"
#include "zypp/base/Algorithm.h"
#include "zypp/base/Functional.h"

#include "zypp/ui/ResPoolProxy.h"
#include "zypp/ui/SelectableImpl.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace ui
  { /////////////////////////////////////////////////////////////////

    struct SelPoolHelper
    {
      typedef std::set<ResPool::Item>         ItemC;
      struct SelC
      {
        void add( ResPool::Item it )
        {
          if ( it.status().isInstalled() )
            installed.insert( it );
          else
            available.insert( it );
        }
        ItemC installed;
        ItemC available;
      };
      typedef std::map<std::string,SelC>      NameC;
      typedef std::map<ResObject::Kind,NameC> KindC;

      KindC _kinds;

      /** collect from a pool */
      void operator()( ResPool::Item it )
      {
        _kinds[it->kind()][it->name()].add( it );
      }


      Selectable::Ptr buildSelectable( const ResObject::Kind & kind_r,
                                       const std::string & name_r,
                                       const PoolItem & installedItem_r,
                                       const ItemC & available )
      {
        return Selectable::Ptr( new Selectable(
               Selectable::Impl_Ptr( new Selectable::Impl( kind_r, name_r,
                                                           installedItem_r,
                                                           available.begin(),
                                                           available.end() ) )
                                              ) );
      }

      /** Build Selectable::Ptr and feed them to some container. */
      template<class _OutputIterator>
        void feed( _OutputIterator result_r )
        {
          for ( KindC::const_iterator kindIt = _kinds.begin(); kindIt != _kinds.end(); ++kindIt )
            {
              for ( NameC::const_iterator nameIt = kindIt->second.begin(); nameIt != kindIt->second.end(); ++nameIt )
                {
                  const ItemC & installed( nameIt->second.installed );
                  const ItemC & available( nameIt->second.available );

                  if ( installed.empty() )
                    {
                      if ( available.empty() )
                        continue;
                      *result_r = buildSelectable( kindIt->first, nameIt->first, PoolItem(), available );
                      ++result_r;
                    }
                  else
                    {
                      // ui want's one Selectable per installed item
                      for ( ItemC::const_iterator instIt = installed.begin(); instIt != installed.end(); ++instIt )
                        {
                          *result_r = buildSelectable( kindIt->first, nameIt->first, *instIt, available );
                          ++result_r;
                        }
                    }
                }
            }
        }
    };

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ResPoolProxy::Impl
    //
    /** ResPoolProxy implementation. */
    struct ResPoolProxy::Impl
    {
    public:

      typedef std::set<Selectable::Ptr> SelPool;

    public:
      Impl()
      {}

      Impl( ResPool_Ref pool_r )
      : _pool( pool_r )
      {

        SelPoolHelper collect;
        std::for_each( _pool.begin(), _pool.end(),
                       functor::functorRef<void,ResPool::Item>( collect ) );
        collect.feed( std::inserter( _selectables, _selectables.end() ) );
      }

    private:
      ResPool_Ref _pool;
      SelPool     _selectables;

    public:
      /** Offer default Impl. */
      static shared_ptr<Impl> nullimpl()
      {
        static shared_ptr<Impl> _nullimpl( new Impl );
        return _nullimpl;
      }
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates ResPoolProxy::Impl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const ResPoolProxy::Impl & obj )
    {
      return str << "ResPoolProxy::Impl";
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ResPoolProxy
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ResPoolProxy::ResPoolProxy
    //	METHOD TYPE : Ctor
    //
    ResPoolProxy::ResPoolProxy()
    : _pimpl( Impl::nullimpl() )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ResPoolProxy::ResPoolProxy
    //	METHOD TYPE : Ctor
    //
    ResPoolProxy::ResPoolProxy( ResPool_Ref pool_r )
    : _pimpl( new Impl( pool_r ) )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ResPoolProxy::~ResPoolProxy
    //	METHOD TYPE : Dtor
    //
    ResPoolProxy::~ResPoolProxy()
    {}

    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const ResPoolProxy & obj )
    {
      return str << *obj._pimpl;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
