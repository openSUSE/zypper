/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ResPoolProxy.h
 *
*/
#ifndef ZYPP_RESPOOLPROXY_H
#define ZYPP_RESPOOLPROXY_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"

#include "zypp/ResPool.h"
#include "zypp/ui/Selectable.h"
#include "zypp/ui/SelFilters.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ResPoolProxy
  //
  /** ResPool::instance().proxy();
   * \todo integrate it into ResPool
  */
  class ResPoolProxy
  {
    friend std::ostream & operator<<( std::ostream & str, const ResPoolProxy & obj );
    friend std::ostream & dumpOn( std::ostream & str, const ResPoolProxy & obj );
    typedef std::multimap<ResKind,ui::Selectable::Ptr> SelectablePool;

  public:
    /** Implementation  */
    class Impl;

    typedef MapKVIteratorTraits<SelectablePool>::Value_const_iterator const_iterator;
    typedef SelectablePool::size_type size_type;

    typedef ResPool::repository_iterator    repository_iterator;

  public:

    /** Default ctor: no pool
     * Nonempty proxies are provided by \ref ResPool.
     * \see \ref ResPool::proxy
     * \code
     * ResPoolProxy p( ResPool::instance().proxy() );
     * \endcode
     */
    ResPoolProxy();

    /** Dtor */
    ~ResPoolProxy();

  public:
    /** \name Lookup individual Selectables. */
    //@{
    ui::Selectable::Ptr lookup( const pool::ByIdent & ident_r ) const;

    ui::Selectable::Ptr lookup( IdString ident_r ) const
    { return lookup( pool::ByIdent( ident_r ) ); }

    ui::Selectable::Ptr lookup( ResKind kind_r, const std::string & name_r ) const
    { return lookup( pool::ByIdent( kind_r, name_r ) ); }

    ui::Selectable::Ptr lookup( const sat::Solvable & solv_r ) const
    { return lookup( pool::ByIdent( solv_r ) ); }

    ui::Selectable::Ptr lookup( const ResObject::constPtr & resolvable_r ) const
    { return resolvable_r ? lookup( resolvable_r->satSolvable() ) : ui::Selectable::Ptr(); }

    ui::Selectable::Ptr lookup( const PoolItem & pi_r ) const
    { return lookup( pi_r.satSolvable() ); }
    //@}

  public:
    /** \name Iterate through all Selectables of a all kind. */
    //@{
    bool empty() const;
    size_type size() const;
    const_iterator begin() const;
    const_iterator end() const;
    //@}

    /** \name Iterate through all Selectables of a certain kind. */
    //@{
    /** True if there are items of a certain kind. */
    bool empty( const ResKind & kind_r ) const;

    template<class TRes>
      bool empty() const
      { return empty( ResTraits<TRes>::kind ); }

    /** Number of Items of a certain kind.  */
    size_type size( const ResKind & kind_r ) const;

    template<class TRes>
      size_type size() const
      { return size( ResTraits<TRes>::kind ); }

    const_iterator byKindBegin( const ResKind & kind_r ) const;

    template<class TRes>
      const_iterator byKindBegin() const
      { return byKindBegin( ResTraits<TRes>::kind ); }


    const_iterator byKindEnd( const ResKind & kind_r ) const;

    template<class TRes>
      const_iterator byKindEnd() const
      { return byKindEnd( ResTraits<TRes>::kind ); }


    Iterable<const_iterator> byKind( const ResKind & kind_r ) const
      { return makeIterable( byKindBegin( kind_r ), byKindEnd( kind_r ) ); }

    template<class TRes>
      Iterable<const_iterator> byKind() const
      { return makeIterable( byKindBegin<TRes>(), byKindEnd<TRes>() ); }

    //@}

 public:
   /** \name Iterate through all Repositories that contribute ResObjects.
   */
   //@{
   size_type knownRepositoriesSize() const;

   repository_iterator knownRepositoriesBegin() const;

   repository_iterator knownRepositoriesEnd() const;

   Iterable<repository_iterator> knownRepositories() const
   { return makeIterable( knownRepositoriesBegin(), knownRepositoriesEnd() ); }
   //@}

  public:
    /** Test whether there is at least one ui::Selectable with
     * an installed object.
    */
    bool hasInstalledObj( const ResKind & kind_r ) const
    {
      return(    make_begin<ui::selfilter::ByHasInstalledObj>( kind_r )
              != make_end<ui::selfilter::ByHasInstalledObj>( kind_r ) );
    }

    template<class TRes>
      bool hasInstalledObj() const
      { return hasInstalledObj( ResTraits<TRes>::kind ); }

  public:
    /** \name Save and restore state per kind of resolvable.
     * Simple version, no safety net. So don't restore or diff,
     * if you didn't save before.
     *
     * Diff returns true, if current stat differs from the saved
     * state.
     *
     * Use \ref scopedSaveState for exception safe scoped save/restore
     */
    //@{
    void saveState() const;

    void saveState( const ResKind & kind_r ) const;

    template<class TRes>
      void saveState() const
      { return saveState( ResTraits<TRes>::kind ); }

    void restoreState() const;

    void restoreState( const ResKind & kind_r ) const;

    template<class TRes>
      void restoreState() const
      { return restoreState( ResTraits<TRes>::kind ); }

    bool diffState() const;

    bool diffState( const ResKind & kind_r ) const;

    template<class TRes>
      bool diffState() const
      { return diffState( ResTraits<TRes>::kind ); }

    /**
     * \class ScopedSaveState
     * \brief Exception safe scoped save/restore state.
     * Call \ref acceptState to prevent the class from restoring
     * the remembered state.
     * \ingroup g_RAII
     */
    struct ScopedSaveState;

    ScopedSaveState scopedSaveState() const;

    ScopedSaveState scopedSaveState( const ResKind & kind_r ) const;

    template<class TRes>
      ScopedSaveState && scopedSaveState() const
      { return scopedSaveState( ResTraits<TRes>::kind ); }

    //@}

  private:
    template<class TFilter>
      filter_iterator<TFilter,const_iterator>
      make_begin( TFilter filter_r, const ResKind & kind_r ) const
      {
        return make_filter_iterator( filter_r,
                                     byKindBegin(kind_r),
                                     byKindEnd(kind_r) );
      }
    template<class TFilter>
      filter_iterator<TFilter,const_iterator>
      make_begin( const ResKind & kind_r ) const
      {
        return make_begin( TFilter(), kind_r );
      }


    template<class TFilter>
      filter_iterator<TFilter,const_iterator>
      make_end( TFilter filter_r, const ResKind & kind_r ) const
      {
        return make_filter_iterator( filter_r,
                                     byKindEnd(kind_r),
                                     byKindEnd(kind_r) );
      }
    template<class TFilter>
      filter_iterator<TFilter,const_iterator>
      make_end( const ResKind & kind_r ) const
      {
        return make_end( TFilter(), kind_r );
      }

  private:
    friend class pool::PoolImpl;
    /** Ctor */
    ResPoolProxy( ResPool pool_r, const pool::PoolImpl & poolImpl_r );
    /** Pointer to implementation */
    RW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates ResPoolProxy Stream output */
  std::ostream & operator<<( std::ostream & str, const ResPoolProxy & obj );

  /** \relates ResPoolProxy Verbose stream output */
  std::ostream & dumpOn( std::ostream & str, const ResPoolProxy & obj );

  ///////////////////////////////////////////////////////////////////

  struct ResPoolProxy::ScopedSaveState
  {
    NON_COPYABLE_BUT_MOVE( ScopedSaveState );

    ScopedSaveState( const ResPoolProxy & pool_r )
    : _pimpl( new Impl( pool_r ) )
    { _pimpl->saveState(); }

    ScopedSaveState( const ResPoolProxy & pool_r, const ResKind & kind_r )
    : _pimpl( new Impl( pool_r, kind_r ) )
    { _pimpl->saveState(); }

    ~ScopedSaveState()
    { if ( _pimpl ) _pimpl->restoreState(); }

    void acceptState()
    { _pimpl.reset(); }

  private:
    struct Impl
    {
      Impl( const ResPoolProxy & pool_r )
      : _pool( pool_r )
      {}
      Impl( const ResPoolProxy & pool_r, const ResKind & kind_r )
      : _pool( pool_r ), _kind( new ResKind( kind_r ) )
      {}
      void saveState()
      { if ( _kind ) _pool.saveState( *_kind ); else _pool.saveState(); }
      void restoreState()
      { if ( _kind ) _pool.restoreState( *_kind ); else _pool.restoreState(); }
      ResPoolProxy _pool;
      scoped_ptr<ResKind> _kind;

    };
    std::unique_ptr<Impl> _pimpl;
  };

  inline ResPoolProxy::ScopedSaveState ResPoolProxy::scopedSaveState() const
  { return ScopedSaveState( *this ); }

  inline ResPoolProxy::ScopedSaveState ResPoolProxy::scopedSaveState( const ResKind & kind_r ) const
  { return ScopedSaveState( *this, kind_r ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESPOOLPROXY_H
