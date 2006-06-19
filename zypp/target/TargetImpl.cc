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
#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
#include <string>
#include <list>
#include <set>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/base/Gettext.h"
#include "zypp/PoolItem.h"
#include "zypp/Resolvable.h"
#include "zypp/ResObject.h"
#include "zypp/Package.h"
#include "zypp/Script.h"
#include "zypp/Message.h"
#include "zypp/Source.h"
#include "zypp/Url.h"

#include "zypp/target/TargetImpl.h"
#include "zypp/target/TargetCallbackReceiver.h"

#include "zypp/pool/GetResolvablesToInsDel.h"
#include "zypp/solver/detail/Helper.h"

using namespace std;
using zypp::solver::detail::Helper;
//using zypp::solver::detail::InstallOrder;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace
    { /////////////////////////////////////////////////////////////////

      struct PubKeyHelper
      {
      };

      /////////////////////////////////////////////////////////////////
    } // namespace
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
    : _root(root_r)
    {
      _rpm.initDatabase(root_r);
      _storage_enabled = false;
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

    const ResStore & TargetImpl::resolvables()
    {
      _store.clear();
      // RPM objects
      std::list<Package::Ptr> packages = _rpm.getPackages();
      for (std::list<Package::Ptr>::const_iterator it = packages.begin();
           it != packages.end();
           it++)
      {
        _store.insert(*it);
      }

      if ( isStorageEnabled() )
      {
        // resolvables stored in the zypp storage database
        std::list<ResObject::Ptr> resolvables = _storage.storedObjects();
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

      return _store;
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
#warning Commit does not provide ZYppCommitResult::_errors

      TargetImpl::PoolItemList to_uninstall;
      TargetImpl::PoolItemList to_install;
      TargetImpl::PoolItemList to_srcinstall;
      getResolvablesToInsDel( pool_r, to_uninstall, to_install, to_srcinstall );

      if ( policy_r.restrictToMedia() ) {
        MIL << "Restrict to media number " << policy_r.restrictToMedia() << endl;
      }

      commit (to_uninstall, policy_r, pool_r );

      if (policy_r.restrictToMedia() == 0) {			// commit all
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
               || ( res->sourceMediaNr() && res->sourceMediaNr() != policy_r.restrictToMedia() ) )
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
          if (pkg && policy_r.restrictToMedia() != pkg->sourceMediaNr()) // check medianr for packages only
          {
            XXX << "Package " << *pkg << ", wrong media " << pkg->sourceMediaNr() << endl;
            result._srcremaining.push_back( *it );
          }
          else {
            current_srcinstall.push_back( *it );
          }
        }
        bad = commit( current_srcinstall, policy_r, pool_r );
        result._srcremaining.insert(result._srcremaining.end(), bad.begin(), bad.end());
      }


      result._result = (to_install.size() - result._remaining.size());
      return result;
    }


    TargetImpl::PoolItemList
    TargetImpl::commit( const TargetImpl::PoolItemList & items_r,
                        const ZYppCommitPolicy & policy_r,
			const ResPool & pool_r )
    {
      TargetImpl::PoolItemList remaining;

      MIL << "TargetImpl::commit(<list>" << policy_r << ")" << endl;

      bool abort = false;

      // remember the last used source (if any)
      Source_Ref lastUsedSource;

      for (TargetImpl::PoolItemList::const_iterator it = items_r.begin(); it != items_r.end(); it++)
      {
        if (isKind<Package>(it->resolvable()))
        {
          Package::constPtr p = dynamic_pointer_cast<const Package>(it->resolvable());
          if (it->status().isToBeInstalled())
          {
	    Pathname localfile;
	    try {
            	localfile = p->source().providePackage(p);
	    }
	    catch( const source::SkipRequestedException & e )
	    {
		ZYPP_CAUGHT( e );
		WAR << "Skipping package " << p << " in commit" << endl;
		continue;
	    }

	    lastUsedSource = p->source();			// remember the package source

#warning Exception handling
	    // create a installation progress report proxy
            RpmInstallPackageReceiver progress( it->resolvable() );
            progress.connect();
            bool success = true;
	    unsigned flags = 0;
            if (p->installOnly()) flags |= rpm::RpmDb::RPMINST_NOUPGRADE;
            if (policy_r.dryRun()) flags |= rpm::RpmDb::RPMINST_TEST;
            if (policy_r.rpmNoSignature()) flags |= rpm::RpmDb::RPMINST_NOSIGNATURE;

            try {
              progress.tryLevel( target::rpm::InstallResolvableReport::RPM );
              rpm().installPackage( localfile, flags );

	      if( progress.aborted() )
	      {
	        WAR << "commit aborted by the user" << endl;
		progress.disconnect();
		abort = true;
		break;
	      }

            }
            catch (Exception & excpt_r) {
              ZYPP_CAUGHT(excpt_r);
              WAR << "Install failed, retrying with --nodeps" << endl;
	      if (policy_r.dryRun()) {
	          WAR << "dry run failed" << endl;
		  progress.disconnect();
		  break;
	      }

              try {
                progress.tryLevel( target::rpm::InstallResolvableReport::RPM_NODEPS );
		flags |= rpm::RpmDb::RPMINST_NODEPS;
                rpm().installPackage( localfile, flags );

	        if( progress.aborted() )
	        {
	          WAR << "commit aborted by the user" << endl;
		  abort = true;
		  progress.disconnect();
		  break;
	        }
              }
              catch (Exception & excpt_r)
              {
                ZYPP_CAUGHT(excpt_r);
                WAR << "Install failed again, retrying with --force --nodeps" << endl;

                try {
                  progress.tryLevel( target::rpm::InstallResolvableReport::RPM_NODEPS_FORCE );
		  flags |= rpm::RpmDb::RPMINST_FORCE;
                  rpm().installPackage( localfile, flags );
                }
                catch (Exception & excpt_r) {
                  remaining.push_back( *it );
                  success = false;
                  ZYPP_CAUGHT(excpt_r);
                }

		if( progress.aborted() )
		{
		    WAR << "commit aborted by the user" << endl;
		    abort = true;
		    progress.disconnect();
		    break;
		}
              }
            }
            if (success
		&& !policy_r.dryRun())
	    {
	      it->status().resetTransact( ResStatus::USER );
            }
            progress.disconnect();
	    p->source().releaseFile( p->location(), p->sourceMediaNr() );
          }
          else
          {
	    bool success = true;

            RpmRemovePackageReceiver progress( it->resolvable() );
            progress.connect();
	    unsigned flags = rpm::RpmDb::RPMINST_NODEPS;
	    if (policy_r.dryRun()) flags |= rpm::RpmDb::RPMINST_TEST;
            try {
              rpm().removePackage( p, flags );
            }
            catch (Exception & excpt_r) {
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
		  Script::constPtr s = dynamic_pointer_cast<const Script>(it->resolvable());
		  Pathname p = s->do_script();
		  if (p != "" && p != "/")
		  {
		    chmod( p.asString().c_str(), S_IRUSR|S_IXUSR );	// "r-x------"
		    ExternalProgram* prog = new ExternalProgram(p.asString(), ExternalProgram::Discard_Stderr, false, -1, true);
		    if (! prog)
		      ZYPP_THROW(Exception("Cannot run the script"));
		    int retval = prog->close();
		    delete prog;
		    if (retval != 0)
		      ZYPP_THROW(Exception("Exit code of script is non-zero"));
		  }
		  else
		  {
		    ERR << "Do script not defined" << endl;
		  }
		}
                else if (!isKind<Atom>(it->resolvable()))	// atoms are re-created from the patch data, no need to save them
                {
		  // #160792 do not just add, also remove older versions
		  if (true) // !installOnly - only on Package?!
		  {
		    // this would delete the same item over and over
		    //for (PoolItem_Ref old = Helper::findInstalledItem (pool_r, *it); old; )
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
		if (isKind<Message>(it->resolvable()))
		{
		  DBG << "Uninstalling message - no-op" << endl;
		}
		else if (isKind<Script>(it->resolvable()))
		{
		  Script::constPtr s = dynamic_pointer_cast<const Script>(it->resolvable());
		  Pathname p = s->undo_script();
		  if (! s->undo_available())
		  {
		    DBG << "Undo script not available" << endl;
		  }
		  if (p != "" && p != "/")
		  {
		    ExternalProgram* prog = new ExternalProgram(p.asString(), ExternalProgram::Discard_Stderr, false, -1, true);
		    if (! prog)
		      ZYPP_THROW(Exception("Cannot run the script"));
		    int retval = prog->close();
		    delete prog;
		    if (retval != 0)
		      ZYPP_THROW(Exception("Exit code of script is non-zero"));
		  }
		  else
		  {
		    ERR << "Undo script not defined" << endl;
		  }
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

      if (lastUsedSource) {		// if a source was used
	lastUsedSource.release();	//  release their medias
      }

      if( abort )
        ZYPP_THROW( TargetAbortedException( N_("Target commit aborted by user.") ) );

      return remaining;
    }

    rpm::RpmDb & TargetImpl::rpm()
    { return _rpm; }

    bool TargetImpl::providesFile (const std::string & path_str, const std::string & name_str) const
    { return _rpm.hasFile(path_str, name_str); }

      /** Return the resolvable which provides path_str (rpm -qf)
	  return NULL if no resolvable provides this file  */
    ResObject::constPtr TargetImpl::whoOwnsFile (const std::string & path_str) const
    {
	string name = _rpm.whoOwnsFile (path_str);
	if (name.empty())
	    return NULL;

	for (ResStore::const_iterator it = _store.begin(); it != _store.end(); ++it) {
	    if ((*it)->name() == name) {
		return *it;
	    }
	}
	return NULL;
    }

    /** Set the log file for target */
    bool TargetImpl::setInstallationLogfile(const Pathname & path_r)
    {
      return rpm::RpmDb::setInstallationLogfile(path_r);
    }

    void
    TargetImpl::getResolvablesToInsDel ( const ResPool pool_r,
                                         TargetImpl::PoolItemList & dellist_r,
                                         TargetImpl::PoolItemList & instlist_r,
                                         TargetImpl::PoolItemList & srclist_r ) const
    {
      pool::GetResolvablesToInsDel collect( pool_r );
      MIL << "GetResolvablesToInsDel: " << endl << collect << endl;
      dellist_r.swap( collect._toDelete );
      instlist_r.swap( collect._toInstall );
      srclist_r.swap( collect._toSrcinstall );
    }

    Date TargetImpl::timestamp() const
    {
      Date ts_rpm;
      Date ts_store;
      
      PathInfo rpmdb_info(root() + "/var/lib/rpm/Packages"); 
      if ( rpmdb_info.isExist() )
        ts_rpm = rpmdb_info.mtime();
          
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
    
    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
