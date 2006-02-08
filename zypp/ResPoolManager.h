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
#include <zypp/ResPoolProxy.h>

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

    ResPoolProxy proxy() const
    { return ResPoolProxy( accessor() ); }

  public:
    /**  */
    void insert( ResObject::constPtr ptr_r, bool installed = false )
    { inserter(installed)( ptr_r ); }

    /**  */
    template <class _InputIterator>
      void insert( _InputIterator first_r, _InputIterator last_r, bool installed = false )
      { std::for_each( first_r, last_r, inserter(installed) ); }

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
    Inserter inserter( bool installed )
    { return Inserter( *_pimpl, installed ); }
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
