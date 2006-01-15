/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ResStore.h
 *
*/
#ifndef ZYPP_RESSTORE_H
#define ZYPP_RESSTORE_H

#include <iosfwd>
#include <set>

#include "zypp/base/PtrTypes.h"
#include "zypp/Package.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ResStore
  //
  /**
   *
   *
  */
  class ResStore
  {
    friend std::ostream & operator<<( std::ostream & str, const ResStore & obj );

    typedef std::set<ResObject::Ptr> StorageT;

  public:
    /** Implementation  */
    class Impl;

    typedef StorageT::size_type      size_type;
    typedef StorageT::iterator       iterator;
    typedef StorageT::const_iterator const_iterator;

  public:
    /** Default ctor */
    ResStore();
    /** Dtor */
    ~ResStore();

  public:
    /**  */
    iterator begin()
    { return store().begin(); }
    /**  */
    iterator end()
    { return store().end(); }
    /**  */
    const_iterator begin() const
    { return store().begin(); }
    /**  */
    const_iterator end() const
    { return store().end(); }

    /**  */
    bool empty() const
    { return store().empty(); }
    /**  */
    size_type size() const
    { return store().size(); }

    // insert/erase
    /**  */
    iterator insert( const ResObject::Ptr & ptr_r )
    { return store().insert( ptr_r ).first; }
    /**  */
    template <class _InputIterator>
      void insert( _InputIterator first_r, _InputIterator last_r )
      { store().insert( first_r, last_r ); }
    /**  */
    size_type erase( const ResObject::Ptr & ptr_r )
    { return store().erase( ptr_r ); }
    /**  */
    void erase( iterator first_r, iterator last_r )
    { store().erase( first_r, last_r ); }
    /**  */
    void clear()
    { store().clear(); }

    // query
    template <class _Function>
      _Function forEach( _Function fnc_r )
      { return std::for_each( store().begin(), store().end(), fnc_r ); }

    template <class _Function>
      _Function forEach( _Function fnc_r ) const
      { return std::for_each( store().begin(), store().end(), fnc_r ); }

  private:
    /**  */
    StorageT _store;
    /**  */
    StorageT & store()
    { return _store; }
    /**  */
    const StorageT & store() const
    { return _store; }

  private:
    /** Pointer to implementation */
    RW_pointer<Impl> _pimpl; // currently unsused
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates ResStore Stream output */
  std::ostream & operator<<( std::ostream & str, const ResStore & obj );

  ///////////////////////////////////////////////////////////////////
  //
  // Predefined filters
  //
  ///////////////////////////////////////////////////////////////////

  /** Filter by kind. */
  template <class _Function>
    struct ByKindFilter
    {
      ByKindFilter( const ResObject::Kind & kind_r, const _Function & fnc_r )
      : _kind( kind_r )
      , _fnc( fnc_r )
      {}

      void operator()( ResObject::Ptr p ) const
      {
        if ( p->kind() == _kind )
          _fnc( p );
      }

      ResObject::Kind _kind;
      const _Function & _fnc;
    };

  /** Convenience creating appropriate ByKindFilter. */
  template <class _Function>
    ByKindFilter<_Function> byKind( const ResObject::Kind & kind_r, const _Function & fnc_r )
    { return ByKindFilter<_Function>( kind_r, fnc_r ); }

  /** Convenience creating appropriate ByKindFilter. */
  template <class _Res, class _Function>
    ByKindFilter<_Function> byKind( const _Function & fnc_r )
    { return ByKindFilter<_Function>( ResTraits<_Res>::kind, fnc_r ); }

  ///////////////////////////////////////////////////////////////////

  /** Filter by name. */
  template <class _Function>
    struct ByNameFilter
    {
      ByNameFilter( const std::string & name_r, const _Function & fnc_r )
      : _name( name_r )
      , _fnc( fnc_r )
      {}

      void operator()( ResObject::Ptr p ) const
      {
        if ( p->name() == _name )
          _fnc( p );
      }

      std::string _name;
      const _Function & _fnc;
    };

  /** Convenience creating appropriate ByNameFilter. */
  template <class _Function>
    ByNameFilter<_Function> byName( const std::string & name_r, const _Function & fnc_r )
    { return ByNameFilter<_Function>( name_r, fnc_r ); }

  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESSTORE_H
