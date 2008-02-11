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

#include "zypp/Capabilities.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Exception.h"
#include "zypp/VendorAttr.h"
#include "zypp/base/Algorithm.h"
#include "zypp/ResPool.h"
#include "zypp/ResStatus.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"
#include "zypp/Capability.h"
#include "zypp/VendorAttr.h"
#include "zypp/Package.h"
#include "zypp/ZYppFactory.h"
#include "zypp/solver/detail/Types.h"
#include "zypp/solver/detail/Helper.h"
#include "zypp/solver/detail/Resolver.h"
#include "zypp/solver/detail/Testcase.h"
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

/** Order on AvialableItemSet.
 * \li best Arch
 * \li best Edition
 * \li ResObject::constPtr as fallback.
*/
struct AVOrder : public std::binary_function<PoolItem,PoolItem,bool>
{
    // NOTE: operator() provides LESS semantics to order the set.
    // So LESS means 'prior in set'. We want 'better' archs and
    // 'better' editions at the beginning of the set. So we return
    // TRUE if (lhs > rhs)!
    //
    bool operator()( const PoolItem lhs, const PoolItem rhs ) const
        {
	    int res = lhs->arch().compare( rhs->arch() );
	    if ( res )
		return res > 0;
	    res = lhs->edition().compare( rhs->edition() );
	    if ( res )
		return res > 0;

	    // no more criteria, still equal:
	    // use the ResObject::constPtr (the poiner value)
	    // (here it's arbitrary whether < or > )
	    return lhs.resolvable() < rhs.resolvable();
        }
};

typedef std::set<PoolItem, AVOrder> PoolItemOrderSet;



// check if downgrade is allowed
// (Invariant on entry: installed.edition >= candidate.edition)
//
// candidate must have allowed vendor (e.g. 'SuSE', 'Novell', ...) and candidates buildtime must be
// newer.

static bool
downgrade_allowed( PoolItem installed, PoolItem candidate, bool silent_downgrades )
{
    if (installed.status().isLocked()) {
	MIL << "Installed " << installed << " is locked, not upgrading" << endl;
	return false;
    }

    Resolvable::constPtr ires = installed.resolvable();
    Package::constPtr ipkg = asKind<Package>(ires);
    Resolvable::constPtr cres = candidate.resolvable();
    Package::constPtr cpkg = asKind<Package>(cres);

    if (ipkg)
      DBG << "Installed vendor '" << ipkg->vendor() << "'" << endl;
    if (cpkg)
      DBG << "Candidate vendor '" << cpkg->vendor() << "'" << endl;

    if (cpkg
	&& VendorAttr::instance().equivalent( ipkg->vendor(), cpkg->vendor() ) )
    {
	if ( silent_downgrades )
	    return true;
	if ( ipkg->buildtime() < cpkg->buildtime() ) {			// installed has older buildtime
	    MIL << "allowed downgrade " << installed << " to " << candidate << endl;
	    return true;						// see bug #152760
	}
    }
    return false;
}



struct FindObsoletes
{
    bool obsoletes;

    FindObsoletes ()
	: obsoletes (false)
    { }

    bool operator()( const CapAndItem & cai )
    {
	obsoletes = true;				// we have a match
	return false;					// stop looping here
    }
};


// does the candidate obsolete the capability ?

bool
Resolver::doesObsoleteCapability (PoolItem candidate, const Capability & cap)
{
    _DEBUG("doesObsoleteCapability " << candidate << ", " << cap);

    Dep dep (Dep::OBSOLETES);
    FindObsoletes info;
    invokeOnEach( _pool.byCapabilityIndexBegin( cap.index(), dep ),
		  _pool.byCapabilityIndexEnd( cap.index(), dep ),
		  resfilter::ByCapMatch( cap ),
		  functor::functorRef<bool,CapAndItem>(info) );

    _DEBUG((info.obsoletes ? "YES" : "NO"));
    return info.obsoletes;
}


bool
Resolver::doesObsoleteItem (PoolItem candidate, PoolItem installed)
{
    Capability installedCap( installed->name(), Rel::EQ, installed->edition(), installed->kind());
    return doesObsoleteCapability (candidate, installedCap);
}

//-----------------------------------------------------------------------------

// find best available providers for installed name

typedef map<string, PoolItem> FindMap;

struct FindProviders
{
    FindMap providers;		// the best providers which matched
    PoolItem forItem;
    bool otherVendorFound;
    FindProviders (PoolItem item)
	:forItem(item),
	 otherVendorFound(false)
    { }

    bool operator()( const CapAndItem & cai )
    {
	PoolItem provider( cai.item );
	if ( !VendorAttr::instance().equivalent(provider->vendor(), forItem->vendor()) )
	{
	    MIL << "Discarding '" << provider << "' from vendor '"
		<< provider->vendor() << "' different to uninstalled '"
		<< forItem->vendor() << "' vendor." << endl;
	    otherVendorFound = true;
	} else if ( provider.status().isToBeUninstalled() ) {
	    MIL << "  IGNORE relation match (package is tagged to delete): " << cai.cap << " ==> " << provider << endl;
	}
	else {
	    FindMap::iterator it = providers.find( provider->name() );

	    if (it != providers.end()) {				// provider with same name found
		if (provider.status().isToBeInstalled()
		    || it->second.status().isToBeInstalled()) {

		    if (provider.status().isToBeInstalled()
			&& it->second.status().isToBeInstalled()) {
			ERR << "only one should be set for installation: " << it->second << "; " << provider << endl;
		    } else {
			if (provider.status().isToBeInstalled()) {
			    it->second = provider; // take thatone which is already set for installation
			}
		    }
		} else {
		    // not the same --> find better provider
		    int cmp = it->second->arch().compare( provider->arch() );
		    if (cmp < 0) {						// new provider has better arch
			it->second = provider;
		    }
		    else if (cmp == 0) {					// new provider has equal arch
			if (it->second->edition().compare( provider->edition() ) < 0) {
			    it->second = provider;				// new provider has better edition
			}
		    }
		}
	    }
	    else {
		providers[provider->name()] = provider;
	    }
	}
	return true;
    }
};


//-----------------------------------------------------------------------------

// Selecting item for installation

class LookForSelected : public resfilter::PoolItemFilterFunctor
{
  public:
    bool found;
    PoolItem candidate;
    
    LookForSelected (PoolItem can)
	: found (false),
	candidate (can)
    { }

    bool operator()( PoolItem item )
    {
	if (item.status().isToBeInstalled()
	    && item->edition() == candidate->edition()
	    && item->arch() == candidate->arch()) {
	    MIL << item << " is already selected for installation --> ignoring" << endl;	    
	    found = true;
	    return false; // stop here
	}
	return true;
    }
};

bool setForInstallation (const ResPool &pool, PoolItem item) {
    LookForSelected info(item);

    invokeOnEach( pool.byIdentBegin (item->kind(),item->name()),
		  pool.byIdentEnd (item->kind(),item->name()),
		  resfilter::ByUninstalled (),			// ByUninstalled
		  functor::functorRef<bool,PoolItem> (info) );
    if (info.found) {
	MIL << "   ---> " << item << " will be ignoring" << endl;
	return true;
    } else {
	return item.status().setToBeInstalled( ResStatus::APPL_HIGH );
    }
}	

//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : Resolver::doUpgrade
//	METHOD TYPE : int
//
//	DESCRIPTION : go through all installed (but not yet touched by user)
//		packages and look for update candidates
//
void
Resolver::doUpgrade( UpgradeStatistics & opt_stats_r )
{
  typedef map<PoolItem,PoolItem> CandidateMap;
  typedef map<PoolItem,PoolItemOrderSet> TodoMap;

  CandidateMap candidatemap;

  TodoMap     addProvided;
  TodoMap     addMultiProvided;

  Target_Ptr target;
  try {
	target = getZYpp()->target();
  }
  catch( const Exception & excpt_r) {
	ERR << "Huh, no target ?";
	ZYPP_CAUGHT(excpt_r);
	if (!_testing) return;		// can't continue without target
	MIL << "Running in test mode, continuing without target" << endl;
  }
  MIL << "target at " << target << endl;

  MIL << "doUpgrade start... "
    << "(delete_unmaintained:" << (opt_stats_r.delete_unmaintained?"yes":"no") << ")"
    << "(silent_downgrades:" << (opt_stats_r.silent_downgrades?"yes":"no") << ")"
    << "(keep_installed_patches:" << (opt_stats_r.keep_installed_patches?"yes":"no") << ")"
    << endl;

  // create a testcase for the updating system
  Testcase testcase("/var/log/updateTestcase");
  testcase.createTestcase (*this, true, false); // create pool, do not solver  

  _update_items.clear();
  {
    UpgradeOptions opts( opt_stats_r );
    opt_stats_r = UpgradeStatistics();
    (UpgradeOptions&)opt_stats_r = opts;
  }

  ///////////////////////////////////////////////////////////////////
  // Reset all auto states and build PoolItemOrderSet of available candidates
  // (those that do not belong to PoolItems set to delete).
  //
  ///////////////////////////////////////////////////////////////////
  PoolItemOrderSet available; // candidates available for install (no matter if selected for install or not)

  for ( ResPool::const_iterator it = _pool.begin(); it != _pool.end(); ++it ) {
    PoolItem item = *it;
    PoolItem candidate;
    PoolItem installed;

    if ( item.status().isToBeUninstalled() ) {
      MIL << "doUpgrade available: SKIP to delete " << item << endl;
      ++opt_stats_r.pre_todel;
      continue;
    }
    if ( item.status().isLocked() ) {
      MIL << "doUpgrade available: SKIP locked " << item << endl;
      if ( item.status().staysInstalled() ) {
	++opt_stats_r.pre_nocand;
      }
      continue;
    }

    if ( item.status().staysInstalled() ) {	// installed item
      installed = item;
      CandidateMap::const_iterator cand_it = candidatemap.find( installed );
      if (cand_it != candidatemap.end()) {
	candidate = cand_it->second;				// found candidate already
      }
      else {
	candidate = Helper::findUpdateItem( _pool, installed );	// find 'best' upgrade candidate
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
      installed = Helper::findInstalledItem( _pool, candidate );
      if (installed) {						// check if we already have an installed
	if ( installed.status().isLocked() ) {
	  MIL << "doUpgrade available: SKIP candidate " << candidate << ", locked " << installed << endl;
	  continue;
	}

	if ( !VendorAttr::instance().equivalent(installed->vendor(), candidate->vendor()) )
	{
	    MIL << "Discarding '" << candidate << "' from vendor '"
		<< candidate->vendor() << "' different to uninstalled '"
		<< installed->vendor() << "' vendor." << endl;
	    continue;
	}

        MIL << "found installed " << installed << " for item " << candidate << endl;
	CandidateMap::const_iterator cand_it = candidatemap.find( installed );
	if (cand_it == candidatemap.end()						// not in map yet
	    || (cand_it->second->arch().compare( candidate->arch() ) < 0)		// or the new has better architecture
	    || ((cand_it->second->arch().compare( candidate->arch() ) == 0)		// or the new has the same architecture
		&& (cand_it->second->edition().compare( candidate->edition() ) < 0)) 	//   and a better edition (-> 157501)
	    )
	{
	    candidatemap[installed] = candidate;				// put it in !
	}
      }
    }

    ++opt_stats_r.pre_avcand;
    available.insert( candidate );

    MIL << "installed " << installed << ", candidate " << candidate << endl;
  } // iterate over the complete pool

  // reset all seen (for next run)
  for ( ResPool::const_iterator it = _pool.begin(); it != _pool.end(); ++it ) {
	it->status().setSeen( false );
  }

  MIL << "doUpgrade: " << opt_stats_r.pre_todel  << " packages tagged to delete" << endl;
  MIL << "doUpgrade: " << opt_stats_r.pre_nocand << " packages without candidate (foreign, replaced or dropped)" << endl;
  MIL << "doUpgrade: " << opt_stats_r.pre_avcand << " packages available for update" << endl;

  ///////////////////////////////////////////////////////////////////
  // Now iterate installed packages, not selected to delete, and
  // figure out what might be an appropriate replacement. Current
  // packages state is changed immediately. Additional packages are
  // reported but set to install later.
  ///////////////////////////////////////////////////////////////////
  MIL << "doUpgrade pass 1..." << endl;

  for ( ResPool::const_iterator it = _pool.begin(); it != _pool.end(); ++it ) {

    PoolItem installed(*it);
    ResStatus status (installed.status());

    if ( ! status.staysInstalled() ) {
      continue;
    }
    ++opt_stats_r.chk_installed_total;

    if ( status.transacts() ) {					// we know its installed, if it transacts also
      MIL << "SKIP to delete: " << installed.resolvable() << endl;	// it'll be deleted
      ++opt_stats_r.chk_already_todel;
      continue;
    }

    if ( installed.status().isLocked() ) {			// skip locked
      MIL << "SKIP taboo: " << installed << endl;
      ++opt_stats_r.chk_is_taboo;
      _update_items.push_back( installed );			// remember in problem list
      continue;
    }

    if ( isKind<Patch>(installed.resolvable())
         || isKind<Atom>(installed.resolvable())
         || isKind<Script>(installed.resolvable())
         || isKind<Message>(installed.resolvable()) )
      {
        if ( ! opt_stats_r.keep_installed_patches )
          {
            if ( isKind<Patch>(installed.resolvable()) )
              MIL << "OUTDATED Patch: " << installed << endl;
            installed.status().setToBeUninstalled( ResStatus::APPL_HIGH );
          }
        else
          {
            if ( isKind<Patch>(installed.resolvable()) )
              MIL << "SKIP Patch: " << installed << endl;
          }
        continue;
      }

    CandidateMap::iterator cand_it = candidatemap.find( installed );

    bool probably_dropped = false;

    MIL << "REPLACEMENT FOR " << installed << endl;
    ///////////////////////////////////////////////////////////////////
    // figure out replacement
    ///////////////////////////////////////////////////////////////////
    if ( cand_it != candidatemap.end() ) {

      PoolItem candidate (cand_it->second);

      if ( ! candidate.status().isToBeInstalled() ) {
	int cmp = installed->edition().compare( candidate->edition() );
	if ( cmp < 0 ) {   // new edition
	  setForInstallation (_pool,candidate);
	  MIL << " ==> INSTALL (new version): " << candidate << endl;
	  ++opt_stats_r.chk_to_update;
	} else {							// older or equal edition
	  // check whether to downgrade:

	  if (cmp == 0							// equal
	      || !downgrade_allowed( installed, candidate,
                                     opt_stats_r.silent_downgrades) )	//  or downgrade not allowed
	  {
	    MIL << " ==> (keep installed)" << candidate << endl;	// keep installed
	    ++opt_stats_r.chk_to_keep_installed;
	  } else {// older and downgrade allowed
	    setForInstallation (_pool, candidate);
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
      Capability installedCap( installed->name(), Rel::EQ, installed->edition(), installed->kind());      

      FindProviders info(installed);

      invokeOnEach( _pool.byCapabilityIndexBegin( installed->name(), dep ),
		    _pool.byCapabilityIndexEnd( installed->name(), dep ),
		    functor::chain( resfilter::ByCaIUninstalled(),
				    resfilter::ByCapMatch( installedCap ) ) ,
		    functor::functorRef<bool,CapAndItem>(info) );

      int num_providers = info.providers.size();

      _DEBUG("lookup " << num_providers << " provides for installed " << installedCap);

      // copy from map to set
      PoolItemOrderSet providers;
      for (FindMap::const_iterator mapit = info.providers.begin(); mapit != info.providers.end(); ++mapit) {
	providers.insert( mapit->second );
      }

      switch ( info.providers.size() ) {
      case 0:
	  if (info.otherVendorFound) {
	      MIL << " only resolvable with other vendor found ==> do nothing" << endl;
	  } else {
	      MIL << " ==> (dropped)" << endl;
	      probably_dropped = true;
	  }
	break;
      case 1:
        addProvided[installed] = providers;
	MIL << " ==> REPLACED by: " << (*providers.begin()) << endl;
	// count stats later
	// check obsoletes later
	break;
      default:
	addMultiProvided[installed] = providers;
	MIL << " ==> pass 2 (" << providers.size() << " times provided)" << endl;
	// count stats later
	// check obsoletes later
	break;
      }

    }	// no candidate


    ///////////////////////////////////////////////////////////////////
    // now handle dropped package
    ///////////////////////////////////////////////////////////////////

    if ( probably_dropped ) {
      if ( opt_stats_r.delete_unmaintained
           && VendorAttr::instance().equivalent( installed->vendor(), "suse" ) ) {
	installed.status().setToBeUninstalled( ResStatus::APPL_HIGH );
      }
      ++opt_stats_r.chk_dropped;
      _update_items.push_back( installed );
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

    PoolItemOrderSet & tset( it->second );		// these are the providers (well, just one)

    for ( PoolItemOrderSet::iterator sit = tset.begin(); sit != tset.end(); ++sit ) {
      PoolItem provider (*sit);

      if (setForInstallation (_pool, provider)) {
	++opt_stats_r.chk_replaced;
      }

      // needs installed

      if ( doesObsoleteItem (provider, it->first ) ) {
	it->first.status().setToBeUninstalled( ResStatus::APPL_HIGH );
      }
    }

  }

  // look at the ones with multiple providers

  for ( TodoMap::iterator it = addMultiProvided.begin(); it != addMultiProvided.end(); ++it ) {
    MIL << "GET ONE OUT OF " << it->second.size() << " for " << it->first << endl;

    PoolItem guess;
    PoolItemOrderSet & gset( it->second );

    for ( PoolItemOrderSet::iterator git = gset.begin(); git != gset.end(); ++git ) {
	PoolItem item (*git);

	if (git == gset.begin())		// default to first of set; the set is ordered, first is the best
	    guess = item;

	if ( item.status().isToBeInstalled()) {
	    MIL << " ==> (pass 2: meanwhile set to install): " << item << endl;
	    if ( ! doesObsoleteItem (item, it->first ) ) {
		it->first.status().setToBeUninstalled( ResStatus::APPL_HIGH );
	    }
	    guess = PoolItem();
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
	// Checking if the selected provider depends on language, if yes try to find out the
	// correct language package
	bool requested_locale_match = false;
	Capabilities freshens( guess->dep( Dep::FRESHENS ) );

	// is this a language package ?
	for (Capabilities::const_iterator cit = freshens.begin(); cit != freshens.end(); ++cit) {
	    string citName = cit->asString();
	    if (citName.length() > 7 &&  citName.compare(0, 7, "locale(") == 0) { // is a language dependency
		requested_locale_match = true;
		break;
	    }
	}

	if (requested_locale_match) {
	    // searching the best language
	    PoolItemOrderSet & gset( it->second );
	    requested_locale_match = false;

	    for ( PoolItemOrderSet::iterator git = gset.begin(); git != gset.end(); ++git ) {
		PoolItem item (*git);

		if ( item.status().isToBeInstalled()) {
		    MIL << " ==> (pass 2: meanwhile set to install): " << item << endl;
		    if ( ! doesObsoleteItem (item, it->first ) ) {
			it->first.status().setToBeUninstalled( ResStatus::APPL_HIGH );
		    }
		    guess = PoolItem();
		    break;
		} else {
		    freshens = item->dep( Dep::FRESHENS );
		    ZYpp::Ptr z = zypp::getZYpp();
		    ZYpp::LocaleSet requested_locales = z->getRequestedLocales();

		    // try to find a match of the locale freshens with one of the requested locales

		    for (Capabilities::const_iterator cit = freshens.begin(); cit != freshens.end(); ++cit) {
			string citName = cit->asString();
			if (citName.length() > 7 &&  citName.compare(0, 7, "locale(") == 0) { // is a language dependency
			    string loc = cit->index();
			    MIL << "Look for language fallback " << loc << ":" << item << endl;
			    if (requested_locales.find( Locale( loc ) ) != requested_locales.end()) {
				MIL << "Locale '" << loc << "' is requested" << endl;
				requested_locale_match = true;
				guess = item;
				break;
			    }
			}
		    }
		}
		if (requested_locale_match) break;
	    }
	}
    }

    if ( guess ) {
      setForInstallation (_pool, guess);
      MIL << " ==> REPLACED by: (pass 2: guessed): " << guess << endl;
      if ( ! doesObsoleteItem (guess, it->first ) ) {
	it->first.status().setToBeUninstalled( ResStatus::APPL_HIGH );
      }
      ++opt_stats_r.chk_replaced_guessed;
    }
  }

  ///////////////////////////////////////////////////////////////////
  // done
  ///////////////////////////////////////////////////////////////////
  MIL << opt_stats_r << endl;

  // Setting Resolver to upgrade mode
  _upgradeMode = true;
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


