/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ResPoolProxy.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp/base/Iterator.h"
#include "zypp/base/Algorithm.h"
#include "zypp/base/Functional.h"

#include "zypp/ResPoolProxy.h"
#include "zypp/ui/SelectableImpl.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /** Tem. friend of PoolItem */
  struct PoolItemSaver
  {
    void saveState( ResPool pool_r )
    {
      std::for_each( pool_r.begin(), pool_r.end(),
                     std::mem_fun_ref(&PoolItem::saveState) );
    }

    void saveState( ResPool pool_r, const ResObject::Kind & kind_r )
    {
      std::for_each( pool_r.byKindBegin(kind_r), pool_r.byKindEnd(kind_r),
                     std::mem_fun_ref(&PoolItem::saveState) );
    }

    void restoreState( ResPool pool_r )
    {
      std::for_each( pool_r.begin(), pool_r.end(),
                     std::mem_fun_ref(&PoolItem::restoreState) );
    }

    void restoreState( ResPool pool_r, const ResObject::Kind & kind_r )
    {
      std::for_each( pool_r.byKindBegin(kind_r), pool_r.byKindEnd(kind_r),
                     std::mem_fun_ref(&PoolItem::restoreState) );
    }

    bool diffState( ResPool pool_r ) const
    {
      // return whether some PoolItem::sameState reported \c false.
      return( invokeOnEach( pool_r.begin(), pool_r.end(),
                            std::mem_fun_ref(&PoolItem::sameState) ) < 0 );
    }

    bool diffState( ResPool pool_r, const ResObject::Kind & kind_r ) const
    {
      // return whether some PoolItem::sameState reported \c false.
      return( invokeOnEach( pool_r.byKindBegin(kind_r), pool_r.byKindEnd(kind_r),
                            std::mem_fun_ref(&PoolItem::sameState) ) < 0 );
    }
  };

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


    ui::Selectable::Ptr buildSelectable( const ResObject::Kind & kind_r,
                                         const std::string & name_r,
                                         const PoolItem & installedItem_r,
                                         const ItemC & available )
    {
      return ui::Selectable::Ptr( new ui::Selectable(
             ui::Selectable::Impl_Ptr( new ui::Selectable::Impl( kind_r, name_r,
                                                                 installedItem_r,
                                                                 available.begin(),
                                                                 available.end() ) )
                                                      ) );
    }

    /** Build Selectable::Ptr and feed them to some container.
     * \todo Cleanup typedefs
    */
    typedef std::set<ui::Selectable::Ptr>             SelectableIndex;
    typedef std::map<ResObject::Kind,SelectableIndex> SelectablePool;

    void feed( SelectablePool & _selPool )
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
                  _selPool[kindIt->first].insert( buildSelectable( kindIt->first, nameIt->first, PoolItem(), available ) );
                }
              else
                {
                  // ui want's one Selectable per installed item
                  for ( ItemC::const_iterator instIt = installed.begin(); instIt != installed.end(); ++instIt )
                    {
                      _selPool[kindIt->first].insert( buildSelectable( kindIt->first, nameIt->first, *instIt, available ) );
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
    Impl()
    :_pool( ResPool::instance() )
    {}

    Impl( ResPool pool_r )
    : _pool( pool_r )
    {
      SelPoolHelper collect;
      std::for_each( _pool.begin(), _pool.end(),
                     functor::functorRef<void,ResPool::Item>( collect ) );
      collect.feed( _selPool );
    }

  public:

    bool empty( const ResObject::Kind & kind_r ) const
    { return _selPool[kind_r].empty(); }

    size_type size( const ResObject::Kind & kind_r ) const
    { return _selPool[kind_r].size(); }

    const_iterator byKindBegin( const ResObject::Kind & kind_r ) const
    { return _selPool[kind_r].begin(); }

    const_iterator byKindEnd( const ResObject::Kind & kind_r ) const
    { return _selPool[kind_r].end(); }

  public:
    size_type knownRepositoriesSize() const
    { return _pool.knownRepositoriesSize(); }

    repository_iterator knownRepositoriesBegin() const
    { return _pool.knownRepositoriesBegin(); }

    repository_iterator knownRepositoriesEnd() const
    { return _pool.knownRepositoriesEnd(); }

  public:

    void saveState() const
    { PoolItemSaver().saveState( _pool ); }

    void saveState( const ResObject::Kind & kind_r ) const
    { PoolItemSaver().saveState( _pool, kind_r ); }

    void restoreState() const
    { PoolItemSaver().restoreState( _pool ); }

    void restoreState( const ResObject::Kind & kind_r ) const
    { PoolItemSaver().restoreState( _pool, kind_r ); }

    bool diffState() const
    { return PoolItemSaver().diffState( _pool ); }

    bool diffState( const ResObject::Kind & kind_r ) const
    { return PoolItemSaver().diffState( _pool, kind_r ); }

  private:
    ResPool _pool;
    mutable SelectablePool _selPool;

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
  ResPoolProxy::ResPoolProxy( ResPool pool_r )
  : _pimpl( new Impl( pool_r ) )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResPoolProxy::~ResPoolProxy
  //	METHOD TYPE : Dtor
  //
  ResPoolProxy::~ResPoolProxy()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  // forward to implementation
  //
  ///////////////////////////////////////////////////////////////////

  bool ResPoolProxy::empty( const ResObject::Kind & kind_r ) const
  { return _pimpl->empty( kind_r ); }

  ResPoolProxy::size_type ResPoolProxy::size( const ResObject::Kind & kind_r ) const
  { return _pimpl->size( kind_r ); }

  ResPoolProxy::const_iterator ResPoolProxy::byKindBegin( const ResObject::Kind & kind_r ) const
  { return _pimpl->byKindBegin( kind_r ); }

  ResPoolProxy::const_iterator ResPoolProxy::byKindEnd( const ResObject::Kind & kind_r ) const
  { return _pimpl->byKindEnd( kind_r ); }

  ResPoolProxy::size_type ResPoolProxy::knownRepositoriesSize() const
  { return _pimpl->knownRepositoriesSize(); }

  ResPoolProxy::repository_iterator ResPoolProxy::knownRepositoriesBegin() const
  { return _pimpl->knownRepositoriesBegin(); }

  ResPoolProxy::repository_iterator ResPoolProxy::knownRepositoriesEnd() const
  { return _pimpl->knownRepositoriesEnd(); }

  void ResPoolProxy::saveState() const
  { _pimpl->saveState(); }

  void ResPoolProxy::saveState( const ResObject::Kind & kind_r ) const
  { _pimpl->saveState( kind_r ); }

  void ResPoolProxy::restoreState() const
  { _pimpl->restoreState(); }

  void ResPoolProxy::restoreState( const ResObject::Kind & kind_r ) const
  { _pimpl->restoreState( kind_r ); }

  bool ResPoolProxy::diffState() const
  { return _pimpl->diffState(); }

  bool ResPoolProxy::diffState( const ResObject::Kind & kind_r ) const
  { return _pimpl->diffState( kind_r ); }

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
} // namespace zypp
///////////////////////////////////////////////////////////////////
