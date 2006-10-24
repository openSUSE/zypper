/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/LogTools.h
 *
*/
#ifndef ZYPP_BASE_LOGTOOLS_H
#define ZYPP_BASE_LOGTOOLS_H

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>
#include "zypp/base/Logger.h"
#include "zypp/base/Iterator.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /** Print range defined by iterators.
   * \code
   * intro [ pfx ITEM [ { sep ITEM }+ ] sfx ] extro
   * \endcode
   * The defaults print the range enclosed in \c {}, one item per
   * line indented by 2 spaces.
   * \code
   * {
   *   item1
   *   item2
   * }
   * {} // on empty rande
   * \endcode
   * A comma separated list enclosed in \c () would be
   * \code
   * dumpRange( stream, begin, end, "(", "", ", ", "", ")" );
   * \endcode
  */
  template<class _Iterator>
    std::ostream & dumpRange( std::ostream & str,
                              _Iterator begin, _Iterator end,
                              const std::string & intro = "{",
                              const std::string & pfx   = "\n  ",
                              const std::string & sep   = "\n  ",
                              const std::string & sfx   = "\n",
                              const std::string & extro = "}" )
    {
      str << intro;
      if ( begin != end )
        {
          str << pfx << *begin;
          for (  ++begin; begin != end; ++begin )
            str << sep << *begin;
          str << sfx;
        }
      return str << extro;
    }

  template<class _Iterator>
    std::ostream & dumpRangeLine( std::ostream & str,
                                  _Iterator begin, _Iterator end )
    { return dumpRange( str, begin, end, "(", "", ", ", "", ")" ); }

  template<class _Tp>
    std::ostream & operator<<( std::ostream & str, const std::vector<_Tp> & obj )
    { return dumpRange( str, obj.begin(), obj.end() ); }

  template<class _Tp>
    std::ostream & operator<<( std::ostream & str, const std::set<_Tp> & obj )
    { return dumpRange( str, obj.begin(), obj.end() ); }

  template<class _Tp>
    std::ostream & operator<<( std::ostream & str, const std::list<_Tp> & obj )
    { return dumpRange( str, obj.begin(), obj.end() ); }

  ///////////////////////////////////////////////////////////////////
  namespace _logtoolsdetail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    // mapEntry
    ///////////////////////////////////////////////////////////////////

    /** std::pair wrapper for std::map output.
     * Just because we want a special output format for std::pair
     * used in a std::map.
    */
    template<class _Pair>
      class MapEntry
      {
      public:
        MapEntry( const _Pair & pair_r )
        : _pair( &pair_r )
        {}

        const _Pair & pair() const
        { return *_pair; }

      private:
        const _Pair *const _pair;
      };

    /** \relates MapEntry Stream output. */
    template<class _Pair>
      std::ostream & operator<<( std::ostream & str, const MapEntry<_Pair> & obj )
      {
        return str << '[' << obj.pair().first << "] = " << obj.pair().second;
      }

    /** \relates MapEntry Convenience function to create MapEntry from std::pair. */
    template<class _Pair>
      MapEntry<_Pair> mapEntry( const _Pair & pair_r )
      { return MapEntry<_Pair>( pair_r ); }

    ///////////////////////////////////////////////////////////////////
    // dumpMap
    ///////////////////////////////////////////////////////////////////

    /** std::map wrapper for stream output.
     * Provides the transform_iterator used to write std::pair formated as
     * MapEntry.
     */
    template<class _Map>
      class DumpMap
      {
      private:
        typedef _Map                        MapType;
        typedef typename _Map::value_type   PairType;
        typedef MapEntry<PairType>          MapEntryType;

        struct Transformer : public std::unary_function<PairType, MapEntryType>
        {
          MapEntryType operator()( const PairType & pair_r ) const
          { return mapEntry( pair_r ); }
        };

        typedef transform_iterator<Transformer, typename MapType::const_iterator>
                MapEntry_const_iterator;

      public:
        DumpMap( const _Map & map_r )
        : _map( &map_r )
        {}

        const _Map & map() const
        { return *_map; }

        MapEntry_const_iterator map_begin() const
        { return make_transform_iterator( map().begin(), Transformer() ); }

        MapEntry_const_iterator map_end() const
        { return make_transform_iterator( map().end(), Transformer() );}

      private:
        const _Map *const _map;
      };

    /** \relates DumpMap Stream output. */
    template<class _Map>
      std::ostream & operator<<( std::ostream & str, const DumpMap<_Map> & obj )
      { return dumpRange( str, obj.map_begin(), obj.map_end() ); }

    /** \relates DumpMap Convenience function to create DumpMap from std::map. */
    template<class _Map>
      DumpMap<_Map> dumpMap( const _Map & map_r )
      { return DumpMap<_Map>( map_r ); }

    ///////////////////////////////////////////////////////////////////
    // dumpKeys
    ///////////////////////////////////////////////////////////////////

    /** std::map wrapper for stream output of keys.
     * Uses MapKVIterator iterate and write the key values.
     * \code
     * std::map<...> mymap;
     * std::cout << dumpKeys(mymap) << std::endl;
     * \endcode
     */
    template<class _Map>
      class DumpKeys
      {
      public:
        DumpKeys( const _Map & map_r )
        : _map( &map_r )
        {}

        const _Map & map() const
        { return *_map; }

      private:
        const _Map *const _map;
      };

    /** \relates DumpKeys Stream output. */
    template<class _Map>
      std::ostream & operator<<( std::ostream & str, const DumpKeys<_Map> & obj )
      { return dumpRange( str, make_map_key_begin(obj.map()), make_map_key_end(obj.map()) ); }

    /** \relates DumpKeys Convenience function to create DumpKeys from std::map. */
    template<class _Map>
      DumpKeys<_Map> dumpKeys( const _Map & map_r )
      { return DumpKeys<_Map>( map_r ); }

    ///////////////////////////////////////////////////////////////////
    // dumpValues
    ///////////////////////////////////////////////////////////////////

    /** std::map wrapper for stream output of values.
     * Uses MapKVIterator iterate and write the values.
     * \code
     * std::map<...> mymap;
     * std::cout << dumpValues(mymap) << std::endl;
     * \endcode
     */
    template<class _Map>
      class DumpValues
      {
      public:
        DumpValues( const _Map & map_r )
        : _map( &map_r )
        {}

        const _Map & map() const
        { return *_map; }

      private:
        const _Map *const _map;
      };

    /** \relates DumpValues Stream output. */
    template<class _Map>
      std::ostream & operator<<( std::ostream & str, const DumpValues<_Map> & obj )
      { return dumpRange( str, make_map_value_begin(obj.map()), make_map_value_end(obj.map()) ); }

    /** \relates DumpValues Convenience function to create DumpValues from std::map. */
    template<class _Map>
      DumpValues<_Map> dumpValues( const _Map & map_r )
      { return DumpValues<_Map>( map_r ); }

    /////////////////////////////////////////////////////////////////
  } // namespace _logtoolsdetail
  ///////////////////////////////////////////////////////////////////

  // iomanipulator
  using _logtoolsdetail::mapEntry;   // std::pair as '[key] = value'
  using _logtoolsdetail::dumpMap;    // dumpRange '[key] = value'
  using _logtoolsdetail::dumpKeys;   // dumpRange keys
  using _logtoolsdetail::dumpValues; // dumpRange values

  template<class _Key, class _Tp>
    std::ostream & operator<<( std::ostream & str, const std::map<_Key, _Tp> & obj )
    { return str << dumpMap( obj ); }

  /** Print stream status bits.
   * Prints the values of a streams \c good, \c eof, \c failed and \c bad bit.
   *
   * \code
   *  [g___] - good
   *  [_eF_] - eof and fail bit set
   *  [__FB] - fail and bad bit set
   * \endcode
  */
  inline std::ostream & operator<<( std::ostream & str, const std::basic_ios<char> & obj )
  {
    std::string ret( "[" );
    ret += ( obj.good() ? 'g' : '_' );
    ret += ( obj.eof()  ? 'e' : '_' );
    ret += ( obj.fail() ? 'F' : '_' );
    ret += ( obj.bad()  ? 'B' : '_' );
    ret += "]";
    return str << ret;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_LOGTOOLS_H
