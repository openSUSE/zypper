/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/TargetImpl.cc
 *
*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <set>

#include <sys/types.h>
#include <dirent.h>

#include "zypp/base/LogTools.h"
#include "zypp/base/Exception.h"
#include "zypp/base/Iterator.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/IOStream.h"
#include "zypp/base/UserRequestException.h"

#include "zypp/ZConfig.h"

#include "zypp/PoolItem.h"
#include "zypp/ResObjects.h"
#include "zypp/Url.h"
#include "zypp/TmpPath.h"
#include "zypp/RepoStatus.h"
#include "zypp/ExternalProgram.h"
#include "zypp/Repository.h"

#include "zypp/ResFilters.h"
#include "zypp/target/CommitLog.h"
#include "zypp/target/TargetImpl.h"
#include "zypp/target/TargetCallbackReceiver.h"
#include "zypp/target/rpm/librpmDb.h"
#include "zypp/target/CommitPackageCache.h"

#include "zypp/pool/GetResolvablesToInsDel.h"
#include "zypp/solver/detail/Helper.h"

#include "zypp/repo/DeltaCandidates.h"
#include "zypp/repo/PackageProvider.h"
#include "zypp/repo/ScriptProvider.h"
#include "zypp/repo/SrcPackageProvider.h"

#include "zypp/sat/Pool.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace
    { /////////////////////////////////////////////////////////////////

      /** Execute script and report against report_r.
       * Return \c std::pair<bool,PatchScriptReport::Action> to indicate if
       * execution was successfull (<tt>first = true</tt>), or the desired
       * \c PatchScriptReport::Action in case execution failed
       * (<tt>first = false</tt>).
       *
       * \note The packager is responsible for setting the correct permissions
       * of the script. If the script is not executable it is reported as an
       * error. We must not modify the permessions.
       */
      std::pair<bool,PatchScriptReport::Action> doExecuteScript( const Pathname & root_r,
                                                                 const Pathname & script_r,
                                                                 callback::SendReport<PatchScriptReport> & report_r )
      {
        MIL << "Execute script " << PathInfo(script_r) << endl;

        CommitLog progresslog;
        progresslog(/*timestamp*/true) << script_r << _(" executed") << endl;
        ExternalProgram prog( script_r.asString(), ExternalProgram::Stderr_To_Stdout, false, -1, true /*, root_r*/ );

        for ( std::string output = prog.receiveLine(); output.length(); output = prog.receiveLine() )
        {
          progresslog() << output;
          if ( ! report_r->progress( PatchScriptReport::OUTPUT, output ) )
          {
            WAR << "User request to abort script " << script_r << endl;
            prog.kill();
            // the rest is handled by exit code evaluation
            // in case the script has meanwhile finished.
          }
        }

        std::pair<bool,PatchScriptReport::Action> ret( std::make_pair( false, PatchScriptReport::ABORT ) );

        if ( prog.close() != 0 )
        {
          ret.second = report_r->problem( prog.execError() );
          WAR << "ACTION" << ret.second << "(" << prog.execError() << ")" << endl;
          progresslog(/*timestamp*/true)<< script_r << _(" execution failed") << " (" << prog.execError() << ")" << endl;
          return ret;
        }

        report_r->finish();
        ret.first = true;
        return ret;
      }

      /** Execute script and report against report_r.
       * Return \c false if user requested \c ABORT.
       */
      bool executeScript( const Pathname & root_r,
                          const Pathname & script_r,
                          callback::SendReport<PatchScriptReport> & report_r )
      {
        std::pair<bool,PatchScriptReport::Action> action( std::make_pair( false, PatchScriptReport::ABORT ) );

        do {
          action = doExecuteScript( root_r, script_r, report_r );
          if ( action.first )
            return true; // success

          switch ( action.second )
          {
            case PatchScriptReport::ABORT:
              WAR << "User request to abort at script " << script_r << endl;
              return false; // requested abort.
              break;

            case PatchScriptReport::IGNORE:
              WAR << "User request to skip script " << script_r << endl;
              return true; // requested skip.
              break;

            case PatchScriptReport::RETRY:
              break; // again
          }
        } while ( action.second == PatchScriptReport::RETRY );

        // THIS is not intended to be reached:
        INT << "Abort on unknown ACTION request " << action.second << " returned" << endl;
        return false; // abort.
      }

      /** Look for patch scripts named 'name-version-release-*' and
       *  execute them. Return \c false if \c ABORT was requested.
       */
      bool RunUpdateScripts( const Pathname & root_r,
                             const Pathname & scriptsPath_r,
                             const std::vector<sat::Solvable> & checkPackages_r,
                             bool aborting_r )
      {
        if ( checkPackages_r.empty() )
          return true; // no installed packages to check

        MIL << "Looking for new patch scripts in (" <<  root_r << ")" << scriptsPath_r << endl;
        Pathname scriptsDir( Pathname::assertprefix( root_r, scriptsPath_r ) );
        if ( ! PathInfo( scriptsDir ).isDir() )
          return true; // no script dir

        std::list<std::string> scripts;
        filesystem::readdir( scripts, scriptsDir, /*dots*/false );
        if ( scripts.empty() )
          return true; // no scripts in script dir

        // Now collect and execute all matching scripts.
        // On ABORT: at least log all outstanding scripts.
        bool abort = false;
        for_( it, checkPackages_r.begin(), checkPackages_r.end() )
        {
          std::string prefix( str::form( "%s-%s-", it->name().c_str(), it->edition().c_str() ) );
          for_( sit, scripts.begin(), scripts.end() )
          {
            if ( ! str::hasPrefix( *sit, prefix ) )
              continue;

            PathInfo script( scriptsDir / *sit );
            if ( ! script.isFile() )
              continue;

            if ( abort || aborting_r )
            {
              WAR << "Aborting: Skip patch script " << *sit << endl;
              CommitLog()(/*timestamp*/true) << script.path() << _(" execution skipped while aborting") << endl;
            }
            else
            {
              MIL << "Found patch script " << *sit << endl;
              callback::SendReport<PatchScriptReport> report;
              report->start( make<Package>( *it ), script.path() );

              if ( ! executeScript( root_r, script.path(), report ) )
                abort = true; // requested abort.
            }
          }
        }
        return !abort;
      }

      /////////////////////////////////////////////////////////////////
    } // namespace
    ///////////////////////////////////////////////////////////////////

    /** Helper for PackageProvider queries during commit. */
    struct QueryInstalledEditionHelper
    {
      bool operator()( const std::string & name_r,
                       const Edition &     ed_r,
                       const Arch &        arch_r ) const
      {
        rpm::librpmDb::db_const_iterator it;
        for ( it.findByName( name_r ); *it; ++it )
          {
            if ( arch_r == it->tag_arch()
                 && ( ed_r == Edition::noedition || ed_r == it->tag_edition() ) )
              {
                return true;
              }
          }
        return false;
      }
    };

    /**
     * \short Let the Source provide the package.
     * \p pool_r \ref ResPool used to get candidates
     * \p pi item to be commited
    */
    struct RepoProvidePackage
    {
      ResPool _pool;
      repo::RepoMediaAccess &_access;

      RepoProvidePackage( repo::RepoMediaAccess &access, ResPool pool_r )
        : _pool(pool_r), _access(access)
      {

      }

      ManagedFile operator()( const PoolItem & pi )
      {
        // Redirect PackageProvider queries for installed editions
        // (in case of patch/delta rpm processing) to rpmDb.
        repo::PackageProviderPolicy packageProviderPolicy;
        packageProviderPolicy.queryInstalledCB( QueryInstalledEditionHelper() );

        Package::constPtr p = asKind<Package>(pi.resolvable());


        // Build a repository list for repos
        // contributing to the pool
        std::list<Repository> repos( _pool.knownRepositoriesBegin(), _pool.knownRepositoriesEnd() );
        repo::DeltaCandidates deltas(repos, p->name());
        repo::PackageProvider pkgProvider( _access, p, deltas, packageProviderPolicy );
        return pkgProvider.providePackage();
      }
    };
    ///////////////////////////////////////////////////////////////////

    IMPL_PTR_TYPE(TargetImpl);

    TargetImpl_Ptr TargetImpl::_nullimpl;

    /** Null implementation */
    TargetImpl_Ptr TargetImpl::nullimpl()
    {
      if (_nullimpl == 0)
        _nullimpl = new TargetImpl;
      return _nullimpl;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : TargetImpl::TargetImpl
    //	METHOD TYPE : Ctor
    //
    TargetImpl::TargetImpl( const Pathname & root_r, bool doRebuild_r )
    : _root( root_r )
    , _requestedLocalesFile( home() / "RequestedLocales" )
    , _softLocksFile( home() / "SoftLocks" )
    , _hardLocksFile( Pathname::assertprefix( _root, ZConfig::instance().locksFile() ) )
    {
      _rpm.initDatabase( root_r, Pathname(), doRebuild_r );
      MIL << "Initialized target on " << _root << endl;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : TargetImpl::~TargetImpl
    //	METHOD TYPE : Dtor
    //
    TargetImpl::~TargetImpl()
    {
      _rpm.closeDatabase();
      MIL << "Targets closed" << endl;
    }

    void TargetImpl::clearCache()
    {
      Pathname base = Pathname::assertprefix( _root,
                                              ZConfig::instance().repoSolvfilesPath() / sat::Pool::instance().systemRepoAlias() );
      filesystem::recursive_rmdir( base );
    }

    void TargetImpl::buildCache()
    {
      Pathname base = Pathname::assertprefix( _root,
                                              ZConfig::instance().repoSolvfilesPath() / sat::Pool::instance().systemRepoAlias() );
      Pathname rpmsolv       = base/"solv";
      Pathname rpmsolvcookie = base/"cookie";

      bool build_rpm_solv = true;
      // lets see if the rpm solv cache exists

      RepoStatus rpmstatus(_root + "/var/lib/rpm/Name");
      bool solvexisted = PathInfo(rpmsolv).isExist();
      if ( solvexisted )
      {
        // see the status of the cache
        PathInfo cookie( rpmsolvcookie );
        MIL << "Read cookie: " << cookie << endl;
        if ( cookie.isExist() )
        {
          RepoStatus status = RepoStatus::fromCookieFile(rpmsolvcookie);
          // now compare it with the rpm database
          if ( status.checksum() == rpmstatus.checksum() )
            build_rpm_solv = false;
          MIL << "Read cookie: " << rpmsolvcookie << " says: "
              << (build_rpm_solv ? "outdated" : "uptodate") << endl;
        }
      }

      if ( build_rpm_solv )
      {
        // Take care we unlink the solvfile on exception
        ManagedFile guard( base, filesystem::recursive_rmdir );

        // if it does not exist yet, we better create it
        filesystem::assert_dir( base );

        filesystem::TmpFile tmpsolv( filesystem::TmpFile::makeSibling( rpmsolv ) );
        if (!tmpsolv)
        {
          Exception ex("Failed to cache rpm database.");
          ex.remember(str::form(
              "Cannot create temporary file under %s.", base.c_str()));
          ZYPP_THROW(ex);
        }

        std::ostringstream cmd;
        cmd << "rpmdb2solv";
        if ( ! _root.empty() )
          cmd << " -r '" << _root << "'";

        if ( solvexisted )
          cmd << " '" << rpmsolv << "'";

        cmd << "  > '" << tmpsolv.path() << "'";

        MIL << "Executing: " << cmd << endl;
        ExternalProgram prog( cmd.str(), ExternalProgram::Stderr_To_Stdout );

        cmd << endl;
        for ( std::string output( prog.receiveLine() ); output.length(); output = prog.receiveLine() ) {
          WAR << "  " << output;
          cmd << "     " << output;
        }

        int ret = prog.close();
        if ( ret != 0 )
        {
          Exception ex(str::form("Failed to cache rpm database (%d).", ret));
          ex.remember( cmd.str() );
          ZYPP_THROW(ex);
        }

        ret = filesystem::rename( tmpsolv, rpmsolv );
        if ( ret != 0 )
          ZYPP_THROW(Exception("Failed to move cache to final destination"));
        // if this fails, don't bother throwing exceptions
        filesystem::chmod( rpmsolv, 0644 );

        rpmstatus.saveToCookieFile(rpmsolvcookie);

        // We keep it.
        guard.resetDispose();
      }
    }

    void TargetImpl::unload()
    {
      Repository system( sat::Pool::instance().findSystemRepo() );
      if ( system )
        system.eraseFromPool();
    }


    void TargetImpl::load()
    {
      buildCache();

      // now add the repos to the pool
      sat::Pool satpool( sat::Pool::instance() );
      Pathname rpmsolv( Pathname::assertprefix( _root,
                        ZConfig::instance().repoSolvfilesPath() / satpool.systemRepoAlias() / "solv" ) );
      MIL << "adding " << rpmsolv << " to pool(" << satpool.systemRepoAlias() << ")" << endl;

      // Providing an empty system repo, unload any old content
      Repository system( sat::Pool::instance().findSystemRepo() );
      if ( system && ! system.solvablesEmpty() )
      {
        system.eraseFromPool(); // invalidates system
      }
      if ( ! system )
      {
        system = satpool.systemRepo();
      }

      try
      {
        system.addSolv( rpmsolv );
      }
      catch ( const Exception & exp )
      {
        ZYPP_CAUGHT( exp );
        MIL << "Try to handle exception by rebuilding the solv-file" << endl;
        clearCache();
        buildCache();

        system.addSolv( rpmsolv );
      }

      // (Re)Load the requested locales et al.
      // If the requested locales are empty, we leave the pool untouched
      // to avoid undoing changes the application applied. We expect this
      // to happen on a bare metal installation only. An already existing
      // target should be loaded before its settings are changed.
      {
        const LocaleSet & requestedLocales( _requestedLocalesFile.locales() );
        if ( ! requestedLocales.empty() )
        {
          satpool.setRequestedLocales( requestedLocales );
        }
      }
      {
        const SoftLocksFile::Data & softLocks( _softLocksFile.data() );
        if ( ! softLocks.empty() )
        {
          ResPool::instance().setAutoSoftLocks( softLocks );
        }
      }
      if ( ZConfig::instance().apply_locks_file() )
      {
        const HardLocksFile::Data & hardLocks( _hardLocksFile.data() );
        if ( ! hardLocks.empty() )
        {
          ResPool::instance().setHardLockQueries( hardLocks );
        }
      }


      MIL << "Target loaded: " << system.solvablesSize() << " resolvables" << endl;
    }

    ZYppCommitResult TargetImpl::commit( ResPool pool_r, const ZYppCommitPolicy & policy_rX )
    {
      // ----------------------------------------------------------------- //
      // Fake outstanding YCP fix: Honour restriction to media 1
      // at installation, but install all remaining packages if post-boot.
      ZYppCommitPolicy policy_r( policy_rX );
      if ( policy_r.restrictToMedia() > 1 )
        policy_r.allMedia();
      // ----------------------------------------------------------------- //

      MIL << "TargetImpl::commit(<pool>, " << policy_r << ")" << endl;

      ///////////////////////////////////////////////////////////////////
      // Store non-package data:
      ///////////////////////////////////////////////////////////////////
      filesystem::assert_dir( home() );
      // requested locales
      _requestedLocalesFile.setLocales( pool_r.getRequestedLocales() );
      // weak locks
      {
        SoftLocksFile::Data newdata;
        pool_r.getActiveSoftLocks( newdata );
        _softLocksFile.setData( newdata );
      }
      // hard locks
      if ( ZConfig::instance().apply_locks_file() )
      {
        HardLocksFile::Data newdata;
        pool_r.getHardLockQueries( newdata );
        _hardLocksFile.setData( newdata );
      }

      ///////////////////////////////////////////////////////////////////
      // Process packages:
      ///////////////////////////////////////////////////////////////////
      if ( root() == "/" && CommitLog::fname().empty() )
      {
        // Yes, we simply hijack /var/log/YaST2/y2logRPM
        // until we maintain some zypp history database.
        CommitLog::setFname( "/var/log/YaST2/y2logRPM" );
      }

      ZYppCommitResult result;

      TargetImpl::PoolItemList to_uninstall;
      TargetImpl::PoolItemList to_install;
      TargetImpl::PoolItemList to_srcinstall;
      {

        pool::GetResolvablesToInsDel
        collect( pool_r, policy_r.restrictToMedia() ? pool::GetResolvablesToInsDel::ORDER_BY_MEDIANR
                 : pool::GetResolvablesToInsDel::ORDER_BY_SOURCE );
        MIL << "GetResolvablesToInsDel: " << endl << collect << endl;
        to_uninstall.swap( collect._toDelete );
        to_install.swap( collect._toInstall );
        to_srcinstall.swap( collect._toSrcinstall );
      }

      if ( policy_r.restrictToMedia() )
      {
        MIL << "Restrict to media number " << policy_r.restrictToMedia() << endl;
      }

      ///////////////////////////////////////////////////////////////////
      // First collect and display all messages
      // associated with patches to be installed.
      ///////////////////////////////////////////////////////////////////
      for_( it, to_install.begin(), to_install.end() )
      {
        if ( ! isKind<Patch>(it->resolvable()) )
          continue;
        if ( ! it->status().isToBeInstalled() )
          continue;

        Patch::constPtr patch( asKind<Patch>(it->resolvable()) );
        if ( ! patch->message().empty() )
        {
          MIL << "Show message for " << patch << endl;
          callback::SendReport<target::PatchMessageReport> report;
          if ( ! report->show( patch ) )
          {
            WAR << "commit aborted by the user" << endl;
            ZYPP_THROW( TargetAbortedException( N_("Installation has been aborted as directed.") ) );
          }
        }
      }

      ///////////////////////////////////////////////////////////////////
      // Remove/install packages.
      ///////////////////////////////////////////////////////////////////
     commit (to_uninstall, policy_r, pool_r );

      if (policy_r.restrictToMedia() == 0)
      {			// commit all
        result._remaining = commit( to_install, policy_r, pool_r );
        result._srcremaining = commit( to_srcinstall, policy_r, pool_r );
      }
      else
      {
        TargetImpl::PoolItemList current_install;
        TargetImpl::PoolItemList current_srcinstall;

        // Collect until the 1st package from an unwanted media occurs.
        // Further collection could violate install order.
        bool hitUnwantedMedia = false;
        for (TargetImpl::PoolItemList::iterator it = to_install.begin(); it != to_install.end(); ++it)
        {
          ResObject::constPtr res( it->resolvable() );

          if ( hitUnwantedMedia
               || ( res->mediaNr() && res->mediaNr() != policy_r.restrictToMedia() ) )
          {
            hitUnwantedMedia = true;
            result._remaining.push_back( *it );
          }
          else
          {
            current_install.push_back( *it );
          }
        }

        TargetImpl::PoolItemList bad = commit( current_install, policy_r, pool_r );
        result._remaining.insert(result._remaining.end(), bad.begin(), bad.end());

        for (TargetImpl::PoolItemList::iterator it = to_srcinstall.begin(); it != to_srcinstall.end(); ++it)
        {
          Resolvable::constPtr res( it->resolvable() );
          Package::constPtr pkg( asKind<Package>(res) );
          if (pkg && policy_r.restrictToMedia() != pkg->mediaNr()) // check medianr for packages only
          {
            XXX << "Package " << *pkg << ", wrong media " << pkg->mediaNr() << endl;
            result._srcremaining.push_back( *it );
          }
          else
          {
            current_srcinstall.push_back( *it );
          }
        }
        bad = commit( current_srcinstall, policy_r, pool_r );
        result._srcremaining.insert(result._srcremaining.end(), bad.begin(), bad.end());
      }

      ///////////////////////////////////////////////////////////////////
      // Try to rebuild solv file while rpm database is still in cache.
      ///////////////////////////////////////////////////////////////////
      buildCache();

      result._result = (to_install.size() - result._remaining.size());
      MIL << "TargetImpl::commit(<pool>, " << policy_r << ") returns: " << result << endl;
      return result;
    }


    TargetImpl::PoolItemList
    TargetImpl::commit( const TargetImpl::PoolItemList & items_r,
                        const ZYppCommitPolicy & policy_r,
                        const ResPool & pool_r )
    {
      TargetImpl::PoolItemList remaining;
      repo::RepoMediaAccess access;
      MIL << "TargetImpl::commit(<list>" << policy_r << ")" << items_r.size() << endl;

      bool abort = false;
      std::vector<sat::Solvable> successfullyInstalledPackages;

      RepoProvidePackage repoProvidePackage( access, pool_r);
      // prepare the package cache.
      CommitPackageCache packageCache( items_r.begin(), items_r.end(),
                                       root() / "tmp", repoProvidePackage );

      for (TargetImpl::PoolItemList::const_iterator it = items_r.begin(); it != items_r.end(); it++)
      {
        if (isKind<Package>(it->resolvable()))
        {
          Package::constPtr p = asKind<Package>(it->resolvable());
          if (it->status().isToBeInstalled())
          {
            ManagedFile localfile;
            try
            {
              localfile = packageCache.get( it );
            }
            catch ( const SkipRequestException &e )
            {
              ZYPP_CAUGHT( e );
              WAR << "Skipping package " << p << " in commit" << endl;
              continue;
            }
#if 0
            // bnc #395704: missing catch causes abort. see if packageCache fails to handle
            // errors correctly.
            catch ( const Exception &e )
            {
              ZYPP_CAUGHT( e );
              SEC << e << endl;
            }
#endif

#warning Exception handling
            // create a installation progress report proxy
            RpmInstallPackageReceiver progress( it->resolvable() );
            progress.connect();
            bool success = true;
            unsigned flags = 0;
            // Why force and nodeps?
            //
            // Because zypp builds the transaction and the resolver asserts that
            // everything is fine.
            // We use rpm just to unpack and register the package in the database.
            // We do this step by step, so rpm is not aware of the bigger context.
            // So we turn off rpms internal checks, because we do it inside zypp.
            flags |= rpm::RpmDb::RPMINST_NODEPS;
            flags |= rpm::RpmDb::RPMINST_FORCE;
            //
            if (p->installOnly()) flags |= rpm::RpmDb::RPMINST_NOUPGRADE;
            if (policy_r.dryRun()) flags |= rpm::RpmDb::RPMINST_TEST;
            if (policy_r.rpmNoSignature()) flags |= rpm::RpmDb::RPMINST_NOSIGNATURE;

            try
            {
              progress.tryLevel( target::rpm::InstallResolvableReport::RPM_NODEPS_FORCE );
              rpm().installPackage( localfile, flags );

              if ( progress.aborted() )
              {
                WAR << "commit aborted by the user" << endl;
                progress.disconnect();
                success = false;
                abort = true;
                break;
              }
            }
            catch (Exception & excpt_r)
            {
              ZYPP_CAUGHT(excpt_r);
              if ( policy_r.dryRun() )
              {
                WAR << "dry run failed" << endl;
                progress.disconnect();
                break;
              }
              // else
              WAR << "Install failed" << endl;
              remaining.push_back( *it );
              progress.disconnect();
              success = false;
              break;
            }

            if ( success && !policy_r.dryRun() )
            {
              it->status().resetTransact( ResStatus::USER );
              // Remember to check this package for presence of patch scripts.
              successfullyInstalledPackages.push_back( it->satSolvable() );
            }
            progress.disconnect();
          }
          else
          {
            bool success = true;

            RpmRemovePackageReceiver progress( it->resolvable() );
            progress.connect();
            unsigned flags = rpm::RpmDb::RPMINST_NODEPS;
            if (policy_r.dryRun()) flags |= rpm::RpmDb::RPMINST_TEST;
            try
            {
              rpm().removePackage( p, flags );

              if ( progress.aborted() )
              {
                WAR << "commit aborted by the user" << endl;
                progress.disconnect();
                success = false;
                abort = true;
                break;
              }
            }
            catch (Exception & excpt_r)
            {
              WAR << "removal of " << p << " failed";
              success = false;
              ZYPP_CAUGHT( excpt_r );
            }
            if (success
                && !policy_r.dryRun())
            {
              it->status().resetTransact( ResStatus::USER );
            }
            progress.disconnect();
          }
        }
        else if (!policy_r.dryRun()) // other resolvables (non-Package)
        {
          it->status().resetTransact( ResStatus::USER );
        }  // other resolvables

      } // for

      // Check presence of patch scripts. If aborting, at least log
      // omitted scripts.
      if ( ! successfullyInstalledPackages.empty() )
      {
        if ( ! RunUpdateScripts( _root, ZConfig::instance().update_scriptsPath(),
                                 successfullyInstalledPackages, abort ) )
        {
          WAR << "Commit aborted by the user" << endl;
          abort = true;
        }
      }

      if ( abort )
      {
        ZYPP_THROW( TargetAbortedException( N_("Installation has been aborted as directed.") ) );
      }

     return remaining;
    }

    rpm::RpmDb & TargetImpl::rpm()
    {
      return _rpm;
    }

    bool TargetImpl::providesFile (const std::string & path_str, const std::string & name_str) const
    {
      return _rpm.hasFile(path_str, name_str);
    }

    /** Set the log file for target */
    bool TargetImpl::setInstallationLogfile(const Pathname & path_r)
    {
      CommitLog::setFname(path_r);
      return true;
    }

    Date TargetImpl::timestamp() const
    {
      return _rpm.timestamp();
    }

    std::string TargetImpl::release() const
    {
      std::ifstream suseRelease( (_root / "/etc/SuSE-release").c_str() );
      for( iostr::EachLine in( suseRelease ); in; in.next() )
      {
        std::string line( str::trim( *in ) );
        if ( ! line.empty() )
          return line;
      }

      return _("Unknown Distribution");
    }

    void TargetImpl::installSrcPackage( const SrcPackage_constPtr & srcPackage_r )
    {
      // provide on local disk
      repo::RepoMediaAccess access_r;
      repo::SrcPackageProvider prov( access_r );
      ManagedFile localfile = prov.provideSrcPackage( srcPackage_r );
      // install it
      rpm().installPackage ( localfile );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
