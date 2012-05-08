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

#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100  + __GNUC_PATCHLEVEL__)
#if GCC_VERSION < 40600 || not defined(__GXX_EXPERIMENTAL_CXX0X__)
#define nullptr NULL
#endif

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_EASY_H
