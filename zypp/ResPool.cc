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

#include "zypp/ZYppFactory.h"
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
    static ResPool _val( pool::PoolTraits::Impl_Ptr( new pool::PoolImpl ) );
    return _val;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResPool::ResPool
  //	METHOD TYPE : Ctor
  //
  ResPool::ResPool( pool::PoolTraits::Impl_Ptr impl_r )
  : _pimpl( impl_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  // Forward to impementation:
  //
  ///////////////////////////////////////////////////////////////////

  ResPoolProxy ResPool::proxy() const
  { return _pimpl->proxy( *this ); }

  Resolver & ResPool::resolver() const
  { return *getZYpp()->resolver(); }

  const SerialNumber & ResPool::serial() const
  { return _pimpl->serial(); }

  bool ResPool::empty() const
  { return _pimpl->empty(); }

  ResPool::size_type ResPool::size() const
  { return _pimpl->size(); }


  PoolItem ResPool::find( const sat::Solvable & slv_r ) const
  { return _pimpl->find( slv_r ); }


  ResPool::size_type ResPool::knownRepositoriesSize() const
  { return _pimpl->knownRepositoriesSize(); }

  ResPool::repository_iterator ResPool::knownRepositoriesBegin() const
  { return _pimpl->knownRepositoriesBegin(); }

  ResPool::repository_iterator ResPool::knownRepositoriesEnd() const
  { return _pimpl->knownRepositoriesEnd(); }

  Repository ResPool::reposFind( const std::string & alias_r ) const
  { return _pimpl->reposFind( alias_r ); }

  bool ResPool::hardLockQueriesEmpty() const
  { return _pimpl->hardLockQueries().empty(); }

  ResPool::size_type ResPool::hardLockQueriesSize() const
  { return _pimpl->hardLockQueries().size(); }

  ResPool::hardLockQueries_iterator ResPool::hardLockQueriesBegin() const
  { return _pimpl->hardLockQueries().begin(); }

  ResPool::hardLockQueries_iterator ResPool::hardLockQueriesEnd() const
  { return _pimpl->hardLockQueries().end(); }

  void ResPool::setHardLockQueries( const HardLockQueries & newLocks_r )
  { _pimpl->setHardLockQueries( newLocks_r ); }

  void ResPool::getHardLockQueries( HardLockQueries & activeLocks_r )
  { _pimpl->getHardLockQueries( activeLocks_r ); }


  const pool::PoolTraits::ItemContainerT & ResPool::store() const
  { return _pimpl->store(); }

  const pool::PoolTraits::Id2ItemT & ResPool::id2item() const
  { return _pimpl->id2item(); }

  ///////////////////////////////////////////////////////////////////
  //
  // Forward to sat::Pool:
  //
  ///////////////////////////////////////////////////////////////////
  void ResPool::setRequestedLocales( const LocaleSet & locales_r )
  { sat::Pool::instance().setRequestedLocales( locales_r ); }

  bool ResPool::addRequestedLocale( const Locale & locale_r )
  { return sat::Pool::instance().addRequestedLocale( locale_r ); }

  bool ResPool::eraseRequestedLocale( const Locale & locale_r )
  { return sat::Pool::instance().eraseRequestedLocale( locale_r ); }

  const LocaleSet & ResPool::getRequestedLocales() const
  { return sat::Pool::instance().getRequestedLocales(); }

  bool ResPool::isRequestedLocale( const Locale & locale_r ) const
  { return sat::Pool::instance().isRequestedLocale( locale_r ); }

  const LocaleSet & ResPool::getAvailableLocales() const
  { return sat::Pool::instance().getAvailableLocales(); }

  bool ResPool::isAvailableLocale( const Locale & locale_r ) const
  { return sat::Pool::instance().isAvailableLocale( locale_r ); }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const ResPool & obj )
  {
    return dumpPoolStats( str << "ResPool " << sat::Pool::instance() << endl << "  ",
                          obj.begin(), obj.end() );
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
