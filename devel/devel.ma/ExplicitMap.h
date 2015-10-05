/*---------------------------------------------------------------------\
 |                          ____ _   __ __ ___                          |
 |                         |__  / \ / / . \ . \                         |
 |                           / / \ V /|  _/  _/                         |
 |                          / /__ | | | | | |                           |
 |                         /_____||_| |_| |_|                           |
 |                                                                      |
 \---------------------------------------------------------------------*/
/** \file zypp/ExplicitMap.h
 *
*/
#ifndef ZYPP_EXPLICITMAP_H
#define ZYPP_EXPLICITMAP_H

#include <iosfwd>
#include <map>

#include <boost/call_traits.hpp>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //    CLASS NAME : ExplicitMap<TKey, Tp>
  //
  /** A simple lookup map using default value for not existing entries.
   *
   * A std::map providing <tt>operator[] const</tt> only. Nor existing
   * entries are mapped to a default value. Entries are maipulated vis
   * methods \ref set and \ref unset. Helper classes \ref TmpSet,
   * \ref TmpUnset and \ref TmpSetDefault are provided to temporarily
   * change and automaticlly restore values.
   */
  template<class TKey, class Tp>
    class ExplicitMap
    {
    public:
      typedef typename boost::call_traits<Tp>::value_type       value_type;
      typedef typename boost::call_traits<Tp>::reference        reference;
      typedef typename boost::call_traits<Tp>::const_reference  const_reference;
      typedef typename boost::call_traits<Tp>::param_type       param_type;

    private:
      typedef typename std::map<TKey,value_type> map_type;
      typedef typename map_type::iterator        iterator;

    public:
      typedef typename map_type::key_type       key_type;
      typedef typename map_type::size_type      size_type;
      typedef typename map_type::const_iterator const_iterator;

    public:
      ExplicitMap()
      {}

      explicit
      ExplicitMap( param_type mapDefault_r )
      : _mapDefault( mapDefault_r )
      {}

      template <class TInputIterator>
        ExplicitMap( TInputIterator first_r, TInputIterator last_r )
        : _map( first_r, last_r )
        {}

      template <class TInputIterator>
        ExplicitMap( TInputIterator first_r, TInputIterator last_r,
                     param_type mapDefault_r )
        : _map( first_r, last_r )
        , _mapDefault( mapDefault_r )
        {}

    public:
      size_type size() const
      { return _map.size(); }

      bool empty() const
      { return _map.empty(); }

      const_iterator begin() const
      { return _map.begin(); }

      const_iterator end() const
      { return _map.end(); }

      const_iterator find( const key_type & key_r ) const
      { return _map.find( key_r ); }

      bool has( const key_type & key_r ) const
      { return _map.find( key_r ) != end(); }

      const_reference get( const key_type & key_r ) const
      {
        const_iterator it = _map.find( key_r );
        return( it == _map.end() ? _mapDefault : it->second );
      }

      const_reference getDefault() const
      { return _mapDefault; }

      const_reference operator[]( const key_type & key_r ) const
      { return get( key_r ); }

    public:
      void clear()
      { _map.clear(); }

      void set( const key_type & key_r, param_type value_r )
      { _map[key_r] = value_r; }

      template <typename TInputIterator>
        void set( TInputIterator first_r, TInputIterator last_r )
        { _map.insert( first_r, last_r ); }

      void unset( const key_type & key_r )
      { _map.erase( key_r ); }

      void setDefault( param_type value_r )
      { _mapDefault = value_r; }

    public:
      class TmpSet;
      class TmpUnset;
      class TmpSetDefault;

      //private:
      value_type _mapDefault;
      map_type   _map;
    };
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //    CLASS NAME : ExplicitMap<TKey, Tp>::TmpSet
  //
  /** Temporarily set a value. */
  template<class TKey, class Tp>
    class ExplicitMap<TKey, Tp>::TmpSet
    {
    public:
      TmpSet( ExplicitMap & map_r, const key_type & key_r, param_type value_r )
      : _map( map_r )
      , _key( key_r )
      {
        const_iterator it = _map.find( _key );
        if ( it == _map.end() )
          {
            _wasDefault = true;
          }
        else
          {
            _wasDefault = false;
            _value = it->second;
          }
        _map.set( _key, value_r );
      }

      ~TmpSet()
      {
        if ( _wasDefault )
          {
            _map.unset( _key );
          }
        else
          {
            _map.set( _key, _value );
          }
      }

    private:
      ExplicitMap & _map;
      key_type      _key;
      param_type    _value;
      bool          _wasDefault;
    };
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //    CLASS NAME : ExplicitMap<TKey, Tp>::TmpUnset
  //
  /** Temporarily unset a value. */
  template<class TKey, class Tp>
    class ExplicitMap<TKey, Tp>::TmpUnset
    {
    public:
      TmpUnset( ExplicitMap & map_r, const key_type & key_r )
      : _map( map_r )
      , _key( key_r )
      {
        const_iterator it = _map.find( _key );
        if ( it == _map.end() )
          {
            _wasDefault = true;
          }
        else
          {
            _wasDefault = false;
            _value = it->second;
            _map.unset( _key );
          }
      }

      ~TmpUnset()
      {
        if ( ! _wasDefault )
          {
            _map.set( _key, _value );
          }
      }

    private:
      ExplicitMap & _map;
      key_type      _key;
      param_type    _value;
      bool          _wasDefault;
    };
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //    CLASS NAME : ExplicitMap<TKey, Tp>::TmpSetDefault
  //
  /** Temporarily change the default value. */
  template<class TKey, class Tp>
    class ExplicitMap<TKey, Tp>::TmpSetDefault
    {
    public:
      TmpSetDefault( ExplicitMap & map_r, param_type value_r )
      : _map( map_r )
      , _value( _map.getDefault() )
      {
        _map.setDefault( value_r );
      }

      ~TmpSetDefault()
      {
        _map.setDefault( _value );
      }

    private:
      ExplicitMap & _map;
      param_type    _value;
    };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_EXPLICITMAP_H
