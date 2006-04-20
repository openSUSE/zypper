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

#include "zypp/solver/detail/InstallOrder.h"

using namespace std;
using zypp::solver::detail::InstallOrder;

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
      _rpm.initDatabase(_root);
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



    ZYppCommitResult TargetImpl::commit( ResPool pool_r, const ZYppCommitPolicy & policy_r )
    {
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

      commit (to_uninstall, policy_r );

      if (policy_r.restrictToMedia() == 0) {			// commit all
        result._remaining = commit( to_install, policy_r );
        result._srcremaining = commit( to_srcinstall, policy_r );
      }
      else
      {
        TargetImpl::PoolItemList current_install;
        TargetImpl::PoolItemList current_srcinstall;

        for (TargetImpl::PoolItemList::iterator it = to_install.begin(); it != to_install.end(); ++it)
        {
          Resolvable::constPtr res( it->resolvable() );
          Package::constPtr pkg( asKind<Package>(res) );
          if (pkg && policy_r.restrictToMedia() != pkg->mediaId())								// check medianr for packages only
          {
            XXX << "Package " << *pkg << ", wrong media " << pkg->mediaId() << endl;
            result._remaining.push_back( *it );
          }
          else
          {
            current_install.push_back( *it );
          }
        }
        TargetImpl::PoolItemList bad = commit( current_install, policy_r );
        result._remaining.insert(result._remaining.end(), bad.begin(), bad.end());

        for (TargetImpl::PoolItemList::iterator it = to_srcinstall.begin(); it != to_srcinstall.end(); ++it)
        {
          Resolvable::constPtr res( it->resolvable() );
          Package::constPtr pkg( asKind<Package>(res) );
          if (pkg && policy_r.restrictToMedia() != pkg->mediaId()) // check medianr for packages only
          {
            XXX << "Package " << *pkg << ", wrong media " << pkg->mediaId() << endl;
            result._srcremaining.push_back( *it );
          }
          else {
            current_srcinstall.push_back( *it );
          }
        }
        bad = commit( current_srcinstall, policy_r );
        result._srcremaining.insert(result._srcremaining.end(), bad.begin(), bad.end());
      }


      result._result = (to_install.size() - result._remaining.size());
      return result;
    }


    TargetImpl::PoolItemList
    TargetImpl::commit( const TargetImpl::PoolItemList & items_r,
                        const ZYppCommitPolicy & policy_r )
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
	    p->source().releaseFile( p->location(), p->mediaId() );
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
        else if (!policy_r.dryRun()) // other resolvables
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

		  MIL << "Displaying the text " << text << endl;
		}
		else if (isKind<Script>(it->resolvable()))
		{
		  Script::constPtr s = dynamic_pointer_cast<const Script>(it->resolvable());
		  Pathname p = s->do_script();
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
		    ERR << "Do script not defined" << endl;
		  }
		}
                else
                {
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

//-----------------------------------------------------------------------------
/******************************************************************
**
**
**	FUNCTION NAME : strip_obsoleted_to_delete
**	FUNCTION TYPE : void
**
** strip packages to_delete which get obsoleted by
** to_install (i.e. delay deletion in case the
** obsoleting package likes to save whatever...
*/

static void
strip_obsoleted_to_delete( TargetImpl::PoolItemList & deleteList_r,
				const TargetImpl::PoolItemList & instlist_r )
{
  if ( deleteList_r.size() == 0 || instlist_r.size() == 0 )
    return; // ---> nothing to do

  // build obsoletes from instlist_r
  CapSet obsoletes;
  for ( TargetImpl::PoolItemList::const_iterator it = instlist_r.begin();
	it != instlist_r.end(); ++it )
  {
    PoolItem_Ref item( *it );
    obsoletes.insert( item->dep(Dep::OBSOLETES).begin(), item->dep(Dep::OBSOLETES).end() );
  }
  if ( obsoletes.size() == 0 )
    return; // ---> nothing to do

  // match them... ;(
  TargetImpl::PoolItemList undelayed;
  // forall applDelete Packages...
  for ( TargetImpl::PoolItemList::iterator it = deleteList_r.begin();
	it != deleteList_r.end(); ++it )
  {
    PoolItem_Ref ipkg( *it );
    bool delayPkg = false;
    // ...check whether an obsoletes....
    for ( CapSet::iterator obs = obsoletes.begin();
	  ! delayPkg && obs != obsoletes.end(); ++obs )
    {
      // ...matches anything provided by the package?
      for ( CapSet::const_iterator prov = ipkg->dep(Dep::PROVIDES).begin();
	    prov != ipkg->dep(Dep::PROVIDES).end(); ++prov )
      {
	if ( obs->matches( *prov ) == CapMatch::yes )
	{
	  // if so, delay package deletion
	  DBG << "Ignore appl_delete (should be obsoleted): " << ipkg << endl;
	  delayPkg = true;
	  ipkg.status().resetTransact( ResStatus::USER );
	  break;
	}
      }
    }
    if ( ! delayPkg ) {
      DBG << "undelayed " << ipkg << endl;
      undelayed.push_back( ipkg );
    }
  }
  // Puhh...
  deleteList_r.swap( undelayed );
}




void
TargetImpl::getResolvablesToInsDel ( const ResPool pool_r,
				    TargetImpl::PoolItemList & dellist_r,
				    TargetImpl::PoolItemList & instlist_r,
				    TargetImpl::PoolItemList & srclist_r ) const
{
    dellist_r.clear();
    instlist_r.clear();
    srclist_r.clear();
    TargetImpl::PoolItemList nonpkglist;

    for ( ResPool::const_iterator it = pool_r.begin(); it != pool_r.end(); ++it )
    {
	if (it->status().isToBeInstalled())
	{
	    if ((*it)->kind() == ResTraits<SrcPackage>::kind) {
		srclist_r.push_back( *it );
	    }
	    else if ((*it)->kind() != ResTraits<Package>::kind) {
		nonpkglist.push_back( *it );
	    }
	    else
		instlist_r.push_back( *it );
	}
	else if (it->status().isToBeUninstalled())
	{
	    if ( it->status().isToBeUninstalledDueToObsolete() )
	    {
		DBG << "Ignore auto_delete (should be obsoleted): " << *it << endl;
	    }
	    else if ( it->status().isToBeUninstalledDueToUpgrade() )
	    {
		DBG << "Ignore auto_delete (should be upgraded): " << *it << endl;
	    }
	    else {
		dellist_r.push_back( *it );
	    }
	}
    }

    MIL << "ResolvablesToInsDel: delete " << dellist_r.size()
      << ", install " << instlist_r.size()
	<< ", srcinstall " << srclist_r.size()
	  << ", nonpkg " << nonpkglist.size() << endl;

    ///////////////////////////////////////////////////////////////////
    //
    // strip packages to_delete which get obsoleted by
    // to_install (i.e. delay deletion in case the
    // obsoleting package likes to save whatever...
    //
    ///////////////////////////////////////////////////////////////////
    strip_obsoleted_to_delete( dellist_r, instlist_r );

    if ( dellist_r.size() ) {
      ///////////////////////////////////////////////////////////////////
      //
      // sort delete list...
      //
      ///////////////////////////////////////////////////////////////////
      TargetImpl::PoolItemSet delset( dellist_r.begin(), dellist_r.end() );  // for delete order
      TargetImpl::PoolItemSet dummy; // dummy, empty, should contain already installed

      InstallOrder order( pool_r, delset, dummy ); // sort according top prereq
      order.init();
      const TargetImpl::PoolItemList dsorted( order.getTopSorted() );

      dellist_r.clear();
      for ( TargetImpl::PoolItemList::const_reverse_iterator cit = dsorted.rbegin();
	    cit != dsorted.rend(); ++cit )
      {
	dellist_r.push_back( *cit );
      }
    }

    ///////////////////////////////////////////////////////////////////
    //
    // sort installed list...
    //
    ///////////////////////////////////////////////////////////////////
    if ( instlist_r.empty() ) {
      instlist_r.splice( instlist_r.end(), nonpkglist );

      return;
    }
#warning Source Rank Priority ?
#if 0
    ///////////////////////////////////////////////////////////////////
    // Get desired order of InstSrc'es to install from.
    ///////////////////////////////////////////////////////////////////
    typedef map<unsigned,unsigned> RankPriority;

    RankPriority rankPriority;
    {
      InstSrcManager::ISrcIdList sourcerank( Y2PM::instSrcManager().instOrderSources() );
      // map InstSrc rank to install priority
      unsigned prio = 0;
      for ( InstSrcManager::ISrcIdList::const_iterator it = sourcerank.begin();
	    it != sourcerank.end(); ++it, ++prio ) {
	rankPriority[(*it)->descr()->default_rank()] = prio;
      }
    }
#endif

    ///////////////////////////////////////////////////////////////////
    // Compute install order according to packages prereq.
    // Try to group packages with respect to the desired install order
    ///////////////////////////////////////////////////////////////////
    // backup list for debug purpose.
    // You can as well build the set, clear the list and rebuild it in install order.
    TargetImpl::PoolItemList instbackup_r;
    instbackup_r.swap( instlist_r );

    TargetImpl::PoolItemSet insset( instbackup_r.begin(), instbackup_r.end() ); // for install order
    TargetImpl::PoolItemSet installed; // dummy, empty, should contain already installed

    InstallOrder order( pool_r, insset, installed );
    // start recursive depth-first-search
    order.init();
    MIL << "order.init() done" << endl;
    order.printAdj( XXX, false );
    ///////////////////////////////////////////////////////////////////
    // build install list in install order
    ///////////////////////////////////////////////////////////////////
    TargetImpl::PoolItemList best_list;
    unsigned best_prio     = 0;
    unsigned best_medianum = 0;

    TargetImpl::PoolItemList last_list;
    unsigned last_prio     = 0;
    unsigned last_medianum = 0;

    TargetImpl::PoolItemList other_list;

    for ( TargetImpl::PoolItemList items = order.computeNextSet(); ! items.empty(); items = order.computeNextSet() )
    {
MIL << "order.computeNextSet: " << items.size() << " resolvables" << endl;
      ///////////////////////////////////////////////////////////////////
      // items contains all packages we could install now. Pick all packages
      // from current media, or best media if none for current.
      ///////////////////////////////////////////////////////////////////

      best_list.clear();
      last_list.clear();
      other_list.clear();

      for ( TargetImpl::PoolItemList::iterator cit = items.begin(); cit != items.end(); ++cit )
      {
	Resolvable::constPtr res( cit->resolvable() );
	if (!res) continue;
	Package::constPtr cpkg( asKind<Package>(res) );
	if (!cpkg) {
	    XXX << "Not a package " << *cit << endl;
	    order.setInstalled( *cit );
	    other_list.push_back( *cit );
	    continue;
	}
	XXX << "Package " << *cpkg << ", media " << cpkg->mediaId() << " last_medianum " << last_medianum << " best_medianum " << best_medianum << endl;
	if ( cpkg->source().numericId() == last_prio &&
	     cpkg->mediaId() == last_medianum ) {
	  // prefer packages on current media.
	  last_list.push_back( *cit );
	  continue;
	}

	if ( last_list.empty() ) {
	  // check for best media as long as there are no packages for current media.

	  if ( ! best_list.empty() ) {

	    if ( cpkg->source().numericId() < best_prio ) {
	      best_list.clear(); // new best
	    } else if ( cpkg->source().numericId() == best_prio ) {
	      if ( cpkg->mediaId() < best_medianum ) {
		best_list.clear(); // new best
	      } else if ( cpkg->mediaId() == best_medianum ) {
		best_list.push_back( *cit ); // same as best -> add
		continue;
	      } else {
		continue; // worse
	      }
	    } else {
	      continue; // worse
	    }
	  }

	  if ( best_list.empty() )
	  {
	    // first package or new best
	    best_list.push_back( *cit );
	    best_prio     = cpkg->source().numericId();
	    best_medianum = cpkg->mediaId();
	    continue;
	  }
	}

      } // for all packages in current set

      ///////////////////////////////////////////////////////////////////
      // remove packages picked from install order and append them to
      // install list.
      ///////////////////////////////////////////////////////////////////
      TargetImpl::PoolItemList & take_list( last_list.empty() ? best_list : last_list );
      if ( last_list.empty() )
      {
	MIL << "SET NEW media " << best_medianum << endl;
	last_prio     = best_prio;
	last_medianum = best_medianum;
      }
      else
      {
	MIL << "SET CONTINUE" << endl;
      }

      for ( TargetImpl::PoolItemList::iterator it = take_list.begin(); it != take_list.end(); ++it )
      {
	order.setInstalled( *it );
	XXX << "SET isrc " << (*it)->source().numericId() << " -> " << (*it) << endl;
      }
      // move everthing from take_list to the end of instlist_r, clean take_list
      instlist_r.splice( instlist_r.end(), take_list );
      // same for other_list
      instlist_r.splice( instlist_r.end(), other_list );

    } // for all sets computed


    if ( instbackup_r.size() != instlist_r.size() )
    {
	ERR << "***************** Lost packages in InstallOrder sort." << endl;
    }
    instlist_r.splice( instlist_r.end(), nonpkglist );
}


    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
