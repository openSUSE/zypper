/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ResStore.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/ResStore.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ResStore::Impl
  //
  /** ResStore implementation. */
  struct ResStore::Impl
  {
    Impl()
    {}
#if 0
    iterator begin()
    { return _store.begin(); }

    iterator end();
    { return _store.end(); }

    const_iterator begin() const;
    { return _store.begin(); }

    const_iterator end() const;
    { return _store.end(); }

    StorageT _store;
#endif
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates ResStore::Impl Stream output */
  inline std::ostream & operator<<( std::ostream & str, const ResStore::Impl & obj )
  {
    return str << "ResStore::Impl";
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ResStore
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResStore::ResStore
  //	METHOD TYPE : Ctor
  //
  ResStore::ResStore()
  //: _pimpl( new Impl )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResStore::~ResStore
  //	METHOD TYPE : Dtor
  //
  ResStore::~ResStore()
  {}
#if 0
  ResStore::iterator ResStore::begin()
  { return _pimpl->begin(); }

  ResStore::iterator ResStore::end();
  { return _pimpl->end(); }

  ResStore::const_iterator ResStore::begin() const;
  { return _pimpl->begin(); }

  ResStore::const_iterator ResStore::end() const;
  { return _pimpl->end(); }
#endif
  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const ResStore & obj )
  {
    return str << "ResStore: " << obj.size();
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
