/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/CommitPackageCache.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"

#include "zypp/target/CommitPackageCache.h"
#include "zypp/target/CommitPackageCacheImpl.h"
#include "zypp/target/CommitPackageCacheReadAhead.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : CommitPackageCache
    //
    ///////////////////////////////////////////////////////////////////

    CommitPackageCache::CommitPackageCache( Impl * pimpl_r )
    : _pimpl( pimpl_r )
    {
      assert( _pimpl );
    }

    CommitPackageCache::CommitPackageCache( const Pathname &        rootDir_r,
                                            const PackageProvider & packageProvider_r )
    {
      if ( getenv("ZYPP_COMMIT_NO_PACKAGE_CACHE") )
        {
          MIL << "$ZYPP_COMMIT_NO_PACKAGE_CACHE is set." << endl;
          _pimpl.reset( new Impl( packageProvider_r ) ); // no cache
        }
      else
        {
          _pimpl.reset( new CommitPackageCacheReadAhead( rootDir_r, packageProvider_r ) );
        }
      assert( _pimpl );
    }

    CommitPackageCache::~CommitPackageCache()
    {}

    void CommitPackageCache::setCommitList( std::vector<sat::Solvable> commitList_r )
    {
      _pimpl->setCommitList( commitList_r );
    }

    ManagedFile CommitPackageCache::get( const PoolItem & citem_r )
    { return _pimpl->get( citem_r ); }

    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const CommitPackageCache & obj )
    { return str << *obj._pimpl; }

    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
