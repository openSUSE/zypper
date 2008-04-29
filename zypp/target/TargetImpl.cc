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

#include "zypp/base/Logger.h"
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

using namespace std;
using namespace zypp;
using namespace zypp::resfilter;
using zypp::solver::detail::Helper;

#define ZYPP_DB ( "/var/lib/zypp/db/" )

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace
    { /////////////////////////////////////////////////////////////////

      // Execute file (passed as pi_t) as script
      // report against report_r
      //
      void ExecuteScript( const Pathname & pn_r,
			  callback::SendReport<ScriptResolvableReport> * report)
      {
        PathInfo pi( pn_r );
        if ( ! pi.isFile() )
        {
          std::ostringstream err;
          err << "Script is not a file: " << pi.fileType() << " " << pn_r;
	  if (report)
	    (*report)->problem( err.str() );
          ZYPP_THROW(Exception(err.str()));
        }

        filesystem::chmod( pn_r, S_IRUSR|S_IWUSR|S_IXUSR );	// "rwx------"
        ExternalProgram prog( pn_r.asString(), ExternalProgram::Stderr_To_Stdout, false, -1, true );

        for ( std::string output = prog.receiveLine(); output.length(); output = prog.receiveLine() )
        {
	  // hmm, this depends on a ScriptResolvableReport :-(
          if ( report
	       && ! (*report)->progress( ScriptResolvableReport::OUTPUT, output ) )
            {
              WAR << "User request to abort script." << endl;
              prog.kill(); // the rest is handled by exit code evaluation.
            }
        }

        int exitCode = prog.close();
        if ( exitCode != 0 )
        {
          std::ostringstream err;
          err << "Script failed with exit code " << exitCode;
	  if (report)
            (*report)->problem( err.str() );
          ZYPP_THROW(Exception(err.str()));
        }
	return;
      }

      // Check for (and run) update script
      // path: directory where to look
      // name,version,release: Script name must match 'name-version.release-' prefix
      //
#warning nedds to be reimplemented excetion safe
      void RunUpdateScript(Pathname path, std::string name, std::string version, std::string release)
      {
	// open the scripts directory

	DIR *dir = opendir(path.asString().c_str());
	if (!dir)
	{
	  WAR << "Cannot access directory " << path << endl;
	  return;
	}

	// compute the name-version.release- prefix
	std::string prefix = name + "-" + version + "-" + release + "-";
	size_t pfx_size = prefix.length();
	if (pfx_size > 255)
	{
	  ERR << "Prefix size (" << pfx_size << ") larger than supported (255)" << endl;
	  pfx_size = 255;
	}

	// scan directory for match
	const char *found = NULL;
	struct dirent *dentry;
	while ((dentry = readdir(dir)))
	{
	  if (strncmp( dentry->d_name, prefix.c_str(), pfx_size) == 0) {
	    found = dentry->d_name;
	    break;
	  }
	}
	if (found)
	{
	  ExecuteScript( Pathname(path / found), NULL );
	}
	closedir(dir);
	return;
      }

      // Fetch and execute remote script
      // access_r: remote access handle
      // script_r: script (resolvable) handle
      // do_r: true for 'do', false for 'undo'
      //
      void ExecuteScriptHelper( repo::RepoMediaAccess & access_r,
                                Script::constPtr script_r,
                                bool do_r )
      {
        MIL << "Execute script " << script_r << endl;
        if ( ! script_r )
        {
          INT << "NULL Script passed." << endl;
          return;
        }

        repo::ScriptProvider prov( access_r );
        ManagedFile localfile = prov.provideScript( script_r, do_r );

        if ( localfile->empty() )
        {
          DBG << "No " << (do_r?"do":"undo") << " script for " << script_r << endl;
          return; // success
        }

        // Go...
        callback::SendReport<ScriptResolvableReport> report;
        report->start( script_r, localfile,
                       (do_r ? ScriptResolvableReport::DO
                        : ScriptResolvableReport::UNDO ) );

	ExecuteScript( localfile, &report );
        report->finish();

        return;
      }

      inline void ExecuteDoScript( repo::RepoMediaAccess & access_r, const Script::constPtr & script_r )
      {
        ExecuteScriptHelper( access_r, script_r, true );
      }

      inline void ExecuteUndoScript( repo::RepoMediaAccess & access_r, const Script::constPtr & script_r )
      {
        ExecuteScriptHelper( access_r, script_r, false );
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
        repo::DeltaCandidates deltas(repos);
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
                                              ZConfig::instance().repoCachePath() / sat::Pool::instance().systemRepoName() );
      filesystem::unlink( base.extend(".solv") );
      filesystem::unlink( base.extend(".cookie") );
    }

    void TargetImpl::buildCache()
    {
      Pathname base = Pathname::assertprefix( _root,
                                              ZConfig::instance().repoCachePath() / sat::Pool::instance().systemRepoName() );
      Pathname rpmsolv = base.extend(".solv");
      Pathname rpmsolvcookie = base.extend(".cookie");

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
        ManagedFile guard( rpmsolv, filesystem::unlink );
        ManagedFile guardcookie( rpmsolvcookie, filesystem::unlink );

        filesystem::Pathname cachePath = Pathname::assertprefix( _root, ZConfig::instance().repoCachePath() );

        // if it does not exist yet, we better create it
        filesystem::assert_dir( cachePath );

        filesystem::TmpFile tmpsolv( cachePath /*dir*/,
                                     sat::Pool::instance().systemRepoName() /* prefix */ );
        if (!tmpsolv)
        {
          Exception ex("Failed to cache rpm database.");
          ex.remember(str::form(
              "Cannot create temporary file under %s.", cachePath.asString().c_str()));
          ZYPP_THROW(ex);
        }

        ostringstream cmd;
        cmd << "rpmdb2solv";

        if ( ! _root.empty() )
          cmd << " -r '" << _root << "'";
#warning DIFF TO EXISTING SOLV FILE DISABLED
#if 0
        // This currently does not work if someone did a rebuilddb.
        // The result is a comletely broken solv file. Thus disabled
        // until rpmdb2solv is fixed.
        if ( solvexisted )
          cmd << " '" << rpmsolv << "'";
#endif
        cmd << "  > '" << tmpsolv.path() << "'";

        MIL << "Executing: " << cmd << endl;
        ExternalProgram prog( cmd.str(), ExternalProgram::Stderr_To_Stdout );

        cmd << endl;
        for ( string output( prog.receiveLine() ); output.length(); output = prog.receiveLine() ) {
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
        guardcookie.resetDispose();
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
      Repository system( satpool.systemRepo() );
      Pathname rpmsolv( Pathname::assertprefix( _root, ZConfig::instance().repoCachePath() + system.name() ).extend(".solv") );
      MIL << "adding " << rpmsolv << " to pool(" << system.name() << ")" << endl;
#warning PROBABLY CLEAR NONEMTY SYSTEM REPO

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
      const LocaleSet & requestedLocales( _requestedLocalesFile.locales() );
      if ( ! requestedLocales.empty() )
      {
        satpool.setRequestedLocales( requestedLocales );
      }

      const SoftLocksFile::Data & softLocks( _softLocksFile.data() );
      if ( ! softLocks.empty() )
      {
        ResPool::instance().setAutoSoftLocks( softLocks );
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

      // Store non-package data:
      filesystem::assert_dir( home() );
      _requestedLocalesFile.setLocales( pool_r.getRequestedLocales() );
      {
        SoftLocksFile::Data newdata;
        pool_r.getActiveSoftLocks( newdata );
        _softLocksFile.setData( newdata );
      }

      // Process packages:
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

      // Try to rebuild solv file while rpm database is still in cache.
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
      MIL << "TargetImpl::commit(<list>" << policy_r << ")" << endl;

      bool abort = false;

      // remember the last used source (if any)
      Repository lastUsedRepo;

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

            lastUsedRepo = p->repository();			// remember the package source

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
	      // check for and run an update script
	      RunUpdateScript(ZConfig::instance().updateScriptsPath(), p->name(), p->edition().version(), p->edition().release());
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
          if (it->status().isToBeInstalled())
          {
            bool success = false;
            try
            {
              if (isKind<Message>(it->resolvable()))
              {
                Message::constPtr m = dynamic_pointer_cast<const Message>(it->resolvable());
                std::string text = m->text().asString();

                callback::SendReport<target::MessageResolvableReport> report;

                report->show( m );

                MIL << "Displaying the text '" << text << "'" << endl;
              }
              else if (isKind<Script>(it->resolvable()))
              {
                ExecuteDoScript( access, asKind<Script>(it->resolvable()));
              }
              else if (!isKind<Atom>(it->resolvable()))	// atoms are re-created from the patch data, no need to save them
              {
                // #160792 do not just add, also remove older versions
                if (true) // !installOnly - only on Package?!
                {
                  // this would delete the same item over and over
                  //for (PoolItem old = Helper::findInstalledItem (pool_r, *it); old; )
                  #warning REMOVE ALL OLD VERSIONS AND NOT JUST ONE
                  PoolItem old = Helper::findInstalledItem (pool_r, *it);
                  if (old)
                  {
                    // FIXME _storage.deleteObject(old.resolvable());
                  }
                }
                // FIXME _storage.storeObject(it->resolvable());
              }
              success = true;
            }
            catch (Exception & excpt_r)
            {
              ZYPP_CAUGHT(excpt_r);
              WAR << "Install of Resolvable from storage failed" << endl;
            }
            if (success)
              it->status().resetTransact( ResStatus::USER );
          }
          else
          {					// isToBeUninstalled
            bool success = false;
            try
            {
              if (isKind<Atom>(it->resolvable()))
              {
                DBG << "Uninstalling atom - no-op" << endl;
              }
              else if (isKind<Message>(it->resolvable()))
              {
                DBG << "Uninstalling message - no-op" << endl;
              }
              else if (isKind<Script>(it->resolvable()))
              {
                ExecuteUndoScript( access, asKind<Script>(it->resolvable()));
              }
              else
              {
                //FIXME _storage.deleteObject(it->resolvable());
              }
              success = true;
            }
            catch (Exception & excpt_r)
            {
              ZYPP_CAUGHT(excpt_r);
              WAR << "Uninstall of Resolvable from storage failed" << endl;
            }
            if (success)
              it->status().resetTransact( ResStatus::USER );
          }

        }  // other resolvables

      } // for

      // we're done with the commit, release the source media
      //   In the case of a single media, end of commit means we don't need _this_
      //   media any more.
      //   In the case of 'commit any media', end of commit means we're completely
      //   done and don't need the source's media anyways.

      if (lastUsedRepo)
      {		// if a source was used
        //lastUsedRepo.release();	//  release their medias
      }

      if ( abort )
        ZYPP_THROW( TargetAbortedException( N_("Installation has been aborted as directed.") ) );

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
