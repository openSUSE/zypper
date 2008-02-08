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

#include "zypp/base/SerialNumber.h"

#include "zypp/ResPool.h"
#include "zypp/pool/PoolImpl.h"
#include "zypp/pool/PoolStats.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResPool::instance
  //	METHOD TYPE : ResPool
  //
  ResPool ResPool::instance()
  {
    static ResPool _val( pool::PoolTraits::Impl_constPtr( new pool::PoolImpl ) );
    return _val;
  }

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
  // Forward to impementation:
  //
  ///////////////////////////////////////////////////////////////////

  ResPoolProxy ResPool::proxy() const
  { return _pimpl->proxy( *this ); }

  const SerialNumber & ResPool::serial() const
  { return _pimpl->serial(); }

  bool ResPool::empty() const
  { return _pimpl->empty(); }

  ResPool::size_type ResPool::size() const
  { return _pimpl->size(); }


  PoolItem ResPool::find( const sat::Solvable & slv_r ) const
  { return _pimpl->find( slv_r ); }

  ResPool::byCapabilityIndex_iterator ResPool::byCapabilityIndexBegin( const std::string & index_r, Dep depType_r ) const
  { return _pimpl->_caphashfake.begin(); }

  ResPool::byCapabilityIndex_iterator ResPool::byCapabilityIndexEnd( const std::string & index_r, Dep depType_r ) const
  { return _pimpl->_caphashfake.end(); }


  ResPool::size_type ResPool::knownRepositoriesSize() const
  { return _pimpl->knownRepositories().size(); }

  ResPool::repository_iterator ResPool::knownRepositoriesBegin() const
  { return _pimpl->knownRepositories().begin(); }

  ResPool::repository_iterator ResPool::knownRepositoriesEnd() const
  { return _pimpl->knownRepositories().end(); }


  void ResPool::setAdditionalRequire( const AdditionalCapabilities & capset ) const
  { _pimpl->setAdditionalRequire( capset ); }
  ResPool::AdditionalCapabilities & ResPool::additionalRequire() const
  { return _pimpl->additionalRequire(); }

  void ResPool::setAdditionalConflict( const AdditionalCapabilities & capset ) const
  { _pimpl->setAdditionalConflict( capset ); }
  ResPool::AdditionalCapabilities & ResPool::additionaConflict() const
  { return _pimpl->additionaConflict(); }

  void ResPool::setAdditionalProvide( const AdditionalCapabilities & capset ) const
  { _pimpl->setAdditionalProvide( capset ); }
  ResPool::AdditionalCapabilities & ResPool::additionaProvide() const
  { return _pimpl->additionaProvide(); }

  const pool::PoolTraits::ItemContainerT & ResPool::store() const
  { return _pimpl->store(); }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const ResPool & obj )
  {
    dumpPoolStats( str << "ResPool " << sat::Pool::instance() << endl << "  ",
                            obj.begin(), obj.end() );
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
