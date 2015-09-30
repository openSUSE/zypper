/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/Hash.h
 *
*/
#ifndef ZYPP_BASE_HASH_H
#define ZYPP_BASE_HASH_H

#include <iosfwd>
#include <unordered_set>
#include <unordered_map>

/** Define hash function for id based classes.
 * Class has to provide a method \c id() retuning a unique number.
 * \code
 *  // in global namespace define:
 *  ZYPP_DEFINE_ID_HASHABLE( ::zypp::sat::Solvable )
 * \endcode
 */
#define ZYPP_DEFINE_ID_HASHABLE(C)		\
namespace std {					\
  template<class Tp> struct hash;		\
  template<> struct hash<C>			\
  {						\
    size_t operator()( const C & __s ) const	\
    { return __s.id(); }			\
  };						\
}

///////////////////////////////////////////////////////////////////
namespace std
{
  /** clone function for RW_pointer */
  template<class D>
  inline unordered_set<D> * rwcowClone( const std::unordered_set<D> * rhs )
  { return new std::unordered_set<D>( *rhs ); }

  /** clone function for RW_pointer */
  template<class K, class V>
  inline std::unordered_map<K,V> * rwcowClone( const std::unordered_map<K,V> * rhs )
  { return new std::unordered_map<K,V>( *rhs ); }
} // namespace std
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_HASH_H
