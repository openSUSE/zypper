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
#include "zypp/base/SerialNumber.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  namespace
  {
    /** the empty pool used by ResPool::ResPool() */
    pool::PoolTraits::Impl_constPtr noPool()
    {
      static pool::PoolTraits::Impl_constPtr _noPool( new pool::PoolImpl );
      return _noPool;
    }
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResPool::ResPool
  //	METHOD TYPE : Ctor
  //
  ResPool::ResPool()
  : _pimpl( noPool() )
  {}

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

  ResPool::byName_iterator ResPool::byNameBegin( const std::string & name_r ) const
  { return _pimpl->_namehash.begin( name_r ); }

  ResPool::byName_iterator ResPool::byNameEnd( const std::string & name_r ) const
  { return _pimpl->_namehash.end( name_r ); }

  ResPool::byCapabilityIndex_iterator ResPool::byCapabilityIndexBegin( const std::string & index_r, Dep depType_r ) const
  { return _pimpl->_caphash.begin( index_r, depType_r ); }

  ResPool::byCapabilityIndex_iterator ResPool::byCapabilityIndexEnd( const std::string & index_r, Dep depType_r ) const
  { return _pimpl->_caphash.end( index_r, depType_r ); }

  ResPool::size_type ResPool::knownRepositoriesSize() const
  { return _pimpl->knownRepositories().size(); }

  ResPool::repository_iterator ResPool::knownRepositoriesBegin() const
  { return _pimpl->knownRepositories().begin(); }

  ResPool::repository_iterator ResPool::knownRepositoriesEnd() const
  { return _pimpl->knownRepositories().end(); }

  void ResPool::setAdditionalRequire( const AdditionalCapSet & capset ) const
  { _pimpl->setAdditionalRequire( capset ); }
  ResPool::AdditionalCapSet & ResPool::additionalRequire() const
  { return _pimpl->additionalRequire(); }

  void ResPool::setAdditionalConflict( const AdditionalCapSet & capset ) const
  { _pimpl->setAdditionalConflict( capset ); }
  ResPool::AdditionalCapSet & ResPool::additionaConflict() const
  { return _pimpl->additionaConflict(); }

  void ResPool::setAdditionalProvide( const AdditionalCapSet & capset ) const
  { _pimpl->setAdditionalProvide( capset ); }
  ResPool::AdditionalCapSet & ResPool::additionaProvide() const
  { return _pimpl->additionaProvide(); }

  const SerialNumber & ResPool::serial() const
  { return _pimpl->serial(); }

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
