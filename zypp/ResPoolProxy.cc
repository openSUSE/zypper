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
#include "zypp/base/LogTools.h"

#include "zypp/base/Iterator.h"
#include "zypp/base/Algorithm.h"
#include "zypp/base/Functional.h"

#include "zypp/ResPoolProxy.h"
#include "zypp/pool/PoolImpl.h"
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

    void saveState( ResPool pool_r, const ResKind & kind_r )
    {
      std::for_each( pool_r.byKindBegin(kind_r), pool_r.byKindEnd(kind_r),
                     std::mem_fun_ref(&PoolItem::saveState) );
    }

    void restoreState( ResPool pool_r )
    {
      std::for_each( pool_r.begin(), pool_r.end(),
                     std::mem_fun_ref(&PoolItem::restoreState) );
    }

    void restoreState( ResPool pool_r, const ResKind & kind_r )
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

    bool diffState( ResPool pool_r, const ResKind & kind_r ) const
    {
      // return whether some PoolItem::sameState reported \c false.
      return( invokeOnEach( pool_r.byKindBegin(kind_r), pool_r.byKindEnd(kind_r),
                            std::mem_fun_ref(&PoolItem::sameState) ) < 0 );
    }
  };

  namespace
  {
    ui::Selectable::Ptr makeSelectablePtr( pool::PoolImpl::Id2ItemT::const_iterator begin_r,
                                           pool::PoolImpl::Id2ItemT::const_iterator end_r )
    {
      pool::PoolTraits::byIdent_iterator begin( begin_r, pool::PoolTraits::Id2ItemValueSelector() );
      pool::PoolTraits::byIdent_iterator end( end_r, pool::PoolTraits::Id2ItemValueSelector() );
      sat::Solvable solv( begin->satSolvable() );

      return new ui::Selectable( ui::Selectable::Impl_Ptr( new ui::Selectable::Impl( solv.kind(), solv.name(), begin, end ) ) );
    }
  } // namespace

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ResPoolProxy::Impl
  //
  /** ResPoolProxy implementation.
   * \todo Seedup as it is still using old index
  */
  struct ResPoolProxy::Impl
  {
    friend std::ostream & operator<<( std::ostream & str, const Impl & obj );
    friend std::ostream & dumpOn( std::ostream & str, const Impl & obj );

    typedef std::unordered_map<sat::detail::IdType,ui::Selectable::Ptr> SelectableIndex;
    typedef ResPoolProxy::const_iterator const_iterator;

  public:
    Impl()
    :_pool( ResPool::instance() )
    {}

    Impl( ResPool pool_r, const pool::PoolImpl & poolImpl_r )
    : _pool( pool_r )
    {
      const pool::PoolImpl::Id2ItemT & id2item( poolImpl_r.id2item() );
      if ( ! id2item.empty() )
      {
        // set startpoint
        pool::PoolImpl::Id2ItemT::const_iterator cbegin = id2item.begin();

        for_( it, id2item.begin(), id2item.end() )
        {
          if ( it->first != cbegin->first )
          {
            // starting a new Selectable, create the previous one
            ui::Selectable::Ptr p( makeSelectablePtr( cbegin, it ) );
            _selPool.insert( SelectablePool::value_type( p->kind(), p ) );
            _selIndex[cbegin->first] = p;
            // remember new startpoint
            cbegin = it;
          }
        }
        // create the final one
        ui::Selectable::Ptr p( makeSelectablePtr( cbegin, id2item.end() ) );
        _selPool.insert( SelectablePool::value_type( p->kind(), p ) );
        _selIndex[cbegin->first] = p;
      }
    }

  public:
    ui::Selectable::Ptr lookup( const pool::ByIdent & ident_r ) const
    {
      SelectableIndex::const_iterator it( _selIndex.find( ident_r.get() ) );
      if ( it != _selIndex.end() )
        return it->second;
      return ui::Selectable::Ptr();
    }

  public:
    bool empty() const
    { return _selPool.empty(); }

    size_type size() const
    { return _selPool.size(); }

    const_iterator begin() const
    { return make_map_value_begin( _selPool ); }

    const_iterator end() const
    { return make_map_value_end( _selPool ); }

  public:
    bool empty( const ResKind & kind_r ) const
    { return( _selPool.count( kind_r ) == 0 );  }

    size_type size( const ResKind & kind_r ) const
    { return _selPool.count( kind_r ); }

    const_iterator byKindBegin( const ResKind & kind_r ) const
    { return make_map_value_lower_bound( _selPool, kind_r ); }

    const_iterator byKindEnd( const ResKind & kind_r ) const
    { return make_map_value_upper_bound( _selPool, kind_r ); }

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

    void saveState( const ResKind & kind_r ) const
    { PoolItemSaver().saveState( _pool, kind_r ); }

    void restoreState() const
    { PoolItemSaver().restoreState( _pool ); }

    void restoreState( const ResKind & kind_r ) const
    { PoolItemSaver().restoreState( _pool, kind_r ); }

    bool diffState() const
    { return PoolItemSaver().diffState( _pool ); }

    bool diffState( const ResKind & kind_r ) const
    { return PoolItemSaver().diffState( _pool, kind_r ); }

  private:
    ResPool _pool;
    mutable SelectablePool _selPool;
    mutable SelectableIndex _selIndex;

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
    return str << "ResPoolProxy (" << obj._pool.serial() << ") [" << obj._pool.size()
               << "solv/" << obj.size()<< "sel]";
  }

  namespace detail
  {
    struct DumpFilter
    {
      bool operator()( const ui::Selectable::Ptr & selp ) const
      { return selp->toModify(); }
    };
  }

  /** \relates ResPoolProxy::Impl Verbose stream output */
  inline std::ostream & dumpOn( std::ostream & str, const ResPoolProxy::Impl & obj )
  {
    detail::DumpFilter f;
    return dumpRange( str << obj << " toModify: ",
		      make_filter_begin( f, obj ),
		      make_filter_end( f, obj ) );
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
  ResPoolProxy::ResPoolProxy( ResPool pool_r, const pool::PoolImpl & poolImpl_r )
  : _pimpl( new Impl( pool_r, poolImpl_r ) )
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

  ui::Selectable::Ptr ResPoolProxy::lookup( const pool::ByIdent & ident_r ) const
  { return _pimpl->lookup( ident_r ); }

  bool ResPoolProxy::empty() const
  { return _pimpl->empty(); }

  ResPoolProxy::size_type ResPoolProxy::size() const
  { return _pimpl->size(); }

  ResPoolProxy::const_iterator ResPoolProxy::begin() const
  { return _pimpl->begin(); }

  ResPoolProxy::const_iterator ResPoolProxy::end() const
  { return _pimpl->end(); }

  bool ResPoolProxy::empty( const ResKind & kind_r ) const
  { return _pimpl->empty( kind_r ); }

  ResPoolProxy::size_type ResPoolProxy::size( const ResKind & kind_r ) const
  { return _pimpl->size( kind_r ); }

  ResPoolProxy::const_iterator ResPoolProxy::byKindBegin( const ResKind & kind_r ) const
  { return _pimpl->byKindBegin( kind_r ); }

  ResPoolProxy::const_iterator ResPoolProxy::byKindEnd( const ResKind & kind_r ) const
  { return _pimpl->byKindEnd( kind_r ); }

  ResPoolProxy::size_type ResPoolProxy::knownRepositoriesSize() const
  { return _pimpl->knownRepositoriesSize(); }

  ResPoolProxy::repository_iterator ResPoolProxy::knownRepositoriesBegin() const
  { return _pimpl->knownRepositoriesBegin(); }

  ResPoolProxy::repository_iterator ResPoolProxy::knownRepositoriesEnd() const
  { return _pimpl->knownRepositoriesEnd(); }

  void ResPoolProxy::saveState() const
  { _pimpl->saveState(); }

  void ResPoolProxy::saveState( const ResKind & kind_r ) const
  { _pimpl->saveState( kind_r ); }

  void ResPoolProxy::restoreState() const
  { _pimpl->restoreState(); }

  void ResPoolProxy::restoreState( const ResKind & kind_r ) const
  { _pimpl->restoreState( kind_r ); }

  bool ResPoolProxy::diffState() const
  { return _pimpl->diffState(); }

  bool ResPoolProxy::diffState( const ResKind & kind_r ) const
  { return _pimpl->diffState( kind_r ); }

  std::ostream & operator<<( std::ostream & str, const ResPoolProxy & obj )
  { return str << *obj._pimpl; }

  std::ostream & dumpOn( std::ostream & str, const ResPoolProxy & obj )
  { return dumpOn( str, *obj._pimpl ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
