/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ResPool.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/ResPool.h"
#include "zypp/pool/PoolImpl.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResPool::ResPool
  //	METHOD TYPE : Ctor
  //
  ResPool::ResPool( pool::PoolTraits::Impl_constPtr impl_r )
  : _pimpl( impl_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResPool::~ResPool
  //	METHOD TYPE : Dtor
  //
  ResPool::~ResPool()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  // Forward to impementation:
  //
  ///////////////////////////////////////////////////////////////////

  bool ResPool::empty() const
  { return _pimpl->empty(); }

  ResPool::size_type ResPool::size() const
  { return _pimpl->size(); }

  ResPool::const_iterator ResPool::begin() const
  { return _pimpl->begin(); }

  ResPool::const_iterator ResPool::end() const
  { return _pimpl->end(); }

  ResPool::const_indexiterator ResPool::providesbegin(const std::string & tag_r) const
  { return _pimpl->providesbegin(tag_r); }

  ResPool::const_indexiterator ResPool::providesend(const std::string & tag_r) const
  { return _pimpl->providesend(tag_r); }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const ResPool & obj )
  {
    return str << *obj._pimpl;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
