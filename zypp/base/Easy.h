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

/** Convenient for-loops using iterator.
 * \code
 *  std::set&lt;std::string&gt; _store;
 *  for_( it, _store.begin(), _store.end() )
 *  {
 *    cout << *it << endl;
 *  }
 * \endcode
*/
#define for_(IT,BEG,END) for ( typeof(BEG) IT = BEG, _for_end = END; IT != _for_end; ++IT )
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

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_EASY_H
