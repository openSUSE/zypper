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

    typedef std::set<ui::Selectable::Ptr>             SelectableIndex;
    typedef std::map<ResObject::Kind,SelectableIndex> SelectablePool;

  public:
    /** Implementation  */
    class Impl;

    typedef SelectableIndex::iterator       iterator;
    typedef SelectableIndex::const_iterator const_iterator;
    typedef SelectableIndex::size_type      size_type;

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
    ui::Selectable::Ptr lookup( IdString ident_r ) const
    { sat::Solvable::SplitIdent id( ident_r ); return lookup( id.kind(), id.name() ); }

    ui::Selectable::Ptr lookup( ResKind kind_r, const std::string & name_r ) const;

    ui::Selectable::Ptr lookup( const sat::Solvable & solv_r ) const
    { return lookup( solv_r.kind(), solv_r.name() ); }

    ui::Selectable::Ptr lookup( const ResObject::constPtr & resolvable_r ) const
    { return resolvable_r ? lookup( resolvable_r->satSolvable() ) : ui::Selectable::Ptr(); }

    ui::Selectable::Ptr lookup( const PoolItem & pi_r ) const
    { return lookup( pi_r.satSolvable() ); }
    //@}

  public:

    /** True if there are items of a certain kind. */
    bool empty( const ResObject::Kind & kind_r ) const;

    template<class _Res>
      bool empty() const
      { return empty( ResTraits<_Res>::kind ); }

    /** Number of Items of a certain kind.  */
    size_type size( const ResObject::Kind & kind_r ) const;

    template<class _Res>
      size_type size() const
      { return size( ResTraits<_Res>::kind ); }

    /** \name Iterate through all Selectables of a certain kind. */
    //@{
    const_iterator byKindBegin( const ResObject::Kind & kind_r ) const;

    template<class _Res>
      const_iterator byKindBegin() const
      { return byKindBegin( ResTraits<_Res>::kind ); }


    const_iterator byKindEnd( const ResObject::Kind & kind_r ) const;

    template<class _Res>
      const_iterator byKindEnd() const
      { return byKindEnd( ResTraits<_Res>::kind ); }
    //@}

 public:
   /** \name Iterate through all Repositories that contribute ResObjects.
   */
   //@{
   size_type knownRepositoriesSize() const;

   repository_iterator knownRepositoriesBegin() const;

   repository_iterator knownRepositoriesEnd() const;
   //@}

  public:
    /** Test whether there is at least one ui::Selectable with
     * an installed object.
    */
    bool hasInstalledObj( const ResObject::Kind & kind_r ) const
    {
      return(    make_begin<ui::selfilter::ByHasInstalledObj>( kind_r )
              != make_end<ui::selfilter::ByHasInstalledObj>( kind_r ) );
    }

    template<class _Res>
      bool hasInstalledObj() const
      { return hasInstalledObj( ResTraits<_Res>::kind ); }

  public:
    /** \name Save and restore state per kind of resolvable.
     * Simple version, no savety net. So don't restore or diff,
     * if you didn't save before.
     *
     * Diff returns true, if current stat differs from the saved
     * state.
    */
    //@{
    void saveState() const;

    void saveState( const ResObject::Kind & kind_r ) const;

    template<class _Res>
      void saveState() const
      { return saveState( ResTraits<_Res>::kind ); }

    void restoreState() const;

    void restoreState( const ResObject::Kind & kind_r ) const;

    template<class _Res>
      void restoreState() const
      { return restoreState( ResTraits<_Res>::kind ); }

    bool diffState() const;

    bool diffState( const ResObject::Kind & kind_r ) const;

    template<class _Res>
      bool diffState() const
      { return diffState( ResTraits<_Res>::kind ); }
    //@}

  private:
    template<class _Filter>
      filter_iterator<_Filter,const_iterator>
      make_begin( _Filter filter_r, const ResObject::Kind & kind_r ) const
      {
        return make_filter_iterator( filter_r,
                                     byKindBegin(kind_r),
                                     byKindEnd(kind_r) );
      }
    template<class _Filter>
      filter_iterator<_Filter,const_iterator>
      make_begin( const ResObject::Kind & kind_r ) const
      {
        return make_begin( _Filter(), kind_r );
      }


    template<class _Filter>
      filter_iterator<_Filter,const_iterator>
      make_end( _Filter filter_r, const ResObject::Kind & kind_r ) const
      {
        return make_filter_iterator( filter_r,
                                     byKindEnd(kind_r),
                                     byKindEnd(kind_r) );
      }
    template<class _Filter>
      filter_iterator<_Filter,const_iterator>
      make_end( const ResObject::Kind & kind_r ) const
      {
        return make_end( _Filter(), kind_r );
      }

  private:
    friend class pool::PoolImpl;
    /** Ctor */
    ResPoolProxy( ResPool pool_r );
    /** Pointer to implementation */
    RW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates ResPoolProxy Stream output */
  std::ostream & operator<<( std::ostream & str, const ResPoolProxy & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESPOOLPROXY_H
