/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ResPoolManager.h
 *
*/
#ifndef ZYPP_RESPOOLMANAGER_H
#define ZYPP_RESPOOLMANAGER_H

#include <iosfwd>

#include <zypp/ResPool.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ResPoolManager
  //
  /** Manage a ResObject pool. */
  class ResPoolManager
  {
    friend std::ostream & operator<<( std::ostream & str, const ResPoolManager & obj );

  public:
    /** */
    typedef pool::PoolTraits::Item           Item;
    typedef pool::PoolTraits::size_type      size_type;
    typedef pool::PoolTraits::iterator       iterator;
    typedef pool::PoolTraits::const_iterator const_iterator;

  public:
    /** Default ctor */
    ResPoolManager();
    /** Dtor */
    ~ResPoolManager();

    ResPool accessor() const
    { return ResPool( _pimpl.getPtr() ); }

  public:
    /**  */
    void insert( ResObject::constPtr ptr_r, bool installed = false )
    { inserter()( ptr_r, installed ); }

    /**  */
    template <class _InputIterator>
      void insert( _InputIterator first_r, _InputIterator last_r, bool installed = false )
      { while (first_r != last_r) { inserter()( *first_r, installed ); ++first_r; } }

    /**  */
    void erase( ResObject::constPtr ptr_r )
    { deleter()( ptr_r ); }

    /**  */
    void erase( iterator first_r, iterator last_r )
    { std::for_each( first_r, last_r, deleter() ); }

    /**  */
    void clear();

  private:
    /**  */
    typedef pool::PoolTraits::ContainerT  ContainerT;
    typedef pool::PoolTraits::Impl        Impl;
    typedef pool::PoolTraits::Inserter    Inserter;
    typedef pool::PoolTraits::Deleter     Deleter;

  private:
    /** Pointer to implementation */
    RW_pointer<pool::PoolTraits::Impl> _pimpl;
    /**  */
    Inserter inserter()
    { return Inserter( *_pimpl ); }
    /**  */
    Deleter deleter()
    { return Deleter( *_pimpl ); }
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates ResPoolManager Stream output */
  std::ostream & operator<<( std::ostream & str, const ResPoolManager & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESPOOLMANAGER_H
