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
    void insert( ResObject::constPtr ptr_r )
    { _inserter( ptr_r ); }

    /**  */
    template <class _InputIterator>
      void insert( _InputIterator first_r, _InputIterator last_r )
      { std::for_each( first_r, last_r, _inserter ); }

    /**  */
    void erase( ResObject::constPtr ptr_r )
    { _deleter( ptr_r ); }

    /**  */
    void erase( iterator first_r, iterator last_r )
    { std::for_each( first_r, last_r, _deleter ); }

    /**  */
    void clear();

  private:
    /**  */
    typedef pool::PoolTraits::ContainerT  ContainerT;
    typedef pool::PoolTraits::Impl        Impl;
    /**  */
    struct Inserter
    {
      void operator()( ResObject::constPtr ptr_r );

      Inserter( ContainerT & store_r )
      : _store( store_r )
      {}
      ContainerT & _store;
    };
    /**  */
    struct Deleter
    {
      void operator()( ResObject::constPtr ptr_r );

      Deleter( ContainerT & store_r )
      : _store( store_r )
      {}
      ContainerT & _store;
    };

  private:
    /** Pointer to implementation */
    RW_pointer<pool::PoolTraits::Impl> _pimpl;
    /**  */
    Inserter _inserter;
    /**  */
    Deleter _deleter;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates ResPoolManager Stream output */
  std::ostream & operator<<( std::ostream & str, const ResPoolManager & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESPOOLMANAGER_H
