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
  /**
   * \todo Make it a _Ref.
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

  public:
    /** Default ctor: no pool */
    ResPoolProxy();
    /** Ctor */
    ResPoolProxy( ResPool_Ref pool_r );
    /** Dtor */
    ~ResPoolProxy();

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
     * Simple version, no savety net.
    */
    //@{
    void saveState( const ResObject::Kind & kind_r ) const;

    template<class _Res>
      void saveState() const
      { return saveState( ResTraits<_Res>::kind ); }

    void restoreState( const ResObject::Kind & kind_r ) const;

    template<class _Res>
      void restoreState() const
      { return restoreState( ResTraits<_Res>::kind ); }
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
