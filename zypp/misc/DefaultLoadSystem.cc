/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/misc/DefaultLoadSystem.cc
 *
*/
#include <iostream>

#include "zypp/base/LogTools.h"
#include "zypp/PathInfo.h"

#include "zypp/misc/DefaultLoadSystem.h"

#include "zypp/ZYppFactory.h"
#include "zypp/zypp_detail/ZYppReadOnlyHack.h"
#include "zypp/Target.h"
#include "zypp/RepoManager.h"
#include "zypp/sat/Pool.h"

using std::endl;

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::misc"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace misc
  { /////////////////////////////////////////////////////////////////

    void defaultLoadSystem( const Pathname & sysRoot_r, LoadSystemFlags flags_r )
    {
      MIL << str::form( "*** Load system at '%s' (%lx)", sysRoot_r.c_str(), (unsigned long)flags_r ) << endl;

      if ( ! PathInfo( sysRoot_r ).isDir() )
        ZYPP_THROW( Exception(str::form("sysRoot_r argument needs to be a directory. (%s)", sysRoot_r.c_str())) );

      if ( ZYppFactory::instance().haveZYpp() )
        ZYPP_THROW( Exception("ZYpp instance is already created. (Call this method earlier.)") );

      if ( flags_r.testFlag( LS_READONLY ) )
        zypp_readonly_hack::IWantIt ();

      sat::Pool satpool( sat::Pool::instance() );

      if ( 1 )
      {
        MIL << "*** load target '" << Repository::systemRepoAlias() << "'\t" << endl;
        getZYpp()->initializeTarget( sysRoot_r );
        getZYpp()->target()->load();
        MIL << satpool.systemRepo() << endl;
      }

      if ( 1 )
      {
        RepoManager repoManager( sysRoot_r );
        RepoInfoList repos = repoManager.knownRepositories();
        for_( it, repos.begin(), repos.end() )
        {
          RepoInfo & nrepo( *it );

          if ( ! nrepo.enabled() )
            continue;

          if ( ! flags_r.testFlag( LS_NOREFRESH ) )
          {
            if ( repoManager.isCached( nrepo )
               && ( nrepo.type() == repo::RepoType::RPMPLAINDIR // refreshes always
                  || repoManager.checkIfToRefreshMetadata( nrepo, nrepo.url() ) == RepoManager::REFRESH_NEEDED ) )
            {
              MIL << str::form( "*** clean cache for repo '%s'\t", nrepo.name().c_str() ) << endl;
              repoManager.cleanCache( nrepo );
              MIL << str::form( "*** refresh repo '%s'\t", nrepo.name().c_str() ) << endl;
              repoManager.refreshMetadata( nrepo );
            }
          }

          if ( ! repoManager.isCached( nrepo ) )
          {
            MIL << str::form( "*** build cache for repo '%s'\t", nrepo.name().c_str() ) << endl;
            repoManager.buildCache( nrepo );
          }

          MIL << str::form( "*** load repo '%s'\t", nrepo.name().c_str() ) << std::flush;
          try
          {
            repoManager.loadFromCache( nrepo );
            MIL << satpool.reposFind( nrepo.alias() ) << endl;
          }
          catch ( const Exception & exp )
          {
            ERR << "*** load repo failed: " << exp.asString() + "\n" + exp.historyAsString() << endl;
            ZYPP_RETHROW ( exp );
          }
        }
      }
      MIL << str::form( "*** Read system at '%s'", sysRoot_r.c_str() ) << endl;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace misc
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
