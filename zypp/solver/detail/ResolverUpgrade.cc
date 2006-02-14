/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/* ResolverUpgrade.cc
 *
 * Implements the distribution upgrade algorithm.
 *
 * Copyright (C) 2005 SUSE Linux Products GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA.
 */

/*
  stolen from PMPackageManager_update.cc
  original author Michael Andres <ma@suse.de>
  zypp port by Klaus Kaempf <kkaempf@suse.de>

/-*/

#include "zypp/CapSet.h"
#include "zypp/capability/SplitCap.h"

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Exception.h"

#include "zypp/base/Algorithm.h"
#include "zypp/ResPool.h"
#include "zypp/ResStatus.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"
#include "zypp/Capability.h"
#include "zypp/CapFactory.h"
#include "zypp/VendorAttr.h"
#include "zypp/Package.h"

#include "zypp/capability/CapabilityImpl.h"
#include "zypp/ZYppFactory.h"

#include "zypp/solver/detail/Types.h"
#include "zypp/solver/detail/Helper.h"
#include "zypp/solver/detail/Resolver.h"

#include "zypp/Target.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

using namespace std;
using namespace zypp;
using zypp::capability::SplitCap;


// check if downgrade is allowed
//
// both must have allowed vendor (e.g. 'SuSE', 'Novell', ...) and candidates buildtime must be
// newer.

static bool
downgrade_allowed (PoolItem_Ref installed, PoolItem_Ref candidate)
{
    if ( installed->edition().compare (candidate->edition()) > 0 )
	return false; // candidate is newer

    static VendorAttr *va = VendorAttr::vendorAttr();

    Resolvable::constPtr ires = installed.resolvable();
    Package::constPtr ipkg = asKind<Package>(ires);
    Resolvable::constPtr cres = candidate.resolvable();
    Package::constPtr cpkg = asKind<Package>(cres);

    if ( va->isKnown( ipkg->vendor() )
	 && va->isKnown( cpkg->vendor() ) )
    {
#warning Had Y2PM::runningFromSystem
	return( ipkg->buildtime() >= cpkg->buildtime() );
    }
    return false;
}



struct FindObsoletes : public resfilter::OnCapMatchCallbackFunctor
{
    bool obsoletes;

    FindObsoletes ()
	: obsoletes (false)
    { }

    bool operator()( PoolItem_Ref provider, const Capability & match )
    {
	obsoletes = true;				// we have a match
	return false;					// stop looping here
    }
};


// does the candidate obsolete the capability ?

bool
Resolver::doesObsoleteCapability (PoolItem_Ref candidate, const Capability & cap)
{
    _DEBUG("doesObsoleteCapability " << candidate << ", " << cap);

    Dep dep (Dep::OBSOLETES);
    FindObsoletes info;
    invokeOnEach( _pool.byCapabilityIndexBegin( cap.index(), dep ),
		  _pool.byCapabilityIndexEnd( cap.index(), dep ),
		  resfilter::callOnCapMatchIn( dep, cap, functor::functorRef<bool,PoolItem_Ref,Capability>(info) ) );

    _DEBUG((info.obsoletes ? "YES" : "NO"));
    return info.obsoletes;
}


bool
Resolver::doesObsoleteItem (PoolItem_Ref candidate, PoolItem_Ref installed)
{
    CapFactory factory;
    Capability installedCap =  factory.parse ( installed->kind(), installed->name(), Rel::EQ, installed->edition());

    return doesObsoleteCapability (candidate, installedCap);
}


//-----------------------------------------------------------------------------


// find all available providers for installed name

struct FindProviders : public resfilter::OnCapMatchCallbackFunctor
{
    PoolItemSet providers;		// the providers which matched

    FindProviders ()
    { }

    bool operator()( PoolItem_Ref provider, const Capability & match )
    {
	if ( provider.status().isToBeUninstalled() ) {
	    MIL << "  IGNORE relation match (package is tagged to delete): " << match << " ==> " << provider << endl;
	}
	else {
	    MIL << "  relation match: " << match << " ==> " << provider << endl;
	    providers.insert (provider);
	}
	return true;
    }
};


//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : Resolver::doUpgrade
//	METHOD TYPE : int
//
//	DESCRIPTION : go through all installed (but not yet touched by user)
//		packages and look for update candidates
//		handle splitprovides and replaced and dropped
//
void
Resolver::doUpgrade( UpgradeStatistics & opt_stats_r )
{
  typedef map<PoolItem_Ref,PoolItem_Ref> CandidateMap;

  typedef intrusive_ptr<const SplitCap> SplitCapPtr;
  typedef map<PoolItem_Ref,PoolItemSet> SplitMap;
  typedef map<PoolItem_Ref,PoolItemSet> TodoMap;

  CandidateMap candidatemap;

  PoolItemList _update_items;

  SplitMap    splitmap;
  TodoMap     applyingSplits;
  TodoMap     addSplitted;
  TodoMap     addProvided;
  TodoMap     addMultiProvided;

  Target_Ptr target;
  try {
	ZYppFactory zf;
	target = zf.getZYpp()->target();
  }
  catch( const Exception & excpt_r) {
	ERR << "Huh, no target ?";
	ZYPP_CAUGHT(excpt_r);
  }
MIL << "target at " << target << endl;

  MIL << "doUpgrade start... "
    << "(delete_unmaintained:" << (opt_stats_r.delete_unmaintained?"yes":"no") << ")"
    << endl;

  _update_items.clear();
  {
    UpgradeOptions opts( opt_stats_r );
    opt_stats_r = UpgradeStatistics();
    (UpgradeOptions&)opt_stats_r = opts;
  }

  ///////////////////////////////////////////////////////////////////
  // Reset all auto states and build PoolItemSet of available candidates
  // (those that do not belong to PoolItems set to delete).
  //
  // On the fly remember splitprovides and afterwards check, which
  // of them do apply.
  ///////////////////////////////////////////////////////////////////
  PoolItemSet available; // candidates available for install (no matter if selected for install or not)

  for ( ResPool::const_iterator it = _pool.begin(); it != _pool.end(); ++it ) {
    PoolItem_Ref item = *it;
    PoolItem_Ref candidate;
    PoolItem_Ref installed;

    if ( item.status().isToBeUninstalled() ) {
      MIL << "doUpgrade available: SKIP to delete " << item << endl;
      ++opt_stats_r.pre_todel;
      continue;
    }
    if ( item.status().staysInstalled() ) {
      installed = item;
      CandidateMap::const_iterator cand_it = candidatemap.find(installed);
      if (cand_it != candidatemap.end()) {
	candidate = cand_it->second;
      }
      else {
	candidate = Helper::findUpdateItem( _pool, installed);
      }
      if (!candidate) {
	MIL << "doUpgrade available: SKIP no candidate for " << installed << endl;
	++opt_stats_r.pre_nocand;
	continue;
      }
MIL << "item " << item << " is installed, candidate is " << candidate << endl;
      if (candidate.status().isSeen()) {			// seen already
	candidate.status().setSeen(true);
	continue;
      }
      candidate.status().setSeen(true);				// mark as seen
      candidatemap[installed] = candidate;
    }
    else {					// assume Uninstalled
      if (item.status().isSeen()) {				// seen already
	item.status().setSeen(true);
	continue;
      }
      candidate = item;
      candidate.status().setSeen(true);				// mark as seen
      installed = Helper::findInstalledItem (_pool, candidate);
      if (installed) {						// check if we already have an installed
MIL << "found installed " << installed << " for item " << candidate << endl;
	CandidateMap::const_iterator cand_it = candidatemap.find(installed);
	if (cand_it == candidatemap.end()					// not in map yet
	    || cand_it->second->edition().compare (candidate->edition()) < 0)	// or the new is better!
	{
	    candidatemap[installed] = candidate;				// put it in !
	}
      }
    }

#warning FIXME needs locks
#if 0
    if ( item->isLocked() ) {
      MIL << "doUpgrade available: SKIP taboo candidate " << item << endl;
      ++opt_stats_r.pre_nocand;
      continue;
    }
#endif

    ++opt_stats_r.pre_avcand;
#warning this should add the best candidate
    available.insert( candidate );

MIL << "installed " << installed << ", candidate " << candidate << endl;

    // remember any splitprovides to packages actually installed.
    CapSet caps = candidate->dep (Dep::PROVIDES);
    for (CapSet::iterator cit = caps.begin(); cit != caps.end(); ++cit ) {
	if (isKind<capability::SplitCap>( *cit ) ) {

	    capability::CapabilityImpl::SplitInfo splitinfo = capability::CapabilityImpl::getSplitInfo( *cit );

	    PoolItem splititem = Helper::findInstalledByNameAndKind (_pool, splitinfo.name, ResTraits<zypp::Package>::kind);
MIL << "has split cap " << splitinfo.name << ":" << splitinfo.path << ", splititem:" << splititem << endl;
	    if (splititem) {
		if (target) {
		    ResObject::constPtr robj = target->whoOwnsFile( splitinfo.path );
MIL << "whoOwnsFile(): " << robj << endl;
		    if (robj
			&& robj->name() == splitinfo.name)
		    {
MIL << "split matched !" << endl;
			splitmap[splititem].insert( candidate );
		    }
		}
	    }
	}
    }

  } // iterate over the complete pool

  // reset all seen
  for (PoolItemSet::const_iterator it = available.begin(); it != available.end(); ++it) {
	it->status().setSeen(false);
  }

#warning Cant update from broken install medium like STABLE
#if 0
  // filter packages with requires that are not fulfilled by other candidates,
  // to reduce errors a bit when trying to update from a broken installation
  // medium (ie. STABLE)
  {
    CheckSetDeps::BrokenMap broken;
    CheckSetDeps checker(available, broken);

    checker.setTrackRelations(false);
    checker.checkAll();

    if(!broken.empty())
    {
      CheckSetDeps::BrokenMap::iterator bit, bend;
      for(bit = broken.begin(), bend = broken.end(); bit != bend; ++bit)
      {
	MIL << bit->first->name() << " is broken, not considering it for update" << endl;
	available.remove(bit->first);
	--opt_stats_r.pre_avcand;
	++opt_stats_r.pre_nocand;
      }
    }
  }
#endif

  MIL << "doUpgrade: " << opt_stats_r.pre_todel  << " packages tagged to delete" << endl;
  MIL << "doUpgrade: " << opt_stats_r.pre_nocand << " packages without candidate (foreign, replaced or dropped)" << endl;
  MIL << "doUpgrade: " << opt_stats_r.pre_avcand << " packages available for update" << endl;

  MIL << "doUpgrade: going to check " << splitmap.size() << " probably splitted packages" << endl;
  {
    ///////////////////////////////////////////////////////////////////
    // splitmap entries are gouped by PoolItems (we know this). So get the
    // filelist as a new PoolItem occures, and use it for consecutive entries.
    //
    // On the fly build SplitPkgMap from splits that do apply (i.e. file is
    // in PoolItems's filelist). The way splitmap was created, candidates added
    // are not initially tagged to delete!
    ///////////////////////////////////////////////////////////////////

    PoolItem_Ref citem;

    for ( SplitMap::iterator it = splitmap.begin(); it != splitmap.end(); ++it ) {
	applyingSplits[it->first].insert( it->second.begin(), it->second.end() );
	_DEBUG("  split count for " << it->first->name() << " now " << applyingSplits[it->first].size());
    }
    splitmap.clear();
  }

  ///////////////////////////////////////////////////////////////////
  // Now iterate installed packages, not selected to delete, and
  // figure out what might be an appropriate replacement. Current
  // packages state is changed immediately. Additional packages are
  // reported but set to install later.
  ///////////////////////////////////////////////////////////////////
  MIL << "doUpgrade pass 1..." << endl;

  for ( ResPool::const_iterator it = _pool.begin(); it != _pool.end(); ++it ) {

    PoolItem_Ref installed(*it);
    ResStatus status (installed.status());

    if ( ! status.staysInstalled() ) {
      continue;
    }
    ++opt_stats_r.chk_installed_total;

    if ( status.transacts() ) {						// we know its installed, if it transacts also
      MIL << "SKIP to delete: " << it->resolvable() << endl;	// it'll be deleted
      ++opt_stats_r.chk_already_todel;
      continue;
    }

#warning This needs locks
#if 0
    if ( (*it)->is_taboo() ) {
      MIL << "SKIP taboo: " << (*it)->installedObj() << endl;
      ++opt_stats_r.chk_is_taboo;
      _update_items.push_back ( *it ); // remember in problem list ?
      continue;
    }
#endif

    CandidateMap::iterator cand_it = candidatemap.find(installed);

    bool probably_dropped = false;

    MIL << "REPLACEMENT FOR " << installed << endl;
    ///////////////////////////////////////////////////////////////////
    // figure out replacement
    ///////////////////////////////////////////////////////////////////
    if ( cand_it != candidatemap.end() ) {

      PoolItem_Ref candidate (cand_it->second);

      if ( ! candidate.status().isToBeInstalled() ) {

	if ( installed->edition().compare (candidate->edition()) < 0 ) {	  // new version
	  candidate.status().setToBeInstalled(ResStatus::APPL_HIGH);
	  MIL << " ==> INSTALL (new version): " << candidate << endl;
	  ++opt_stats_r.chk_to_update;
	} else {
	  // check whether to downgrade:

	  if (!downgrade_allowed (installed, candidate)) {
	    MIL << " ==> (keep installed)" << candidate << endl;
	    ++opt_stats_r.chk_to_keep_installed;
	  } else {
	    candidate.status().setToBeInstalled(ResStatus::APPL_HIGH);
	    MIL << " ==> INSTALL (SuSE version downgrade): " << candidate << endl;
	    ++opt_stats_r.chk_to_downgrade;
	  }
	}
      } else {
	MIL << " ==> INSTALL (preselected): " << candidate << endl;
	++opt_stats_r.chk_already_toins;
      }

    }
    else {		// no candidate

      // replaced or dropped (anyway there's no candidate for this!)
      // If unique provides exists check if obsoleted (replaced).
      // Remember new package for 2nd pass.

      Dep dep (Dep::PROVIDES);
      CapFactory factory;
      Capability installedCap =  factory.parse ( installed->kind(), installed->name(), Rel::EQ, installed->edition());

      FindProviders info;
#if 0
      invokeOnEach( _pool.byCapabilityIndexBegin( installed->name(), dep ),
		    _pool.byCapabilityIndexEnd( installed->name(), dep ),
		    resfilter::ByUninstalled (),
		    resfilter::callOnCapMatchIn( dep, installedCap, functor::functorRef<bool,PoolItem,Capability>(info) ) );
#endif		
	ResPool::const_indexiterator pend = pool().providesend(installed->name());
	for (ResPool::const_indexiterator it = pool().providesbegin(installed->name()); it != pend; ++it) {
	    if (it->second.second.status().staysUninstalled()
		&& installedCap.matches (it->second.first) == CapMatch::yes) {
		if (!info( it->second.second, it->second.first))
		    break;
	    }
	}

      int num_providers = info.providers.size();

      _DEBUG("lookup " << num_providers << " provides for installed " << installedCap);

      switch ( info.providers.size() ) {
      case 0:
	MIL << " ==> (dropped)" << endl;
	// wait untill splits are processed. Might be a split obsoletes
	// this one (i.e. package replaced but not provided by new one).
	// otherwise it's finaly dropped.
	probably_dropped = true;
	break;
      case 1:
        addProvided[installed] = info.providers;
	MIL << " ==> REPLACED by: " << (*info.providers.begin()) << endl;
	// count stats later
	// check obsoletes later
	break;
      default:
	addMultiProvided[installed] = info.providers;
	MIL << " ==> pass 2 (" << info.providers.size() << " times provided)" << endl;
	// count stats later
	// check obsoletes later
	break;
      }

    }	// no candidate

    ///////////////////////////////////////////////////////////////////
    // anyway check for packages split off
    ///////////////////////////////////////////////////////////////////

    TodoMap::iterator sit = applyingSplits.find( installed );
    if ( sit != applyingSplits.end() ) {
      PoolItemSet & toadd( sit->second );
      if ( !toadd.size() ) {
	INT << "Empty SplitPkgMap entry for " << installed << endl;
      } else {
	for ( PoolItemSet::iterator ait = toadd.begin(); ait != toadd.end(); ++ait ) {
	  PoolItem_Ref split_candidate = *ait;
	  MIL << " ==> ADD (splitted): " << split_candidate << endl;
	  if ( probably_dropped
	       && split_candidate.status().staysUninstalled()
	       && doesObsoleteItem (split_candidate, installed))
	  {
	    probably_dropped = false;
	  }
	}
	addSplitted[installed] = toadd;
      }
      // count stats later
    }

    ///////////////////////////////////////////////////////////////////
    // now handle dropped package
    ///////////////////////////////////////////////////////////////////

    if ( probably_dropped ) {
      if ( opt_stats_r.delete_unmaintained ) {
	installed.status().setToBeUninstalled(ResStatus::APPL_HIGH);
      }
      ++opt_stats_r.chk_dropped;
      _update_items.push_back ( installed );
    }

  } // pass 1 end

  ///////////////////////////////////////////////////////////////////
  // Now check the remembered packages and check non unique provided.
  // Maybe one of them was somehow selected. Otherwise we have to guess
  // one.
  ///////////////////////////////////////////////////////////////////
  MIL << "doUpgrade pass 2..." << endl;

  // look at the ones with a single provide first

  for ( TodoMap::iterator it = addProvided.begin(); it != addProvided.end(); ++it ) {

    PoolItemSet & tset( it->second );		// these are the providers (well, just one)

    for ( PoolItemSet::iterator sit = tset.begin(); sit != tset.end(); ++sit ) {
      PoolItem_Ref provider (*sit);

      if (provider.status().setToBeInstalled(ResStatus::APPL_HIGH)) {
	++opt_stats_r.chk_replaced;
      }

      // needs installed

      if ( doesObsoleteItem (provider, it->first ) ) {
	it->first.status().setToBeUninstalled(ResStatus::APPL_HIGH);
      }
    }

  }

  // look at the split providers

  for ( TodoMap::iterator it = addSplitted.begin(); it != addSplitted.end(); ++it ) {

    PoolItemSet & tset( it->second );
    for ( PoolItemSet::iterator sit = tset.begin(); sit != tset.end(); ++sit ) {
      if ((*sit).status().setToBeInstalled(ResStatus::APPL_HIGH)) {
	++opt_stats_r.chk_add_split;
      }
    }

  }

  // look at the ones with multiple providers

  for ( TodoMap::iterator it = addMultiProvided.begin(); it != addMultiProvided.end(); ++it ) {
    MIL << "GET ONE OUT OF " << it->second.size() << " for " << it->first << endl;

    PoolItem_Ref guess;
    PoolItemSet & gset( it->second );
    for ( PoolItemSet::iterator git = gset.begin(); git != gset.end(); ++git ) {
      PoolItem_Ref item (*git);
      if ( item.status().isToBeInstalled()) {
	MIL << " ==> (pass 2: meanwhile set to install): " << item << endl;
	if ( ! doesObsoleteItem (item, it->first ) ) {
	  it->first.status().setToBeUninstalled(ResStatus::APPL_HIGH);
	}
	guess = PoolItem_Ref();
	break;
      } else {
	// Be prepared to guess.
	// Most common situation for guessing is something like:
	//   qt-devel
	//   qt-devel-experimental
	//   qt-devel-japanese
	// That's why currently the shortest package name wins.
	if ( !guess || guess->name().size() > item->name().size() ) {
	  guess = item;
	}
      }
    }

    if ( guess ) {
      guess.status().setToBeInstalled(ResStatus::APPL_HIGH);
      MIL << " ==> REPLACED by: (pass 2: guessed): " << guess << endl;
      if ( ! doesObsoleteItem (guess, it->first ) ) {
	it->first.status().setToBeUninstalled(ResStatus::APPL_HIGH);
      }
      ++opt_stats_r.chk_replaced_guessed;
    }
  }

  ///////////////////////////////////////////////////////////////////
  // done
  ///////////////////////////////////////////////////////////////////
  MIL << opt_stats_r << endl;
}

///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////


