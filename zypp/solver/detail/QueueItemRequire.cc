/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* QueueItemRequire.cc
 *
 * Copyright (C) 2000-2002 Ximian, Inc.
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

#include <sstream>

#include "zypp/CapFactory.h"
#include "zypp/CapMatch.h"
#include "zypp/CapSet.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"
#include "zypp/VendorAttr.h"
#include "zypp/base/Algorithm.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"
#include "zypp/ResStatus.h"
#include "zypp/Dep.h"

#include "zypp/ZYppFactory.h"

#include "zypp/solver/detail/QueueItemRequire.h"
#include "zypp/solver/detail/QueueItemBranch.h"
#include "zypp/solver/detail/QueueItemUninstall.h"
#include "zypp/solver/detail/QueueItemInstall.h"
#include "zypp/solver/detail/QueueItem.h"
#include "zypp/solver/detail/ResolverContext.h"
#include "zypp/solver/detail/ResolverInfoDependsOn.h"
#include "zypp/solver/detail/ResolverInfoMisc.h"
#include "zypp/solver/detail/ResolverInfoNeededBy.h"
#include "zypp/solver/detail/ResolverInfoMissingReq.h"
#include "zypp/solver/detail/Helper.h"

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

IMPL_PTR_TYPE(QueueItemRequire);

//---------------------------------------------------------------------------

std::ostream &
QueueItemRequire::dumpOn( std::ostream & os ) const
{
    os << "[" << (_soft?"Soft":"") << "Require: ";
    os << _capability;
    if (_requiring_item) {
	os << ", Required by " << _requiring_item;
    }
    if (_upgraded_item) {
	os << ", Upgrades " << _upgraded_item;
    }
    if (_lost_item) {
	os << ", Lost " << _lost_item;
    }
    if (_remove_only) os << ", Remove Only";
    return os << "]";
}

//---------------------------------------------------------------------------

QueueItemRequire::QueueItemRequire (const ResPool & pool, const Capability & cap, bool soft)
    : QueueItem (QUEUE_ITEM_TYPE_REQUIRE, pool)
    , _capability (cap)
    , _soft (soft)
    , _remove_only (false)
{
    _XDEBUG("QueueItemRequire::QueueItemRequire(" << cap << (soft?", soft":"") << ")");
}


QueueItemRequire::~QueueItemRequire()
{
}

//---------------------------------------------------------------------------

void
QueueItemRequire::addPoolItem (PoolItem_Ref item)
{
    assert (!_requiring_item);
    _requiring_item = item;
}


//---------------------------------------------------------------------------
struct UniqTable
{
  /** Order PoolItems based on name and edition only. */
  struct Order
  {
    /** 'less then' based on name and edition */
    bool operator()( PoolItem_Ref lhs, PoolItem_Ref rhs ) const
    {
      int res = lhs->name().compare( rhs->name() );
      if ( res )
        return res == -1; // lhs < rhs ?
      // here: lhs == rhs, so compare edition:
      return lhs->edition() < rhs->edition();
    }
  };
  /** Set of PoolItems unified by Order. */
  typedef std::set<PoolItem,Order> UTable;


  /** Test whether a matching PoolItem_Ref is in the table. */
  bool has( PoolItem_Ref item_r ) const
  { return _table.find( item_r ) != _table.end(); }

  /** Remember \a item_r (unless another matching PoolItem
   *  is already in the table)
  */
  void remember( PoolItem_Ref item_r )
  { _table.insert(  item_r ); }


  /** The set. */
  UTable _table;
};
//---------------------------------------------------------------------------

struct RequireProcess
{
    PoolItem_Ref _requirer;
    const Capability _capability;
    ResolverContext_Ptr _context;
    ResPool _pool;

    PoolItemList providers;		// the provider which matched

    // only keep 'best' candidate for a package name
    typedef map<string,PoolItem_Ref> UniqMap;
    UniqMap uniq;

    RequireProcess (PoolItem_Ref r, const Capability & c, ResolverContext_Ptr ctx, const ResPool & p)
	: _requirer (r)
	, _capability (c)
	, _context (ctx)
	, _pool (p)
    { }

    bool operator()( const CapAndItem & cai )
    {
	PoolItem provider = cai.item;
	Capability match = cai.cap;

	ResStatus status = _context->getStatus( provider );
	PoolItem_Ref upgrades = Helper::findInstalledItem (_pool, provider);

	XXX << "RequireProcessInfo (" << provider << " provides " << match << ", is " << status << ")" << endl;
// ERR << "RequireProcessInfo(required: " << *capability << ")" << endl;
// ERR << "require_process_cb(itemIsPossible -> " <<  context->itemIsPossible (*provider) << ")" << endl;

	/* capability is set for item set childern only. If it is set
	   allow only exactly required version */

	if (_capability != Capability()
	    && _capability != match) {		// exact match required
	    return true;
	}

	if (!provider->arch().compatibleWith( _context->architecture() )) {
	    MIL << "provider " << provider << " has incompatible arch '" << provider->arch() << "'" << endl;
	    return true;
	}

	if ( upgrades
	     && upgrades.resolvable()->arch() != provider->arch()
	     && provider->kind() != ResTraits<Atom>::kind )             // if patch provides arch upgrade, allow it (#266178)
 	                                                                // (because of #168840 and #170098, the patch parser grabs the 'best' atom
                                                                        //  and does not have knowledge about already installed atom with the same name.
                                                                        //  The problem #266178 shows is a previously installed patch (noarch) and atom (ppc)
                                                                        //  conflict with a later patch which offers an arch upgrade (ppc -> ppc64)
                                                                        //  This has no effect on the patch, since the patch is noarch. But is has effect
                                                                        //  on the atom, since it is installed as ppc and the upgrade is ppc64.
                                                                        //  Here, we look at arch changes only if they don't affect an atom. So atoms are
                                                                        //  allowed for arch upgrades.
                                                                        //  However, this only applies to atoms, not to packages. The package will stay
                                                                        //  at its architecture. Not doing arch upgrades was one of the requirements for Code10.)
	{
	    
	    MIL << "provider " << provider << " has OTHER arch '" << provider->arch() << "' than the updated item "
		<< upgrades << endl;
	    PoolItemList ignore = _context->getIgnoreArchitectureItem();
	    PoolItemList::iterator it;
	    for (it = ignore.begin(); it != ignore.end(); ++it) {
		if (provider == *it) break;
	    }
	    if (it != ignore.end()) {
		MIL << " ---> will be ignored " << endl;
	    } else {
		return true;
	    }
	}

	// checking vendor
	bool vendorFit = true;
	if ( provider
	     && upgrades
	     && !VendorAttr::instance().equivalent(provider->vendor(), upgrades->vendor())) {
	    // checking if there is already an ignore
	    MIL << "provider " << provider << " has ANOTHER vendor '" << provider->vendor() << "' than the updated item "
		<< upgrades << "(vendor: " << upgrades->vendor() << ")" <<  endl;
	    PoolItemList ignore = _context->getIgnoreVendorItem();
	    PoolItemList::iterator it;
	    for (it = ignore.begin(); it != ignore.end(); ++it) {
		if (provider == *it) break;
	    }
	    if (it != ignore.end()) {
		MIL << " ---> will be ignored " << endl;
	    } else {
		vendorFit = false;
	    }
	}

        Capability failed_cap;

	if (! (status.isToBeUninstalled() || status.isImpossible())
	    && ! _context->isParallelInstall( provider )
	    && _context->itemIsPossible( provider, failed_cap )
	    && ! provider.status().isLocked()
            && vendorFit
	    && ! (provider.status().isKept()
		  &&provider.status().isByUser())
	    ) {

	    // if we found a to-be-installed provider, choose this and drop all others
	    if (status.isToBeInstalled()			// scheduled for install
		|| (status.isUninstalled()
		    && provider.status().isToBeInstalled()))	// or will-be-scheduled 
	    {
		providers.clear();
		providers.push_front( provider );
		return false;
	    }


	    if (!_context->tryAllPossibilities()) {
		// if we already have same name
		//   check for better architecture, then edition
		//   see yast2-pkg-bindings, Package.cc, ProvideProcess

		UniqMap::iterator upos = uniq.find( provider->name() );

		if (upos != uniq.end()) {
		    if ((upos->second->arch().compare( provider->arch() ) < 0)	// better arch
			|| ((upos->second->arch().compare( provider->arch() ) == 0)		    // or same arch
			    && (upos->second->edition().compare( provider->edition() ) < 0) ) ) // and better edition
		    {
			// new provider is 'better'

			// erase the old provider
			for (PoolItemList::iterator it = providers.begin(); it != providers.end(); ++it) {
			    if (*it == upos->second) {
				// which we are currently ignore
				_XDEBUG("Kicking " << *it << " for " << provider);
				providers.erase( it );
				_context->setSkippedPossibilities( true ); // Flag that there are other possibilities			
				break;
			    }
			} 
			upos = uniq.end();	// trigger insertion of new provider below
		    } else {
			if (upos->second != provider) {
			    // which we are currently ignore
			    _XDEBUG("Ignoring " <<  provider);
			    _context->setSkippedPossibilities( true ); // Flag that there are other possibilities
			}
		    }
		}
		if (upos == uniq.end()) {
		    providers.push_front( provider );
		    uniq[provider->name()] = provider;
		}
	    } else {
		// try all alternatives
		providers.push_front( provider );
	    }
	}

	return true;
    }
};


struct NoInstallableProviders
{
    PoolItem_Ref requirer;
    ResolverContext_Ptr context;
    ResPool pool;    

    bool operator()( const CapAndItem cai)
    {
	PoolItem provider = cai.item;
	Capability match = cai.cap;
	PoolItem_Ref upgrades = Helper::findInstalledItem (pool, provider);	

	string msg_str;
	//const Capability match;

	ResStatus status = context->getStatus( provider );

	ResolverInfoMisc_Ptr misc_info;
        Capability failed_cap;

	if (status.isToBeUninstalled()) {
	    misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_UNINSTALL_PROVIDER, requirer, RESOLVER_INFO_PRIORITY_VERBOSE, match);
	    misc_info->setOtherPoolItem (provider);
	} else if (context->isParallelInstall (provider)) {
	    misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_PARALLEL_PROVIDER, requirer, RESOLVER_INFO_PRIORITY_VERBOSE, match);
	    misc_info->setOtherPoolItem (provider);
	} else if (status.isImpossible()
		  || ! context->itemIsPossible( provider, failed_cap )) {
	    misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_NOT_INSTALLABLE_PROVIDER, requirer, RESOLVER_INFO_PRIORITY_VERBOSE, match);
	    misc_info->setOtherPoolItem (provider);
	   if (!requirer       // user initiated the request
	       && failed_cap != Capability::noCap)  // solver knows why it failed
	     {
		ResolverInfoMissingReq_Ptr missing = new ResolverInfoMissingReq( provider, failed_cap );
		missing->flagAsImportant();
		context->addInfo( missing );
	     }
	} else if (provider.status().isLocked()) {
	    misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_LOCKED_PROVIDER, requirer, RESOLVER_INFO_PRIORITY_VERBOSE, match);
	    misc_info->setOtherPoolItem (provider);
	} else if (provider.status().isKept() && provider.status().isByUser()) {
	    misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_KEEP_PROVIDER, requirer, RESOLVER_INFO_PRIORITY_VERBOSE, match);
	    misc_info->setOtherPoolItem (provider);	    
 	} else	if (provider
		    && upgrades
		    && !VendorAttr::instance().equivalent(provider->vendor(), upgrades->vendor())) {		    
	    misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_OTHER_VENDOR_PROVIDER,
								   upgrades, RESOLVER_INFO_PRIORITY_VERBOSE, match);
	    misc_info->setOtherPoolItem (provider);
	}
	else if (provider->arch().compatibleWith( context->architecture() )) {
	    misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_OTHER_ARCH_PROVIDER,
								   requirer, RESOLVER_INFO_PRIORITY_VERBOSE, match);
	    misc_info->setOtherPoolItem (provider);
	} 
	
	if (misc_info != NULL) {
	    context->addInfo (misc_info);
	}

	return true;
    }
};

typedef map<string, PoolItem_Ref> UpgradesMap;

struct LookForUpgrades
{
    PoolItem_Ref installed;
    ResolverContext_Ptr _context;    
    UpgradesMap upgrades;

    LookForUpgrades (PoolItem_Ref i, ResolverContext_Ptr ctx)
	: installed (i)
	, _context (ctx)	
    { }

    bool operator()( PoolItem_Ref provider )
    {
	if ( provider.status().maySetToBeInstalled( ResStatus::SOLVER )) {
	    // Take canditates only which are really installable. Bug 292077 
	    UpgradesMap::iterator it = upgrades.find( provider->name() );

	    if (it != upgrades.end()) {				// provider with same name found
		if (!_context->upgradeMode()
		    && it->second->arch() != installed->arch()
		    && provider->arch() == installed->arch()) {
		    // prefering the same architecture as the installed item
		    // NOT in upgrade mode
		    it->second = provider;
		} else {
		    int cmp = it->second->arch().compare( provider->arch() );
		    if ((_context->upgradeMode()  					// only in upgrade mode
			 || it->second->arch() != installed->arch())                // or have not found the same arch as installed item
			&& cmp < 0) {						// new provider has better arch
			it->second = provider;
		    }
		    else if (cmp == 0) {					// new provider has equal arch
			if (it->second->edition().compare( provider->edition() ) < 0) {
			    it->second = provider;				// new provider has better edition
			}
		    }
		}
	    } else {
		upgrades[provider->name()] = provider;
	    }
	} 
	return true;
    }
};


// check for an installed or to-be-installed item
// (used to match supplements/enhances of multiple providers)

struct HintItem
{
    PoolItem_Ref match;

    bool operator()( const CapAndItem & cai )
    {
	if (cai.item.status().staysInstalled()
	    || cai.item.status().isToBeInstalled())
	{
	    match = cai.item;
	    return false;		// have one, we're done
	}
	return true;
    }
};


// check 'codependent' items
//   by looking at the name prefix: 'foo' and 'foo-bar' are codependent

static bool
codependent_items (const PoolItem_Ref item1, const PoolItem_Ref item2)
{
    string name1 = item1->name();
    string name2 = item2->name();
    string::size_type len1 = name1.size();
    string::size_type len2 = name2.size();

    if (len2 < len1) {
	string swap = name1;
	string::size_type swap_len = len1;
	name1 = name2;
	name2 = swap;
	len1 = len2;
	len2 = swap_len;
    }

    // foo and foo-bar are automatically co-dependent
    if (len1 < len2
	&& name1.compare (0, len1, name2) == 0
	&& name2[len1] == '-') {
	return true;
    }

    return false;
}


// check if we have a match for a (supplements/enhances) hint

static bool
hint_match( const CapSet & cset, ResPool pool )
{
    HintItem info;
    
    for (CapSet::const_iterator cit = cset.begin(); cit != cset.end(); ++cit) {
	Dep dep( Dep::PROVIDES );
	invokeOnEach( pool.byCapabilityIndexBegin( cit->index(), dep ),
		      pool.byCapabilityIndexEnd( cit->index(), dep ),
		      resfilter::ByCapMatch( *cit ),
		      functor::functorRef<bool,CapAndItem>(info) );
    }
    if (info.match) MIL << "hint_match(" << info.match << ")" << endl;
    return info.match; // as bool !
}

//----------------------------------------------------------------------------

bool
QueueItemRequire::process (const QueueItemList & mainQueue,
			   ResolverContext_Ptr context, QueueItemList & new_items)
{
    _XDEBUG("QueueItemRequire::process(" << *this << ")");

    bool fulfilled = false;
	    
    if (_requiring_item)
    {
	fulfilled = context->requirementIsInstalledOrUnneeded (_capability, _requiring_item,
							       _soft?Dep::RECOMMENDS:Dep::REQUIRES);
    } else {
	fulfilled = context->requirementIsMet (_capability, PoolItem_Ref(), Dep::REQUIRES);
    }
    
    if (fulfilled) {
	_XDEBUG("requirement is already met in current context");
	return true;
    }

    // checking for ignoring dependencies
    IgnoreMap ignoreMap = context->getIgnoreRequires();
    for (IgnoreMap::iterator it = ignoreMap.begin();
	 it != ignoreMap.end(); it++) {
	if ( (!(it->first) // ignore ALL requirements on this capability
	      || it->first == _requiring_item)
	     && it->second == _capability) {
	    _XDEBUG("Found ignoring requires " << _capability << " for " << _requiring_item);
	    return true;
	} else {
	    _XDEBUG("Ignoring requires " << it->second << " for " <<  it->first << " does not fit");	    
 	}
    }

    RequireProcess info (_requiring_item,  Capability(), context,  pool());

    int num_providers = 0;

    if (! _remove_only) {

	Dep dep( Dep::PROVIDES );
	XXX << "Look for providers of " << _capability << endl;
	// world->foreachProvidingResItem (_capability, require_process_cb, &info);
	invokeOnEach( pool().byCapabilityIndexBegin( _capability.index(), dep ),
		      pool().byCapabilityIndexEnd( _capability.index(), dep ),
		      resfilter::ByCapMatch( _capability ),
		      functor::functorRef<bool,CapAndItem>(info) );

	_XDEBUG("Look for providers of " << _capability);

	num_providers = info.providers.size();

	_XDEBUG( "requirement is met by " << num_providers << " resolvable");


	// if there are multiple providers, try to reduce branching by cleaning up the providers list
	// first check all providers if they are to-be-installed or uninstalled
	//   if there are to-be-installed providers, erase all uninstalled providers
	//   if there are locale providers and none match, try a language fallback
	//   prefer providers which enhance an installed or to-be-installed resolvables

	if (num_providers > 1) {					// prefer to-be-installed providers
#if 1 // see also line 618
	    // if there are exactly two providers which differ in architecture
	    // prefer the better arch
	    // this will reduce the number of branches for X-32bit.x86_64 vs. X.i586 dramatically
	    //
	    // - left commented out as advised by mls@suse.de, might be problematic on non-x86 archs
	    // - Due the new behaviour of the solver this code will be executed.
	    //   New: If there is no solution for the "best-architecture" way the solver tries
	    //        the other branches too.

	    if (num_providers == 2) {
		PoolItemList::iterator it = info.providers.begin();
		PoolItem first( *it++ );
		PoolItem second( *it );

		if (NVRA(first.resolvable()) == NVRA(second.resolvable()))
		{
		    // regarding items with the same NVRA only. Bug238284 
		    int cmp = first->arch().compare( second->arch() );
		    if (cmp < 0) {		// second is better
			--it;
		    }

		    if (cmp != 0) {
			info.providers.erase( it );		// erase one of both
			num_providers = 1;
			goto provider_done;
		    }
		}
	    }
#endif
	    MIL << "Have " << num_providers << " providers for " << _capability << endl;
	    int to_be_installed = 0;
	    int uninstalled = 0;
	    std::map<std::string,PoolItem> language_freshens;
	    ZYpp::Ptr z = zypp::getZYpp();
	    ZYpp::LocaleSet requested_locales = z->getRequestedLocales();
	    bool requested_locale_match = false;
	    PoolItem requested_locale_item;
	    PoolItemSet hints;			// those which supplement or enhance an installed or to-be-installed

	    for (PoolItemList::iterator it = info.providers.begin(); it != info.providers.end(); ++it) {
		PoolItem item = *it;
		if (item.status().isToBeInstalled()) {
		    to_be_installed++;
		}
		if (item.status().staysUninstalled()) {
		    uninstalled++;
		}
		CapSet freshens( item->dep( Dep::FRESHENS ) );

		// try to find a match of the locale freshens with one of the requested locales

		for (CapSet::const_iterator cit = freshens.begin(); cit != freshens.end(); ++cit) {
		    if (cit->refers() == ResTraits<Language>::kind) {
			string loc = cit->index();
			MIL << "Look for language fallback " << loc << ":" << item << endl;
			if (requested_locales.find( Locale( loc ) ) != requested_locales.end()) {
			    MIL << "Locale '" << loc << "' is requested" << endl;
			    requested_locale_match = true;
			    requested_locale_item = item;
			}
			language_freshens[loc] = item;
		    }
		}

		// now check if a provider supplements or enhances an installed or to-be-installed resolvable
		if (hint_match( item->dep( Dep::SUPPLEMENTS ), pool() )
		    || hint_match( item->dep( Dep::ENHANCES ), pool() ))
		{
		    hints.insert( item );
		}

	    } // for

	    if (hints.empty()
		&& to_be_installed == 0
		&& !requested_locale_match)
	    {
		// loop over requested locales, loop over their fallbacks, and try to find a matching provider

		for (ZYpp::LocaleSet::const_iterator rit = requested_locales.begin(); rit != requested_locales.end(); ++rit) {

		    // loop over possible fallbacks
		    Locale l = *rit;
		    for (;;) {
			Locale fallback = l.fallback();
			if (fallback == Locale::noCode
			    || fallback == l)
			{
			    break;
			}
			MIL << "requested " << l << " fallback " << fallback << endl;
			std::map<std::string,PoolItem>::const_iterator match = language_freshens.find( fallback.code() );
			if (match != language_freshens.end()) {
			    MIL << match->second << " matches the fallback" << endl;
			    info.providers.clear();
			    info.providers.push_back( match->second );
			    break;
			}
			l = fallback;
		    }
		}
#if 0	// just debug
		std::string mil = "language_freshens ";
		for (std::map<std::string,PoolItem>::const_iterator it = language_freshens.begin(); it != language_freshens.end(); ++it) {
		    if (it != language_freshens.begin()) mil += ", ";
		    mil += it->first;
		}
		MIL << mil << endl;
#endif
	    }
	    else if (to_be_installed == 0
		     && !hints.empty())
	    {
		MIL << "Have " << hints.size() << " hints" << endl;
		info.providers.clear();
		for (PoolItemSet::const_iterator it = hints.begin(); it != hints.end(); ++it) {
		    if (*it == requested_locale_item) {
			// This is the requested language item
			info.providers.push_back( *it );
		    } else {
			// go through the list of language items and check if they are exists
			std::map<std::string,PoolItem>::const_iterator itFr;
		    
			for (itFr = language_freshens.begin(); itFr != language_freshens.end(); ++itFr) {
			    if (*it == itFr->second) break;
			}
			if (itFr == language_freshens.end()) {
			    // item was only in the supplements --> branch for it
			    info.providers.push_back( *it );
			}
		    }
		}
		if (info.providers.empty()) {
		    // they are all language items which does not fit in the required language
		    // So take the firstone to fulfill the requirement. Should never happens
		    info.providers.push_back( *(hints.begin()) );
		}
	    }
	    else { 

	    // if we have one or more to-be-installed, erase all uninstalled (if there are any)

	    MIL << to_be_installed << " to-be-installed, " << uninstalled << " uninstalled" << endl;

	    if (to_be_installed > 0
		&& uninstalled > 0)
	    {
		PoolItemList::iterator next;
		for (PoolItemList::iterator it = info.providers.begin(); it != info.providers.end();) {
		    next = it; ++next;
		    if (it->status().staysUninstalled()) {
			MIL << "Not considering " << *it << endl;
			info.providers.erase (it);
		    }
		    it = next;
		}
	    }
	    }

	    if (!context->tryAllPossibilities()) {
		// Evaluate the best architecture of the providers
		Arch bestArch = Arch(); // is noarch
		PoolItemList::iterator it;
		for (it = info.providers.begin();
		     it != info.providers.end(); it++) {
		    if (bestArch.compare( (*it)->arch() ) < 0) {	// better arch
			bestArch = (*it)->arch();
		    }	
		}
		
		// filter out all resolvables which have worser architecture, are NOT noarch
		// and have not the same name as the requirement. The last one is needed
		// for updating packages via patch/atoms.
		PoolItemList::iterator next;	    
		for (it = info.providers.begin();
		     it != info.providers.end();) {

		    bool nameFit = false;
		    CapFactory factory;		    
		    if (isKind<capability::NamedCap>( _capability ) ) {
			Capability capTest =  factory.parse ( (*it)->kind(), (*it)->name(), Rel::ANY, Edition::noedition );
			if (capTest.matches (_capability) == CapMatch::yes) {
			    nameFit = true;
			    _XDEBUG("Required Capability " << _capability << " has the same name as the provider:" << *it);
			    _XDEBUG("    --> do not trow away althout it could have the wrong architecture");
			}
		    }
		    
		    next = it; ++next;
		    if ((*it)->arch() != bestArch
			&& (*it)->arch() != Arch_noarch
			&& !nameFit) {
			_XDEBUG("Kicking " << *it << " due best architecture " << bestArch);
			info.providers.erase( it );
		    }
		    it = next;
		}
	    }

	    num_providers = info.providers.size();

	} // num_providers > 1
#if 1 // see also line 474
provider_done:;
#endif
    } // !_remove_only

    //
    // No providers found
    //

    if (num_providers == 0) {

	if (_soft) goto finished;			// don't care for soft requires

	_DEBUG( "Unfulfilled requirement '" << _capability << "'. trying different solution");

	QueueItemUninstall_Ptr uninstall_item = NULL;
	QueueItemBranch_Ptr branch_item = NULL;
	bool explore_uninstall_branch = true;

	

	if (!_upgraded_item
	    || _lost_item)
	{
	    ResolverInfo_Ptr err_info;
	    NoInstallableProviders info;
	    info.requirer = _requiring_item;
	    info.context = context;
	    info.pool = pool();

	    // Maybe we can add some extra info on why none of the providers are suitable.

	    // pool()->foreachProvidingResItem (_capability, no_installable_providers_info_cb, (void *)&info);

	    Dep dep( Dep::PROVIDES );

	    invokeOnEach( pool().byCapabilityIndexBegin( _capability.index(), dep ), // begin()
			  pool().byCapabilityIndexEnd( _capability.index(), dep ),   // end()
			  resfilter::ByCapMatch( _capability ),
			  functor::functorRef<bool,CapAndItem>(info) );

	}
	//
	// If this is an upgrade, we might be able to avoid removing stuff by upgrading it instead.
	//

	if ((_upgraded_item
	     || _lost_item // foo-devel requires foo; foo is lost due obsolete --> try to upgrade foo-devel
	     || context->verifying()) // We are in the verify mode. So there could be already missing requirements
	                              // So try to solve it via update
	    && _requiring_item)
	{

	    LookForUpgrades info (_requiring_item, context);

//	    pool()->foreachUpgrade (_requiring_item, new Channel(CHANNEL_TYPE_ANY), look_for_upgrades_cb, (void *)&upgrade_list);

	    invokeOnEach( pool().byNameBegin( _requiring_item->name() ), pool().byNameEnd( _requiring_item->name() ),
			  functor::chain (resfilter::ByKind( _requiring_item->kind() ),
					  resfilter::byEdition<CompareByGT<Edition> >( _requiring_item->edition() ) ),
					  functor::functorRef<bool,PoolItem>(info) );

	    if (!info.upgrades.empty()) {
		string label;

		branch_item = new QueueItemBranch (pool());

		ostringstream req_str;	req_str << _requiring_item;
		ostringstream up_str;
		if (_upgraded_item)
		    up_str << _upgraded_item;
		else
		    up_str << _requiring_item;
		ostringstream cap_str; cap_str << _capability;

		 // Translator: 1.%s = dependency; 2.%s and 3.%s = name of package,patch,...
		label = str::form (_("for requiring %s for %s when upgrading %s"),
				   cap_str.str().c_str(), req_str.str().c_str(), up_str.str().c_str());
		branch_item->setLabel (label);
		_DEBUG("Branching: " + label)

		for (UpgradesMap::const_iterator iter = info.upgrades.begin(); iter != info.upgrades.end(); ++iter) {
		    PoolItem_Ref upgrade_item = iter->second;
		    QueueItemInstall_Ptr install_item;
		    Capability failed_cap;
		   
		    if (context->itemIsPossible( upgrade_item, failed_cap )) {

			install_item = new QueueItemInstall (pool(), upgrade_item, _soft);
		    	install_item->setUpgrades (_requiring_item);
			branch_item->addItem (install_item);

			ResolverInfoNeededBy_Ptr upgrade_info = new ResolverInfoNeededBy (upgrade_item);
			if (_upgraded_item)
			    upgrade_info->addRelatedPoolItem (_upgraded_item);
			upgrade_info->setCapability (_capability, Dep::REQUIRES);
			install_item->addInfo (upgrade_info);

			// If an upgrade item has its requirements met, don't do the uninstall branch.
			//   FIXME: should we also look at conflicts here?

			if (explore_uninstall_branch) {
			    CapSet requires = upgrade_item->dep (Dep::REQUIRES);
			    CapSet::const_iterator iter = requires.begin();
			    for (; iter != requires.end(); iter++) {
				const Capability req = *iter;
				if (! context->requirementIsMet (req, upgrade_item, Dep::REQUIRES)) {
					break;
				}
			    }
			    if (iter == requires.end()) {
				explore_uninstall_branch = false;
			    }
			}

		    } /* if (context->itemIsPossible ( ... */
		} /* for (iter = upgrade_list; ... */
	    } /* if (info.upgrades) ... */

	    if (!info.upgrades.empty()
		&& branch_item->isEmpty ())
	    {

		for (UpgradesMap::const_iterator iter = info.upgrades.begin(); iter != info.upgrades.end(); ++iter) {
		    ResolverInfoMisc_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_NO_UPGRADE, _requiring_item, RESOLVER_INFO_PRIORITY_VERBOSE);
		    if (iter == info.upgrades.begin()) {
			misc_info->setOtherPoolItem( iter->second );
		    }
		    misc_info->addRelatedPoolItem( iter->second );
		    context->addInfo( misc_info );

		    explore_uninstall_branch = true;
		}

		//
		//  The exception: we always want to consider uninstalling
		//  when the requirement has resulted from a item losing
		//  one of it's provides.

	    } else if (!info.upgrades.empty()
		       && explore_uninstall_branch
		       && _requiring_item
		       && _upgraded_item
		       && codependent_items (_requiring_item, _upgraded_item)
		       && !_lost_item)
	    {
		explore_uninstall_branch = false;
	    }

	} /* if (_upgrade_item && _requiring_item) ... */

	ResStatus status = context->getStatus(_requiring_item);
	
	if (context->verifying()) {
	    // We always consider uninstalling when in verification mode.
	    explore_uninstall_branch = true;
	}
	else if (status.isToBeInstalled()           // scheduled for installation
		 && !status.isToBeUninstalled()     // not scheduled for uninstallation
		 || _requiring_item.status().staysInstalled()) // not scheduled at all but installed
	{
	    // The item has to be set for installing/updating explicity.
	    // So the uninstall branch could be not useful if upgrade is not successful.
	    // Adding the info for solution handling at the end of solving
	    ResolverInfo_Ptr misc_info;
	    if (!_upgraded_item) {
		if (_remove_only) {
		    misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_NO_OTHER_PROVIDER,
						     _requiring_item, RESOLVER_INFO_PRIORITY_VERBOSE, _capability);
		} else {
		    misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_NO_PROVIDER,
						     _requiring_item, RESOLVER_INFO_PRIORITY_VERBOSE, _capability);
		}
	    } else {
		misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_CANT_SATISFY,
						  _requiring_item, RESOLVER_INFO_PRIORITY_VERBOSE,
						  _capability);
	    }	    
	    context->addInfo (misc_info);
	}

	if (explore_uninstall_branch && _requiring_item) {
	    ResolverInfo_Ptr log_info;
	    uninstall_item = new QueueItemUninstall (pool(), _requiring_item, QueueItemUninstall::UNSATISFIED);
	    uninstall_item->setPriority(0); // evaluate explizit uninstall first (greater priority) in order
	                                     // to be sure, that this item still exist after the solver run would be finished. Bug 273918	    
	    uninstall_item->setCapability (_capability);

	    if (_lost_item) {
		log_info = new ResolverInfoDependsOn (_requiring_item, _lost_item);
		uninstall_item->addInfo (log_info);
	    }

	    if (_remove_only)
		uninstall_item->setRemoveOnly ();
	}

	if (uninstall_item && branch_item) {
	    branch_item->addItem (uninstall_item);
	    new_items.push_back (branch_item);
	} else if (uninstall_item) {
	    new_items.push_front (uninstall_item);
	} else if (branch_item) {
	    new_items.push_back (branch_item);
	} else {
	    // We can't do anything to resolve the missing requirement, so we fail.
	    ResolverInfo_Ptr misc_info;
	    if (!_upgraded_item) {
		if (_remove_only) {
		    misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_NO_OTHER_PROVIDER,
						     _requiring_item, RESOLVER_INFO_PRIORITY_VERBOSE, _capability);
		} else {
		    misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_NO_PROVIDER,
						     _requiring_item, RESOLVER_INFO_PRIORITY_VERBOSE, _capability);
		}
	    } else {
		ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_CANT_SATISFY,
								   _requiring_item, RESOLVER_INFO_PRIORITY_VERBOSE,
								   _capability);
	    }
	    context->addError (misc_info);
	}

    }

    //
    // exactly 1 provider found
    //

    else if (num_providers == 1) {

	_XDEBUG( "Found exactly one resolvable, installing it.");

	PoolItem item = info.providers.front();

	// if this a soft require (a recommends), don't override a KEEP_STATE (-> bug #154650)
	// see also below
	if (_soft
	    && item.status().isUninstalled()
	    && !item.status().maySetSoftTransact( true, ResStatus::SOLVER ) )
	{
	    _DEBUG("Can't soft-transact " << item);
	    goto finished;
	}
	QueueItemInstall_Ptr install_item = new QueueItemInstall (pool(), item, _soft);
	install_item->setDependency (_capability);

	// The requiring item could be NULL if the requirement was added as an extra dependency.
	if (_requiring_item) {
	    install_item->setNeededBy (_requiring_item);
	}
	new_items.push_front (install_item);

    }

    //
    // multiple providers found
    //

    else if (num_providers > 1) {

	_DEBUG( "Branching: Found more than one provider of " << _capability);

	QueueItemBranch_Ptr branch_item = new QueueItemBranch( pool() );

	for (PoolItemList::const_iterator iter = info.providers.begin(); iter != info.providers.end(); iter++) {

	    PoolItem item = *iter;

	    // if this a soft require (a recommends), don't override a KEEP_STATE (-> bug #154650)
	    // see also above
	    if (_soft
		&& item.status().isUninstalled()
		&& !item.status().maySetSoftTransact( true, ResStatus::SOLVER ) )
	    {
		_DEBUG("Can't soft-transact " << item);
		continue;
	    }

	    QueueItemInstall_Ptr install_item = new QueueItemInstall( pool(), item, _soft );
	    install_item->setDependency( _capability );
	    branch_item->addItem( install_item );

	    // The requiring item could be NULL if the requirement was added as an extra dependency.
	    if (_requiring_item) {
		install_item->setNeededBy( _requiring_item );
	    }
	}

	if (!branch_item->isEmpty())
	    new_items.push_back (branch_item);

    } else {
	abort ();
    }

finished:

    return true;
}

//---------------------------------------------------------------------------

QueueItem_Ptr
QueueItemRequire::copy (void) const
{
    QueueItemRequire_Ptr new_require = new QueueItemRequire (pool(), _capability);

    new_require->QueueItem::copy(this);

    new_require->_requiring_item = _requiring_item;
    new_require->_upgraded_item  = _upgraded_item;
    new_require->_remove_only    = _remove_only;

    return new_require;
}


int
QueueItemRequire::cmp (QueueItem_constPtr item) const
{
    int cmp = this->compare (item);		// assures equal type
    if (cmp != 0)
	return cmp;

    QueueItemRequire_constPtr require = dynamic_pointer_cast<const QueueItemRequire>(item);

    if (_capability != require->capability())
    {
	cmp = -1;
    }
    return cmp;
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

