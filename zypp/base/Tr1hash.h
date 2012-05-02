/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/Tr1hash.h
 *
*/
#ifndef ZYPP_BASE_TR1HASH_H
#define ZYPP_BASE_TR1HASH_H

#include <iosfwd>
#include <tr1/unordered_set>
#include <tr1/unordered_map>

/** Define hash function for id based classes.
 * Class has to provide a method \c id() retuning a unique number.
 * \code
 *  // in global namespace define:
 *  ZYPP_DEFINE_ID_HASHABLE( ::zypp::sat::Sovable )
 * \endcode
 */
#define ZYPP_DEFINE_ID_HASHABLE(C)           \
namespace std { namespace tr1 {              \
  template<class _Tp> struct hash;           \
  template<> struct hash<C>                  \
  {                                          \
    size_t operator()( const C & __s ) const \
    { return __s.id(); }                     \
  };                                         \
}}

///////////////////////////////////////////////////////////////////
namespace std
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace tr1
  { /////////////////////////////////////////////////////////////////

    /** clone function for RW_pointer */
    template<class _D>
    inline unordered_set<_D> * rwcowClone( const std::tr1::unordered_set<_D> * rhs )
    { return new std::tr1::unordered_set<_D>( *rhs ); }

    /** clone function for RW_pointer */
    template<class _K, class _V>
    inline std::tr1::unordered_map<_K,_V> * rwcowClone( const std::tr1::unordered_map<_K,_V> * rhs )
    { return new std::tr1::unordered_map<_K,_V>( *rhs ); }

    /////////////////////////////////////////////////////////////////
  } // namespace tr1
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace std
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_TR1HASH_H
