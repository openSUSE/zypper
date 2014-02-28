/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/Easy.h
 *
*/
#ifndef ZYPP_BASE_EASY_H
#define ZYPP_BASE_EASY_H

#include <cstdio>

/** Convenient for-loops using iterator.
 * \code
 *  std::set<std::string>; _store;
 *  for_( it, _store.begin(), _store.end() )
 *  {
 *    cout << *it << endl;
 *  }
 * \endcode
*/
#ifndef __GXX_EXPERIMENTAL_CXX0X__
#define for_(IT,BEG,END) for ( typeof(BEG) IT = BEG, _for_end = END; IT != _for_end; ++IT )
#else
#define for_(IT,BEG,END) for ( auto IT = BEG, _for_end = END; IT != _for_end; ++IT )
#endif
#define for_each_(IT,CONT) for_( IT, CONT.begin(), CONT.end() )

/** Simple C-array iterator
 * \code
 *  const char * defstrings[] = { "",  "a", "default", "two words" };
 *  for_( it, arrayBegin(defstrings), arrayEnd(defstrings) )
 *    cout << *it << endl;
 * \endcode
*/
#define arrayBegin(A) (&A[0])
#define arraySize(A)  (sizeof(A)/sizeof(*A))
#define arrayEnd(A)   (&A[0] + arraySize(A))

/**
 * \code
 * defConstStr( strANY(), "ANY" );
 * std::str str = strANY();
 * \endcode
 */
#define defConstStr(FNC,STR) inline const std::string & FNC { static const std::string val( STR ); return val; }

#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100  + __GNUC_PATCHLEVEL__)
#if GCC_VERSION < 40600 || not defined(__GXX_EXPERIMENTAL_CXX0X__)
#define nullptr NULL
#endif

/** Delete copy ctor and copy assign */
#define NON_COPYABLE(CLASS)			\
  CLASS( const CLASS & ) = delete;		\
  CLASS & operator=( const CLASS & ) = delete

/** Default copy ctor and copy assign */
#define DEFAULT_COPYABLE(CLASS)			\
  CLASS( const CLASS & ) = default;		\
  CLASS & operator=( const CLASS & ) = default

/** Delete move ctor and move assign */
#define NON_MOVABLE(CLASS)			\
  CLASS( CLASS && ) = delete;			\
  CLASS & operator=( CLASS && ) = delete

/** Default move ctor and move assign */
#define DEFAULT_MOVABLE(CLASS)			\
  CLASS( CLASS && ) = default;			\
  CLASS & operator=( CLASS && ) = default

/** Delete copy ctor and copy assign but enable default move */
#define NON_COPYABLE_BUT_MOVE( CLASS ) 		\
  NON_COPYABLE(CLASS);				\
  DEFAULT_MOVABLE(CLASS)

/** Default move ctor and move assign but enable default copy */
#define NON_MOVABLE_BUT_COPY( CLASS ) 		\
  NON_MOVABLE(CLASS);				\
  DEFAULT_COPYABLE(CLASS)

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_EASY_H
