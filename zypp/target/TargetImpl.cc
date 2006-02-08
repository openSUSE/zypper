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
#include "zypp/PoolItem.h"
#include "zypp/Resolvable.h"
#include "zypp/ResObject.h"
#include "zypp/Package.h"

#include "zypp/target/TargetImpl.h"
#include "zypp/target/TargetCallbackReceiver.h"

#include "zypp/solver/detail/Types.h"
#include "zypp/solver/detail/InstallOrder.h"

using namespace std;
using zypp::solver::detail::InstallOrder;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////

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
      // TODO objects from the XML store
      return _store;
    }

    
    Pathname TargetImpl::getRpmFile(Package::constPtr package)
    {
	callback::SendReport<source::DownloadResolvableReport> report;

	// FIXME: error handling
	// FIXME: Url	
	report->start( package, Url() );

	Pathname file = package->getPlainRpm();

	report->finish( package, source::DownloadResolvableReport::NO_ERROR, "" );
	
	return file;
    }


    void TargetImpl::commit(ResPool pool_r, unsigned int medianr, PoolItemList & errors_r, PoolItemList & remaining_r, PoolItemList & srcremaining_r)
    {
      MIL << "TargetImpl::commit(<pool>, " << medianr << ")" << endl;

      errors_r.clear();
      remaining_r.clear();
      srcremaining_r.clear();

      PoolItemList to_uninstall;
      PoolItemList to_install;
      PoolItemList to_srcinstall;
      getResolvablesToInsDel( pool_r, to_uninstall, to_install, to_srcinstall );

      if ( medianr ) {
	MIL << "Restrict to media number " << medianr << endl;
      }

      commit (to_uninstall);

      if (medianr == 0) {			// commit all
	remaining_r = commit( to_install );
	srcremaining_r = commit( to_srcinstall );
      }
      else {
	PoolItemList current_install;
	PoolItemList current_srcinstall;

	for (PoolItemList::iterator it = to_install.begin(); it != to_install.end(); ++it) {
	    Resolvable::constPtr res( it->resolvable() );
	    Package::constPtr pkg( asKind<Package>(res) );
	    if (pkg
		&& medianr != pkg->mediaId())								// check medianr for packages only
	    {
		MIL << "Package " << *pkg << ", wrong media " << pkg->mediaId() << endl;
		remaining_r.push_back( *it );
	    }
	    else {
		current_install.push_back( *it );
	    }
	}
	PoolItemList bad = commit (current_install);
	remaining_r.insert(remaining_r.end(), bad.begin(), bad.end());

	for (PoolItemList::iterator it = to_srcinstall.begin(); it != to_srcinstall.end(); ++it) {
	    Resolvable::constPtr res( it->resolvable() );
	    Package::constPtr pkg( asKind<Package>(res) );
	    if (pkg
		&& medianr != pkg->mediaId())								// check medianr for packages only
	    {
		MIL << "Package " << *pkg << ", wrong media " << pkg->mediaId() << endl;
		srcremaining_r.push_back( *it );
	    }
	    else {
		current_srcinstall.push_back( *it );
	    }
	}
	bad = commit (current_srcinstall);
	srcremaining_r.insert(remaining_r.end(), bad.begin(), bad.end());
      }

      return;
    }


    PoolItemList
    TargetImpl::commit( const PoolItemList & items_r)
    {
      PoolItemList remaining;

      MIL << "TargetImpl::commit(<list>)" << endl;
      for (PoolItemList::const_iterator it = items_r.begin();
	it != items_r.end(); it++)
      {
	if (isKind<Package>(it->resolvable()))
	{
	  Package::constPtr p = dynamic_pointer_cast<const Package>(it->resolvable());
	  if (it->status().isToBeInstalled())
	  {
	    Pathname localfile = getRpmFile(p);
	    
#warning Exception handling
	    // create a installation progress report proxy
    	    RpmInstallPackageReceiver progress(it->resolvable());
	    
	    progress.connect();
	    bool success = true;

	    try {
		progress.tryLevel( target::rpm::InstallResolvableReport::RPM );
		
		rpm().installPackage(localfile,
		    p->installOnly() ? rpm::RpmDb::RPMINST_NOUPGRADE : 0);
	    }
	    catch (Exception & excpt_r) {
		ZYPP_CAUGHT(excpt_r);

		WAR << "Install failed, retrying with --nodeps" << endl;
		try {
		    progress.tryLevel( target::rpm::InstallResolvableReport::RPM_NODEPS );
		    rpm().installPackage(localfile,
			p->installOnly() ? rpm::RpmDb::RPMINST_NOUPGRADE : rpm::RpmDb::RPMINST_NODEPS);
		}
		catch (Exception & excpt_r) {
		    ZYPP_CAUGHT(excpt_r);
		    WAR << "Install failed again, retrying with --force --nodeps" << endl;

		    try {
			progress.tryLevel( target::rpm::InstallResolvableReport::RPM_NODEPS_FORCE );
			rpm().installPackage(localfile,
			    p->installOnly() ? rpm::RpmDb::RPMINST_NOUPGRADE : (rpm::RpmDb::RPMINST_NODEPS|rpm::RpmDb::RPMINST_FORCE));
		    }
		    catch (Exception & excpt_r) {
			remaining.push_back( *it );
			success = false;
			ZYPP_CAUGHT(excpt_r);
		    }
		}
	    }
	    if (success) {
		it->status().setStatus( ResStatus::installed );
	    }
	    progress.disconnect();

	  }
	  else
	  {
    	    RpmRemovePackageReceiver progress(it->resolvable());
	    
	    progress.connect();

	    try {
		rpm().removePackage(p);
	    }
	    catch (Exception & excpt_r) {
		ZYPP_CAUGHT(excpt_r);
		WAR << "Remove failed, retrying with --nodeps" << endl;
		rpm().removePackage(p, rpm::RpmDb::RPMINST_NODEPS);
	    }
	    progress.disconnect();
	    it->status().setStatus( ResStatus::uninstalled );
	  }

	}
#warning FIXME other resolvables
      }

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
strip_obsoleted_to_delete( PoolItemList & deleteList_r,
				const PoolItemList & instlist_r )
{
  if ( deleteList_r.size() == 0 || instlist_r.size() == 0 )
    return; // ---> nothing to do

  // build obsoletes from instlist_r
  CapSet obsoletes;
  for ( PoolItemList::const_iterator it = instlist_r.begin();
	it != instlist_r.end(); ++it )
  {
    PoolItem_Ref item( *it );
    obsoletes.insert( item->dep(Dep::OBSOLETES).begin(), item->dep(Dep::OBSOLETES).end() );
  }
  if ( obsoletes.size() == 0 )
    return; // ---> nothing to do

  // match them... ;(
  PoolItemList undelayed;
  // forall applDelete Packages...
  for ( PoolItemList::iterator it = deleteList_r.begin();
	it != deleteList_r.end(); ++it )
  {
    PoolItem_Ref ipkg( *it );
    bool delayPkg = false;
    // ...check whether an obsolets....
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
	  break;
	}
      }
    }
    if ( ! delayPkg ) {
MIL << "undelayed " << ipkg << endl;
      undelayed.push_back( ipkg );
    }
  }
  // Puhh...
  deleteList_r.swap( undelayed );
}




void
TargetImpl::getResolvablesToInsDel ( const ResPool pool_r,
				    PoolItemList & dellist_r,
				    PoolItemList & instlist_r,
				    PoolItemList & srclist_r )
{
    dellist_r.clear();
    instlist_r.clear();
    srclist_r.clear();

    for ( ResPool::const_iterator it = pool_r.begin(); it != pool_r.end(); ++it )
    {
	if (it->status().isToBeInstalled())
	{
	    if (it->resolvable()->arch() == Arch_src)
		srclist_r.push_back( *it );
	    else
		instlist_r.push_back( *it );
	}
	else if (it->status().isToBeUninstalled())
	{
	    if ( it->status().isToBeUninstalledDueToObsolete() )
	    {
		DBG << "Ignore auto_delete (should be obsoleted): " << *it << endl;
	    } else {
		dellist_r.push_back( *it );
	    }
	}
    }

    MIL << "PackagesToInsDel: delete " << dellist_r.size()
      << ", install " << instlist_r.size()
	<< ", srcinstall " << srclist_r.size() << endl;

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
      PoolItemList dlist;  // for delete order
      PoolItemList dummy; // dummy, empty, should contain already installed
      for ( PoolItemList::iterator pkgIt = dellist_r.begin();
	    pkgIt != dellist_r.end(); ++pkgIt )
      {
	dlist.push_back( *pkgIt );
      }

      InstallOrder order( pool_r, dlist, dummy ); // sort according top prereq
      order.init();
      const PoolItemList dsorted( order.getTopSorted() );

      dellist_r.clear();
      for ( PoolItemList::const_reverse_iterator cit = dsorted.rbegin();
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
    PoolItemList instbackup_r;
    instbackup_r.swap( instlist_r );

    PoolItemList ilist; // for install order
    PoolItemList installed; // dummy, empty, should contain already installed
    for ( PoolItemList::iterator pkgIt = instbackup_r.begin(); pkgIt != instbackup_r.end(); ++pkgIt ) {
      ilist.push_back( *pkgIt );
    }
    InstallOrder order( pool_r, ilist, installed );
    // start recursive depth-first-search
    order.startrdfs();

    ///////////////////////////////////////////////////////////////////
    // build install list in install order
    ///////////////////////////////////////////////////////////////////
    PoolItemList best_list;
//    unsigned best_prio     = 0;
    unsigned best_medianum = 0;

    PoolItemList last_list;
//    unsigned last_prio     = 0;
    unsigned last_medianum = 0;

    for ( PoolItemList pkgs = order.computeNextSet(); ! pkgs.empty(); pkgs = order.computeNextSet() )
    {
MIL << "order.computeNextSet: " << pkgs.size() << " packages" << endl;
      ///////////////////////////////////////////////////////////////////
      // pkgs contains all packages we could install now. Pick all packages
      // from current media, or best media if none for current.
      ///////////////////////////////////////////////////////////////////

      best_list.clear();
      last_list.clear();

      for ( PoolItemList::iterator cit = pkgs.begin(); cit != pkgs.end(); ++cit )
      {
	Resolvable::constPtr res( cit->resolvable() );
	Package::constPtr cpkg( asKind<Package>(res) );

MIL << "Packge " << cpkg << ", media " << cpkg->mediaId() << endl;
	if ( 									//  rankPriority[cpkg->instSrcRank()] == last_prio &&
	     cpkg->mediaId() == last_medianum ) {
	  // prefer packages on current media.
	  last_list.push_back( *cit );
	  continue;
	}

	if ( last_list.empty() ) {
	  // check for best media as long as there are no packages for current media.

	  if ( ! best_list.empty() ) {

#if 0
	    if ( rankPriority[cpkg->instSrcRank()] < best_prio ) {
	      best_list.clear(); // new best
	    } else if ( rankPriority[cpkg->instSrcRank()] == best_prio ) {
#endif

	      if ( cpkg->mediaId() < best_medianum ) {
		best_list.clear(); // new best
	      } else if ( cpkg->mediaId() == best_medianum ) {
		best_list.push_back( *cit ); // same as best -> add
		continue;
	      } else {
		continue; // worse
	      }
#if 0
	    } else {
	      continue; // worse
	    }
#endif
	  }

	  if ( best_list.empty() )
	  {
	    // first package or new best
	    best_list.push_back( *cit );
//	    best_prio     = rankPriority[cpkg->instSrcRank()];
	    best_medianum = cpkg->mediaId();
	    continue;
	  }
	}

      } // for all packages in current set

      ///////////////////////////////////////////////////////////////////
      // remove packages picked from install order and append them to
      // install list.
      ///////////////////////////////////////////////////////////////////
      PoolItemList & take_list( last_list.empty() ? best_list : last_list );
      if ( last_list.empty() )
      {
	MIL << "SET NEW media " << best_medianum << endl;
//	last_prio     = best_prio;
	last_medianum = best_medianum;
      }
      else
      {
	MIL << "SET CONTINUE" << endl;
      }

      for ( PoolItemList::iterator it = take_list.begin(); it != take_list.end(); ++it )
      {
	order.setInstalled( *it );
	MIL << "SET isrc " << (*it) << endl;
      }
      instlist_r.splice( instlist_r.end(), take_list );

    } // for all sets computed


    if ( instbackup_r.size() != instlist_r.size() )
    {
	INT << "Lost packages in InstallOrder sort." << endl;
    }

}


    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
