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
#include <sstream>
#include <string>
#include <list>
#include <set>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/base/Iterator.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/UserRequestException.h"

#include "zypp/PoolItem.h"
#include "zypp/Resolvable.h"
#include "zypp/ResObject.h"
#include "zypp/Package.h"
#include "zypp/SrcPackage.h"
#include "zypp/Pattern.h"
#include "zypp/Selection.h"
#include "zypp/Script.h"
#include "zypp/Message.h"
#include "zypp/Url.h"

#include "zypp/CapMatchHelper.h"
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

using namespace std;
using namespace zypp;
using namespace zypp::resfilter;
using zypp::solver::detail::Helper;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace
    { /////////////////////////////////////////////////////////////////
      void ExecuteScriptHelper( repo::RepoMediaAccess & access_r,
                                Script::constPtr script_r,
                                bool do_r,
                                storage::PersistentStorage & storage_r )
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

        PathInfo pi( localfile );
        if ( ! pi.isFile() )
        {
          std::ostringstream err;
          err << "Script is not a file: " << pi.fileType() << " " << localfile;
          report->problem( err.str() );
          ZYPP_THROW(Exception(err.str()));
        }

        filesystem::chmod( localfile, S_IRUSR|S_IWUSR|S_IXUSR );	// "rwx------"
        ExternalProgram prog( localfile->asString(), ExternalProgram::Stderr_To_Stdout, false, -1, true );
        for ( std::string output = prog.receiveLine(); output.length(); output = prog.receiveLine() )
        {
          if ( ! report->progress( ScriptResolvableReport::OUTPUT, output ) )
            {
              WAR << "User request to abort script." << endl;
              prog.kill(); // the rest is handled by exit code evaluation.
            }
        }

        int exitCode = prog.close();
        if ( exitCode != 0 )
        {
          storage_r.setObjectFlag( script_r, "SCRIPT_EXEC_FAILED" );
          std::ostringstream err;
          err << "Script failed with exit code " << exitCode;
          report->problem( err.str() );
          ZYPP_THROW(Exception(err.str()));
        }
        else if ( storage_r.doesObjectHasFlag( script_r, "SCRIPT_EXEC_FAILED" ) )
        {
          storage_r.removeObjectFlag( script_r, "SCRIPT_EXEC_FAILED" );
        }

        report->finish();
        return;
      }

      inline void ExecuteDoScript( repo::RepoMediaAccess & access_r, const Script::constPtr & script_r,
                                   storage::PersistentStorage & storage_r )
      {
        ExecuteScriptHelper( access_r, script_r, true, storage_r );
      }

      inline void ExecuteUndoScript( repo::RepoMediaAccess & access_r, const Script::constPtr & script_r,
                                     storage::PersistentStorage & storage_r )
      {
        ExecuteScriptHelper( access_r, script_r, false, storage_r );
      }
      /////////////////////////////////////////////////////////////////
    } // namespace
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace
    { /////////////////////////////////////////////////////////////////

      /** Helper removing obsoleted non-Package from store. */
      struct StorageRemoveObsoleted
      {
        StorageRemoveObsoleted( storage::PersistentStorage & storage_r,
                                const PoolItem & byPoolitem_r )
        : _storage( storage_r )
        , _byPoolitem( byPoolitem_r )
        {}

        bool operator()( const PoolItem & poolitem_r ) const
        {
          if ( ! poolitem_r.status().isInstalled() )
            return true;

          if ( isKind<Package>(poolitem_r.resolvable()) )
            {
              ERR << "Ignore unsupported Package/non-Package obsolete: "
                  << _byPoolitem << " obsoletes " << poolitem_r << endl;
              return true;
            }

          try
            {
              _storage.deleteObject( poolitem_r.resolvable() );
               MIL<< "Obsoleted: " << poolitem_r << " (by " << _byPoolitem << ")" << endl;
            }
          catch ( Exception & excpt_r )
            {
              ZYPP_CAUGHT( excpt_r );
              WAR << "Failed obsolete: " << poolitem_r << " (by " << _byPoolitem << ")" << endl;
            }

          return true;
        }

      private:
        storage::PersistentStorage & _storage;
        const PoolItem               _byPoolitem;
      };

      /** Helper processing non-Package obsoletes.
      *
      * Scan \a pool_r for items obsoleted \a byPoolitem_r and remove them from
      * \a storage_r.
      */
      void obsoleteMatchesFromStorage( storage::PersistentStorage & storage_r,
                                       const ResPool & pool_r,
                                       const PoolItem & byPoolitem_r )
      {
        forEachPoolItemMatchedBy( pool_r, byPoolitem_r, Dep::OBSOLETES,
                                  OncePerPoolItem( StorageRemoveObsoleted( storage_r,
                                                                           byPoolitem_r ) ) );
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
    TargetImpl::TargetImpl(const Pathname & root_r)
    : _root(root_r), _storage_enabled(false)
    {
      _rpm.initDatabase(root_r);
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

    bool TargetImpl::isStorageEnabled() const
    {
      return _storage_enabled;
    }


    void TargetImpl::enableStorage(const Pathname &root_r)
    {
      _storage.init(root_r);
      _storage_enabled = true;
    }

    Pathname TargetImpl::root() const
    {
      return _root;
    }

    void TargetImpl::loadKindResolvables( const Resolvable::Kind kind )
    {
      // if this kind is already loaded, return
      if ( _resstore_loaded[kind] )
        return;

      if ( kind == ResTraits<zypp::Package>::kind )
      {
        std::list<Package::Ptr> packages = _rpm.getPackages();
        for (std::list<Package::Ptr>::const_iterator it = packages.begin();
             it != packages.end();
             it++)
        {
          _store.insert(*it);
        }
        _resstore_loaded[kind] = true;
      }
      else
      {
        if ( isStorageEnabled() )
        {
          // resolvables stored in the zypp storage database
          std::list<ResObject::Ptr> resolvables = _storage.storedObjects(kind);
          for (std::list<ResObject::Ptr>::iterator it = resolvables.begin();
               it != resolvables.end();
               it++)
          {
            _store.insert(*it);
          }
        }
        else
        {
          WAR << "storage target not enabled" << std::endl;
        }
        _resstore_loaded[kind] = true;
      } // end switch
    }

    ResStore::resfilter_const_iterator TargetImpl::byKindBegin( const ResObject::Kind & kind_r ) const
    {
      TargetImpl *ptr = const_cast<TargetImpl *>(this);
      ptr->loadKindResolvables(kind_r);
      resfilter::ResFilter filter = ByKind(kind_r);
      return make_filter_iterator( filter, _store.begin(), _store.end() );
    }

    ResStore::resfilter_const_iterator TargetImpl::byKindEnd( const ResObject::Kind & kind_r  ) const
    {
      TargetImpl *ptr = const_cast<TargetImpl *>(this);
      ptr->loadKindResolvables(kind_r);
      resfilter::ResFilter filter = ByKind(kind_r);
      return make_filter_iterator( filter, _store.end(), _store.end() );
    }

    const ResStore & TargetImpl::resolvables()
    {
      loadKindResolvables( ResTraits<zypp::Patch>::kind );
      loadKindResolvables( ResTraits<zypp::Selection>::kind );
      loadKindResolvables( ResTraits<zypp::Pattern>::kind );
      loadKindResolvables( ResTraits<zypp::Product>::kind );
      loadKindResolvables( ResTraits<zypp::Language>::kind );
      loadKindResolvables( ResTraits<zypp::Package>::kind );
      return _store;
    }

    void TargetImpl::reset()
    {
      // make this smarter later
      _store.clear();
      _resstore_loaded[ResTraits<zypp::Patch>::kind] = false;
      _resstore_loaded[ResTraits<zypp::Selection>::kind] = false;
      _resstore_loaded[ResTraits<zypp::Pattern>::kind] = false;
      _resstore_loaded[ResTraits<zypp::Product>::kind] = false;
      _resstore_loaded[ResTraits<zypp::Language>::kind] = false;
      _resstore_loaded[ResTraits<zypp::Package>::kind] = false;
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
              success = false;
            }

            if ( success && !policy_r.dryRun() )
            {
              it->status().resetTransact( ResStatus::USER );
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
          if ( isStorageEnabled() )
          {
            if (it->status().isToBeInstalled())
            {
              // Process OBSOLETES and remove them from store.
              obsoleteMatchesFromStorage( _storage, pool_r, *it );

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
                  ExecuteDoScript( access, asKind<Script>(it->resolvable()), _storage );
                }
                else if (!isKind<Atom>(it->resolvable()))	// atoms are re-created from the patch data, no need to save them
                {
                  // #160792 do not just add, also remove older versions
                  if (true) // !installOnly - only on Package?!
                  {
                    // this would delete the same item over and over
                    //for (PoolItem_Ref old = Helper::findInstalledItem (pool_r, *it); old; )
#warning REMOVE ALL OLD VERSIONS AND NOT JUST ONE
                    PoolItem_Ref old = Helper::findInstalledItem (pool_r, *it);
                    if (old)
                    {
                      _storage.deleteObject(old.resolvable());
                    }
                  }
                  _storage.storeObject(it->resolvable());
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
                  ExecuteUndoScript( access, asKind<Script>(it->resolvable()), _storage );
                }
                else
                {
                  _storage.deleteObject(it->resolvable());
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
          }
          else
          {
            WAR << "storage target disabled" << std::endl;
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

    /** Return the resolvable which provides path_str (rpm -qf)
    return NULL if no resolvable provides this file  */
    ResObject::constPtr TargetImpl::whoOwnsFile (const std::string & path_str) const
    {
      string name = _rpm.whoOwnsFile (path_str);
      if (name.empty())
        return NULL;

      for (ResStore::const_iterator it = _store.begin(); it != _store.end(); ++it)
      {
        if ((*it)->name() == name)
        {
          return *it;
        }
      }
      return NULL;
    }

    /** Set the log file for target */
    bool TargetImpl::setInstallationLogfile(const Pathname & path_r)
    {
      CommitLog::setFname(path_r);
      return true;
    }

    Date TargetImpl::timestamp() const
    {
      Date ts_rpm;
      Date ts_store;

      ts_rpm = _rpm.timestamp();

      if ( isStorageEnabled() )
        ts_store = _storage.timestamp();

      if ( ts_rpm > ts_store )
      {
        return ts_rpm;
      }
      else if (ts_rpm < ts_store)
      {
        return ts_store;
      }
      else
      {
        // they are the same
        if ( ts_rpm != 0 )
          return ts_rpm;
        else
          return Date::now();
      }
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
