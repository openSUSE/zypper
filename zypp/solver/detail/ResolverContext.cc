/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResolverContext.cc
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


#include <values.h>

#include "zypp/CapSet.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/String.h"

#include "zypp/base/Algorithm.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"
#include "zypp/Package.h"
#include "zypp/Resolvable.h"

#include "zypp/solver/detail/Types.h"
#include "zypp/solver/detail/Helper.h"
#include "zypp/solver/detail/ResolverContext.h"
#include "zypp/solver/detail/ResolverInfoMisc.h"

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

IMPL_PTR_TYPE(ResolverContext);

//---------------------------------------------------------------------------

ostream&
operator<<( ostream& os, const ResolverContext & context)
{
    if (context._parent != NULL) {
	os << "Parent @" << context._parent << endl;
	os << *(context._parent);
    }
    os << "ResolverContext with " << context._context.size() << " entries" << endl;
    for (ResolverContext::Context::const_iterator iter = context._context.begin(); iter != context._context.end(); ++iter) {
	os << iter->first << " : " << iter->second << endl;
    }
    return os;
}

//---------------------------------------------------------------------------

ResolverContext::ResolverContext (const ResPool & pool, const Arch & arch, ResolverContext_Ptr parent)
    : _parent (parent)
    , _pool (pool)
    , _download_size (0)
    , _install_size (0)
    , _total_priority (0)
    , _min_priority (0)
    , _max_priority (0)
    , _other_penalties (0)
    , _verifying (false)
    , _establishing (false)
    , _invalid (false)
    , _architecture(arch)
    , _forceResolve(false)
    , _upgradeMode(false)
      
{
_XDEBUG( "ResolverContext[" << this << "]::ResolverContext(" << parent << ")" );
    if (parent != NULL) {
	_pool		     = parent->_pool;
	_download_size       = parent->_download_size;
	_install_size        = parent->_install_size;
	_total_priority      = parent->_total_priority;
	_max_priority        = parent->_max_priority;
	_min_priority        = parent->_min_priority;
	_other_penalties     = parent->_other_penalties;
	_verifying	     = parent->_verifying;
	_establishing	     = parent->_establishing;
	_ignoreConflicts     = parent->_ignoreConflicts;
	_ignoreRequires      = parent->_ignoreRequires;
	_ignoreArchitecture  = parent->_ignoreArchitecture;
	_ignoreInstalledItem = parent->_ignoreInstalledItem;
	_forceResolve        = parent->_forceResolve;
	_upgradeMode         = parent->_upgradeMode;
    } else {
	_min_priority = MAXINT;
    }
}


ResolverContext::~ResolverContext()
{
}

//---------------------------------------------------------------------------
// status retrieve

ResStatus
ResolverContext::getStatus (PoolItem_Ref item)
{
_XDEBUG( "[" << this << "]getStatus(" << item << ")" );

    if (item == _last_checked_item) return _last_checked_status;

    _last_checked_item = item;

    Context::const_iterator it;
    ResolverContext_constPtr context = this;

    while (context) {					// go through the _parent chain

	it = context->_context.find(item);		// part of local context ?
	if (it != context->_context.end()) {
_XDEBUG( "[" << context << "]:" << it->second );
	    _last_checked_status = it->second;
	    return it->second;				// Y: return
	}
	context = context->_parent;			// N: go up the chain
    }
#if 1
    ResStatus status;
    if (item.status().isInstalled())
	status = ResStatus::installed;			// return _is_ state, not _to be_ state
    else
	status = ResStatus::uninstalled;		// return _is_ state, not _to be_ state

    _last_checked_status = status;
    _XDEBUG( "[NULL]:" << status );    
#else
    _last_checked_status = item.status();    
#endif

    return _last_checked_status;				// Not part of context, return Pool status
}


//---------------------------------------------------------------------------
// status change

void
ResolverContext::setStatus (PoolItem_Ref item, const ResStatus & status)
{
    if (_invalid) return;

_XDEBUG( "[" << this << "]setStatus(" << item << ", " << status << ")" );
    if (status == item.status()) {		// same as original status
	Context::iterator it;
	it = _context.find(item);		// part of local context ?
	if (it != _context.end()) {
_XDEBUG( "UNMARK" );
	    _context.erase (it);		// erase it !
	}
    }
    else {
	ResStatus old_status = getStatus (item);

	if (old_status != status) {		// new status ?
_XDEBUG( "MARK" );
	    _context[item] = status;		// set it !
	}
    }

    _last_checked_item = item;
    _last_checked_status = status;

    return;
}


// change state to TO_BE_INSTALLED (does not upgrade)

bool
ResolverContext::install (PoolItem_Ref item, bool is_soft, int other_penalty)
{
    ResStatus status, new_status;
    std::string msg;

    status = getStatus(item);
    _XDEBUG( "ResolverContext[" << this << "]::install(<" << status  << "> " << item << ")" );

    if (status.isToBeUninstalled()
	&& !status.isToBeUninstalledDueToUnlink()) {
	ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_INSTALL_TO_BE_UNINSTALLED, item, RESOLVER_INFO_PRIORITY_VERBOSE);
	addError (misc_info);
	return false;
    }

    if (status.isImpossible()) {
	ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_UNINSTALLABLE, item, RESOLVER_INFO_PRIORITY_VERBOSE);
	// it is only an error, if the user wants to install explicity.
	if (is_soft) {
	    addInfo (misc_info);
	} else {
	    addError (misc_info);
	}
	return false;
    }

    if (status.isUnneeded()) {
	ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_INSTALL_UNNEEDED, item, RESOLVER_INFO_PRIORITY_VERBOSE);
	addError (misc_info);
	return false;
    }

    if (status.isToBeInstalled()) {
	return true;
    }

    if (isParallelInstall (item)) {
	ResolverInfoMisc_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_INSTALL_PARALLEL, item, RESOLVER_INFO_PRIORITY_VERBOSE);
	misc_info->setOtherPoolItem (getParallelInstall (item));
	addError (misc_info);
	return false;
    }

    if (is_soft)
	setStatus (item, ResStatus::toBeInstalledSoft);
    else if (status.isToBeUninstalledDueToUnlink())
	setStatus (item, ResStatus(true));
    else
	setStatus (item, ResStatus::toBeInstalled);

    if (status.wasUninstalled()) {
	Resolvable::constPtr res = item.resolvable();
	Package::constPtr pkg = asKind<Package>(res);			// try to access it as a package
	if (pkg) {							// if its !=NULL, get size information

	    _download_size += pkg->archivesize();
	    _install_size += pkg->size();

	}

	int priority;
#if 0
	if (item->local())
	    priority = 0;
	else {
#endif
	    priority = getSourcePriority (item->source());
//	}

	if (priority < _min_priority) _min_priority = priority;
	if (priority > _max_priority) _max_priority = priority;

	_other_penalties += other_penalty;

    }

    return true;
}


// change state to TO_BE_INSTALLED (does upgrade)

bool
ResolverContext::upgrade (PoolItem_Ref item, PoolItem_Ref old_item, bool is_soft, int other_penalty)
{
    ResStatus status;

    _XDEBUG( "ResolverContext[" << this << "]::upgrade(" << item << " upgrades " << old_item << ")" );

    status = getStatus(item);

    if (status.isToBeUninstalled()
	|| status.isImpossible())
	return false;
    
    if (status.isToBeInstalled())
	return true;
    
    if (is_soft) {
	setStatus (item, ResStatus::toBeInstalledSoft);
    }
    else {
	setStatus (item, ResStatus::toBeInstalled);
    }
    
    Resolvable::constPtr res = old_item.resolvable();
    Package::constPtr pkg = asKind<Package>(res);			// try to access it as a package
    if (pkg) {							// if its !=NULL, get size information

	_install_size -= pkg->size();
    }

    if (status == ResStatus::uninstalled) {
	res = item.resolvable();
	pkg = asKind<Package>(res);					// try to access it as a package
	if (pkg) {							// if its !=NULL, get size information

	    _download_size += pkg->archivesize();
	    _install_size += pkg->size();

	}

	int priority;
#if 0
	if (item->local())
	    priority = 0;
	else {
#endif
	    priority = getSourcePriority (item->source());
//	}

	if (priority < _min_priority) _min_priority = priority;
	if (priority > _max_priority) _max_priority = priority;

	_other_penalties += other_penalty;
    }
    return true;
}


// change state to 'TO_BE_UNINSTALLED{, DUE_TO_OBSOLETE, DUE_TO_UNLINK}'

bool
ResolverContext::uninstall (PoolItem_Ref item, bool part_of_upgrade, bool due_to_obsolete, bool due_to_unlink)
{
    ResStatus status, new_status;
    std::string msg;

    _XDEBUG( "ResolverContext[" << this << "]::uninstall("
		    << item << " " << (part_of_upgrade ? "part_of_upgrade" : "") << " "
		    << (due_to_obsolete ? "due_to_obsolete": "") << " "
	     << (due_to_unlink ? "due_to_unlink" : "") << ")" << "pool-status:" << item.status());

    assert (! (due_to_obsolete && due_to_unlink));

    status = getStatus(item);

    if ( ( (forceResolve() // This is the behaviour of ZMD
	    || upgradeMode())
	  && (status.isToBeInstalled()             // \ The resolvable will be installed
	      || item.status().isToBeInstalled())) // / explicit.
	 
	 || ( (!forceResolve() // This is the bahaviour of YaST
	       && !upgradeMode())
	     && (status.staysInstalled() || status.isToBeInstalled())               //   \ We will have the resolvable
	     && (item.status().staysInstalled() || item.status().isToBeInstalled()) //   / available.
	     && !part_of_upgrade
	     && !due_to_obsolete
	     && !due_to_unlink)) {
	// We have a resolvable which should be kept on the system or is set to be installed explicit.
	// So we are not allowed deleting it. The reason WHY this resolvable has to be deleted here
	// is not show. We go back to the ResolverInfo to evaluate the reason. This reason (marked as an info)
	// will be stored at the end of ResolverInfo and will be marked as an error which is shown 
	// to the UI (solution included)
	// Canditates of searched ResolverInfo are RESOLVER_INFO_TYPE_CONFLICT_CANT_INSTALL
	//                                         RESOLVER_INFO_TYPE_NO_PROVIDER
 	//                                         RESOLVER_INFO_TYPE_NO_OTHER_PROVIDER
	//                                         RESOLVER_INFO_TYPE_CANT_SATISFY
	//
	// Testcases are:
	// conflict2-test.xml  conflict-test.xml              remove-still-needed2-test.xml  require-test.xml
	// conflict3-test.xml  remove-still-needed1-test.xml  remove-still-needed3-test.xml  unfulfilled-2-test.xml
	//
	bool found = false;
	ResolverInfoList addList;
	for (ResolverInfoList::const_iterator iter = _log.begin(); iter != _log.end(); iter++) {
	    ResolverInfo_Ptr info = *iter;
//	    if (info->affected() == item
	    if (info->type() == RESOLVER_INFO_TYPE_CONFLICT_CANT_INSTALL
		    || info->type() == RESOLVER_INFO_TYPE_NO_PROVIDER
		    || info->type() == RESOLVER_INFO_TYPE_NO_OTHER_PROVIDER
		    || info->type() == RESOLVER_INFO_TYPE_CANT_SATISFY)
	    {
		// put the info on the end as error
		found = true;
		addList.push_back (info);
	    }
	}
	if (!found) {
	    // generating a default problem
	    ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_REJECT_INSTALL, item, RESOLVER_INFO_PRIORITY_VERBOSE);
	    addError (misc_info);
	} else {
	    // Put the info at the end of the list, flagged as error
	    for (ResolverInfoList::const_iterator iter = addList.begin(); iter != addList.end(); iter++) {
		ResolverInfo_Ptr info = *iter;
		addError (info);
	    }
	}
	
	return false;
    }

    if (status.isToBeUninstalled()
	&& !status.isToBeUninstalledDueToUnlink())
    {
	return true;
    }

    if (status.wasUninstalled()
	|| status.isImpossible()
	|| status.isToBeUninstalledDueToUnlink())
    {
	ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_UNINSTALLABLE, item, RESOLVER_INFO_PRIORITY_VERBOSE);
	addInfo (misc_info);
    }

    if (due_to_obsolete) {
	setStatus (item, ResStatus::toBeUninstalledDueToObsolete);
    }
    else if (due_to_unlink) {
	setStatus (item, ResStatus::toBeUninstalledDueToUnlink);
    }
    else if (status.wasUninstalled()) {
	setStatus (item, ResStatus::impossible);
    }
    else {
	setStatus (item, ResStatus::toBeUninstalled);
    }

    if (status.wasInstalled()) {
	Resolvable::constPtr res = item.resolvable();
	Package::constPtr pkg = asKind<Package>(res);			// try to access it as a package
	if (pkg) {							// if its !=NULL, get size information
	    _install_size -= pkg->size();
	}
    }

    return true;
}


// change state to UNNEEDED

bool
ResolverContext::unneeded (PoolItem_Ref item, int other_penalty)
{
    ResStatus status;

    _XDEBUG( "ResolverContext[" << this << "]::unneeded(" << item << ")" );

    status = getStatus(item);

    if (status.wasInstalled()) {
	setStatus (item, ResStatus::satisfied);
    }
    else if (status.wasUninstalled()) {
	setStatus (item, ResStatus::unneeded);
    }

    return true;
}


// change state to SATISFIED

bool
ResolverContext::satisfy (PoolItem_Ref item, int other_penalty)
{
    ResStatus status;

    status = getStatus(item);

    _XDEBUG( "ResolverContext[" << this << "]::satisfy(" << item << ":" << status << ")" );

    if (status.wasInstalled()) {
	setStatus (item, ResStatus::complete);
    }
    else if (status.wasUninstalled()) {
	setStatus (item, ResStatus::satisfied);
    }

    return true;
}


// change state to INCOMPLETE

bool
ResolverContext::incomplete (PoolItem_Ref item, int other_penalty)
{
    ResStatus status = getStatus (item);

    _XDEBUG( "ResolverContext[" << this << "]::incomplete(" << item << "):" << status );

    if (_establishing) {
	if (status.wasInstalled()) {
	    setStatus (item, ResStatus::incomplete);
	}
	else {
	    setStatus (item, ResStatus::needed);
	}

	return true;
    }

    // if something goes 'incomplete' outside of the establishing call, its always an error

    ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_INCOMPLETES, item, RESOLVER_INFO_PRIORITY_VERBOSE);
    addError (misc_info);
    return false;
}

//---------------------------------------------------------------------------

// is it installed (after transaction) ?
// if yes, install/requires requests are considered done

bool
ResolverContext::isPresent (PoolItem_Ref item)
{
    ResStatus status = getStatus(item);

_XDEBUG("ResolverContext::itemIsPresent(<" << status << ">" << item << ")");

    return (status.staysInstalled()
	    || (status.isToBeInstalled() && !status.isNeeded())
	    || status.isUnneeded()
	    || status.isSatisfied());
}


// is it uninstalled (after transaction) ?
// if yes, uninstall requests are considered done

bool
ResolverContext::isAbsent (PoolItem_Ref item)
{
    ResStatus status;

    status = getStatus(item);

_XDEBUG("ResolverContext::itemIsAbsent(<" << status << ">" << item << ")");

    // DONT add incomplete here, uninstall requests for incompletes must be handled

    return (status.staysUninstalled()
	   || status.isToBeUninstalled()
	   || status.isImpossible());
}


//---------------------------------------------------------------------------
// marked

void
ResolverContext::foreachMarked (MarkedPoolItemFn fn, void *data) const
{
    ResolverContext_constPtr context = this;
    while (context) {
	for (Context::const_iterator iter = context->_context.begin(); iter != context->_context.end(); ++iter) {
	    fn (iter->first, iter->second, data);
	}
	context = context->_parent;
    }
}


//---------------------------------------------------------------------------
// collect

typedef struct {
    PoolItemList *rl;
    int status;				// <0: uninstalls, ==0: all, >0: installs
} MarkedResolvableInfo;


static void
marked_item_collector (PoolItem_Ref item, const ResStatus & status, void *data)
{
    MarkedResolvableInfo *info = (MarkedResolvableInfo *)data;
    if (info->status == 0
       || (info->status > 0 && status.isToBeInstalled())
       || (info->status < 0 && status.isToBeUninstalled()))
    {
	info->rl->push_back (item);
    }
}


PoolItemList
ResolverContext::getMarked (int which) const
{
    PoolItemList rl;
    MarkedResolvableInfo info = { &rl, which };

    foreachMarked (marked_item_collector, &info);

    return rl;
}

//---------------------------------------------------------------------------
// install

typedef struct {
    ResPool pool;
    MarkedPoolItemFn fn;
    PoolItemList *rl;
    int count;
} InstallInfo;

static void
install_item_cb (PoolItem_Ref item, const ResStatus & status, void *data)
{
    InstallInfo *info = (InstallInfo *)data;

    if (status.isToBeInstalled()
	&& !item.status().isInstalled()
	&& !Helper::findInstalledItem( info->pool, item))
    {
	if (info->fn) info->fn (item, status, info->rl);
	++info->count;
    }
}


int
ResolverContext::foreachInstall (MarkedPoolItemFn fn, void *data) const
{
    PoolItemList *rl = (PoolItemList *)data;
    InstallInfo info = { _pool, fn, rl, 0 };

    foreachMarked (install_item_cb, (void *)&info);

    return info.count;
}


static void
context_item_collector (PoolItem_Ref item, const ResStatus & status, void *data)
{
    PoolItemList *rl = (PoolItemList *)data;
    if (status.isToBeInstalled()
	|| status.isToBeUninstalled())
    {
	rl->push_front (item);
    }
}


PoolItemList
ResolverContext::getInstalls (void) const
{
    PoolItemList rl;

    foreachInstall (context_item_collector, (void *)&rl);

    return rl;
}


//---------------------------------------------------------------------------
// satisfy

typedef struct {
    ResPool pool;
    MarkedPoolItemFn fn;
    PoolItemList *rl;
    int count;
} SatisfyInfo;

static void
satisfy_item_cb (PoolItem_Ref item, const ResStatus & status, void *data)
{
    SatisfyInfo *info = (SatisfyInfo *)data;
    if (status.isSatisfied()
       && ! status.staysInstalled ()
       && !Helper::findInstalledItem (info->pool, item))
    {
       if (info->fn) info->fn (item, status, info->rl);
       ++info->count;
    }
}


int
ResolverContext::foreachSatisfy (MarkedPoolItemFn fn, void *data) const
{
    PoolItemList *rl = (PoolItemList *)data;
    SatisfyInfo info = { _pool, fn, rl, 0 };

    foreachMarked (satisfy_item_cb, (void *)&info);

    return info.count;
}


static void
context_item_collector_satisfy (PoolItem_Ref item, const ResStatus & status, void *data)
{
    PoolItemList *rl = (PoolItemList *)data;
    if (status.isSatisfied ())
    {
       rl->push_front (item);
    }
}


PoolItemList
ResolverContext::getSatisfies (void) const
{
    PoolItemList rl;

    foreachSatisfy (context_item_collector_satisfy, (void *)&rl);

    return rl;
}


//---------------------------------------------------------------------------
// incomplete

typedef struct {
    ResPool pool;
    MarkedPoolItemFn fn;
    PoolItemList *rl;
    int count;
} IncompleteInfo;

static void
incomplete_item_cb (PoolItem_Ref item, const ResStatus & status, void *data)
{
    IncompleteInfo *info = (IncompleteInfo *)data;

    if (status.isIncomplete ()) {
       if (info->fn) info->fn (item, status, info->rl);
       ++info->count;
    }
}


int
ResolverContext::foreachIncomplete (MarkedPoolItemFn fn, void *data) const
{
    PoolItemList *rl = (PoolItemList *)data;
    IncompleteInfo info = { _pool, fn, rl, 0 };

    foreachMarked (incomplete_item_cb, (void *)&info);

    return info.count;
}


static void
context_item_collector_incomplete (PoolItem_Ref item, const ResStatus & status, void *data)
{
    PoolItemList *rl = (PoolItemList *)data;
    if (status.isIncomplete ())
    {
       rl->push_front (item);
    }
}


PoolItemList
ResolverContext::getIncompletes (void) const
{
    PoolItemList rl;

    foreachIncomplete (context_item_collector_incomplete, (void *)&rl);

    return rl;
}


//---------------------------------------------------------------------------
// upgrade

typedef struct {
    ResPool pool;
    MarkedPoolItemPairFn fn;
    void *data;
    ResolverContext_Ptr context;
    int count;
} UpgradeInfo;

static void
upgrade_item_cb (PoolItem_Ref item, const ResStatus & status, void *data)
{
    UpgradeInfo *info = (UpgradeInfo *)data;

    PoolItem_Ref to_be_upgraded;

    if (status.isToBeInstalled()
	&& ! item.status().isInstalled ())
    {
	to_be_upgraded = Helper::findInstalledItem(info->pool, item);
	if (to_be_upgraded) {
	    if (info->fn) {
		info->fn (item, status, to_be_upgraded, info->context->getStatus(to_be_upgraded), info->data);
	    }
	    ++info->count;
	}
    }
}


int
ResolverContext::foreachUpgrade (MarkedPoolItemPairFn fn, void *data)
{
    UpgradeInfo info = { _pool, fn, data, this, 0 };

    foreachMarked (upgrade_item_cb, (void *)&info);

    return info.count;
}


static void
pair_item_collector (PoolItem_Ref item, const ResStatus & status, PoolItem_Ref old_item, const ResStatus & old_status, void *data)
{
    PoolItemList *rl = (PoolItemList *)data;
    rl->push_back (item);
}


PoolItemList
ResolverContext::getUpgrades (void)
{
    PoolItemList rl;

    foreachUpgrade (pair_item_collector, (void *)&rl);

    return rl;
}


//---------------------------------------------------------------------------
// uninstall

typedef std::map<std::string,PoolItem_Ref> UpgradeTable;

typedef struct {
    MarkedPoolItemFn fn;
    PoolItemList *rl;
    UpgradeTable upgrade_hash;
    int count;
} UninstallInfo;

static void
uninstall_item_cb (PoolItem_Ref item, const ResStatus & status, void *data)
{
    UninstallInfo *info = (UninstallInfo *)data;

    UpgradeTable::const_iterator pos = info->upgrade_hash.find(item->name());

    if (status.isToBeUninstalled ()
	&& pos == info->upgrade_hash.end())		// dont count upgrades
    {
	if (info->fn)
	    info->fn (item, status, info->rl);
	++info->count;
    }
}


static void
build_upgrade_hash_cb (PoolItem_Ref item_add, const ResStatus & add_status, PoolItem_Ref item_del, const ResStatus & del_status, void *data)
{
    UpgradeTable *upgrade_hash = (UpgradeTable *)data;
    (*upgrade_hash)[item_del->name()] = item_del;
}


int
ResolverContext::foreachUninstall (MarkedPoolItemFn fn, void *data)
{
    UninstallInfo info;		// inits upgrade_hash

    info.fn = fn;
    info.rl = (PoolItemList *)data;
    info.count = 0;

    foreachUpgrade (build_upgrade_hash_cb, (void *)&(info.upgrade_hash));
    foreachMarked (uninstall_item_cb, (void *)&info);

    return info.count;
}


PoolItemList
ResolverContext::getUninstalls (void)
{
    PoolItemList rl;

    foreachUninstall (context_item_collector, (void *)&rl);

    return rl;
}


//---------------------------------------------------------------------------
// impossible

typedef struct {
    ResPool pool;
    MarkedPoolItemFn fn;
    int count;
    void *data;
} ImpossibleInfo;

static void
impossible_item_cb (PoolItem_Ref item, const ResStatus & status, void *data)
{
    ImpossibleInfo *info = (ImpossibleInfo *)data;

    if (status.isImpossible ()) {
       if (info->fn) info->fn (item, status, info->data);
       ++info->count;
    }
}


int
ResolverContext::foreachImpossible (MarkedPoolItemFn fn, void *data)
{
    ImpossibleInfo info = { _pool, fn, 0, data };

    foreachMarked (impossible_item_cb, (void *)&info);

    return info.count;
}


//---------------------------------------------------------------------------

static void
install_count_cb (PoolItem_Ref item, const ResStatus & status, void *data)
{
    int *count = (int *)data;
    if (!item.status().isInstalled ()) {
	++*count;
    }
}

int
ResolverContext::installCount (void) const
{
    int count = 0;

    foreachInstall (install_count_cb, (void *)&count);

    return count;
}


static void
uninstall_count_cb (PoolItem_Ref item, const ResStatus & status, void *data)
{
    int *count = (int *)data;
    if (item.status().isInstalled ()) {
	++*count;
    }
}


int
ResolverContext::uninstallCount (void)
{
    int count = 0;

    foreachUninstall (uninstall_count_cb, (void *)&count);

    return count;
}


int
ResolverContext::upgradeCount (void)
{
    return foreachUpgrade ((MarkedPoolItemPairFn)NULL, (void *)NULL);
}


static void
satisfy_count_cb (PoolItem_Ref item, const ResStatus & status, void *data)
{
    int *count = (int *)data;
    if (!item.status().isInstalled ()) {
	++*count;
    }
}

int
ResolverContext::satisfyCount (void) const
{
    int count = 0;

    foreachSatisfy (satisfy_count_cb, (void *)&count);

    return count;
}


int
ResolverContext::incompleteCount (void) const
{
    return foreachIncomplete ((MarkedPoolItemFn)NULL, (void *)NULL);
}



//---------------------------------------------------------------------------
// info

void
ResolverContext::addInfo (ResolverInfo_Ptr info)
{
    _XDEBUG( "ResolverContext[" << this << "]::addInfo(" << *info << ")" );
    _log.push_back (info);

    // _propagated_importance = false;

    if (info->error ()) {

	if (! _invalid) {
	    ResolverInfo_Ptr info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_INVALID_SOLUTION, PoolItem_Ref(), RESOLVER_INFO_PRIORITY_VERBOSE);
	    info->flagAsError ();
	    _log.push_back (info);
	}

	_invalid = true;
    }
}


void
ResolverContext::addError (ResolverInfo_Ptr info)
{
    info->flagAsError ();
    WAR << "******** Error: " << *info << endl;
    addInfo (info);
}


//---------------------------------------------------------------------------
// foreach info

//  We call a item mentioned by an error info an "error-item".
//  We call a item mentioned by an important info an "important-item".
//
//  The rules:
//  (1) An info item that mentions an error-item is important.
//  (2) An info item is about an important-item is important.

static void
mark_important_info (ResolverInfoList & il)
{
    // set of all items mentioned in the ResolverInfoList
    PoolItemSet error_set;

    bool did_something;
    int pass_num = 1;

    /* First of all, store all error-items in a set. */

    for (ResolverInfoList::iterator info_iter = il.begin(); info_iter != il.end(); ++info_iter) {
	ResolverInfo_Ptr info = (*info_iter);
	if (info != NULL						// list items might be NULL
	    && info->error ()) {					// only look at error infos

	    PoolItem_Ref item = info->affected();		// get item from ResolverInfoList
	    if (item) {
		error_set.insert (item);
	    }

	    // the info might be a container, check it by doing a dynamic cast

	    PoolItemList containerItems;
	    ResolverInfoContainer_constPtr c = dynamic_pointer_cast<const ResolverInfoContainer>(*info_iter);
	    if (c != NULL) containerItems = c->items();

	    // containerItems is non-empty if info is really a ResolverInfoContainer

	    for (PoolItemList::iterator res_iter = containerItems.begin(); res_iter != containerItems.end(); res_iter++) {
		PoolItem_Ref item = (*res_iter);
		if (item) {
		    error_set.insert (item);
		}
	    }
	}
    }

    // now collect all important ones

    PoolItemSet important_set;

    do {
	++pass_num;
	assert (pass_num < 10000);

	did_something = false;

	for (ResolverInfoList::iterator info_iter = il.begin(); info_iter != il.end(); ++info_iter) {
	    ResolverInfo_Ptr info = (*info_iter);
	    if (info != NULL						// list items might be set to NULL
		&& info->important ()) {				// only look at important ones
		bool should_be_important = false;

		for (PoolItemSet::const_iterator res_iter = error_set.begin(); res_iter != error_set.end() && ! should_be_important; ++res_iter) {
		    ResolverInfoContainer_constPtr c = dynamic_pointer_cast<const ResolverInfoContainer>(*info_iter);
		    if (c != NULL					// check if it really is a container
			&& c->mentions (*res_iter)) {
			should_be_important = true;
		    }
		}

		for (PoolItemSet::const_iterator res_iter = important_set.begin(); res_iter != important_set.end() && ! should_be_important; ++res_iter) {
		    if (info->isAbout (*res_iter)) {
			should_be_important = true;
			break;
		    }
		}

		if (should_be_important) {
		    did_something = true;
		    info->flagAsImportant ();
		    PoolItemList items;
		    ResolverInfoContainer_constPtr c = dynamic_pointer_cast<const ResolverInfoContainer>(*info_iter);		// check if it really is a container
		    if (c != NULL) items = c->items();
		    for (PoolItemList::iterator res_iter = items.begin(); res_iter != items.end(); res_iter++) {
			important_set.insert (*res_iter);
		    }
		}
	    }
	}

    } while (did_something);

}

void
ResolverContext::foreachInfo (PoolItem_Ref item, int priority, ResolverInfoFn fn, void *data) const
{
    ResolverInfoList info_list;

    ResolverContext_constPtr context = this;
     // Assemble a list of copies of all of the info objects
    while (context != NULL) {
	for (ResolverInfoList::const_iterator iter = context->_log.begin(); iter != context->_log.end(); iter++) {
	    if ((item == PoolItem_Ref() || (*iter)->affected() == item)
		&& (*iter)->priority() >= priority) {
		info_list.push_back ((*iter)->copy());
	    }
	}
	context = context->_parent;
    }
#if 0
    // Merge info objects
    for (ResolverInfoList::iterator iter = info_list.begin(); iter != info_list.end(); iter++) {

	ResolverInfo_Ptr info1 = (*iter);
	ResolverInfoList::iterator subiter = iter;
	if (info1 != NULL) {
	    for (subiter++; subiter != info_list.end(); subiter++) {
		ResolverInfo_Ptr info2 = *subiter;
		if (info2 && info1->merge (info2)) {
		    *subiter = NULL;
		}
	    }
	}
    }
#endif
    mark_important_info (info_list);

    // Walk across the list of info objects and invoke our callback

    for (ResolverInfoList::iterator iter = info_list.begin(); iter != info_list.end(); iter++) {
	if (*iter != NULL) {
	    fn (*iter, data);
	}
    }
}



static void
get_info_foreach_cb (ResolverInfo_Ptr info, void *data)
{
    ResolverInfoList *il = (ResolverInfoList *)data;

    if (info->important ()) {
	il->push_back (info);
    }
}



ResolverInfoList
ResolverContext::getInfo (void) const
{
    ResolverInfoList il;
    foreachInfo (PoolItem_Ref(), -1, get_info_foreach_cb, (void *)&il);
    return il;
}


//---------------------------------------------------------------------------
// spew

static void
spew_item_cb (PoolItem_Ref item, const ResStatus & status, void *unused)
{
    MIL << "  " << item << " (" << status << ")" << endl;
}


void
spew_item_pair_cb (PoolItem_Ref item1, const ResStatus & status1, PoolItem_Ref item2, const ResStatus & status2, void *unused)
{
    MIL << "  " << item2 << " (" << status2 << ") => (" << item1 << " (" << status2 << ")" << endl;
}


void
ResolverContext::spew (void)
{
    MIL << "TO INSTALL:" << endl;
    foreachInstall (spew_item_cb, NULL);
    MIL << endl;

    MIL << "TO REMOVE:" << endl;
    foreachUninstall (spew_item_cb, NULL);
    MIL << endl;

    MIL << "TO UPGRADE:" << endl;
    foreachUpgrade (spew_item_pair_cb, NULL);
    MIL << endl;
}


static void
spew_info_cb (ResolverInfo_Ptr info, void *unused)
{
    if (info->error ()) MIL << "[ERROR] )";
    else if (info->important()) MIL << "[>>>>>] )";
    MIL << *info << endl;
}


void
ResolverContext::spewInfo (void) const
{
    _XDEBUG( "ResolverContext[" << this << "]::spewInfo" );
    foreachInfo (PoolItem_Ref(), -1, spew_info_cb, NULL);
}

//---------------------------------------------------------------------------
// requirements

struct RequirementMet
{
    ResolverContext_Ptr context;
    const Capability capability;
    bool flag;

    RequirementMet (ResolverContext_Ptr ctx, const Capability & c)
	: context (ctx)
	, capability (c)
	, flag (false)
    { }


    bool operator()( const CapAndItem & cai )
    {
	Capability match( cai.cap );
	PoolItem provider( cai.item );
	// capability is set for item set children. If it is set, query the
	//   exact version only.
	if ((capability == Capability::noCap
	     || capability == match)
	    && context->isPresent (provider))
	{
	    flag = true;
	}

//	ERR << "RequirementMet(" <<  item << ", " << cap << ") [capability " <<
//	  capability << "] -> " <<  (flag ? "true" : "false") << endl;

	return ! flag;
    }
};


bool
ResolverContext::requirementIsMet (const Capability & dependency, bool is_child)
{
    RequirementMet info (this, is_child ? dependency : Capability::noCap);

    //    world()->foreachProviding (dependency, requirement_met_cb, (void *)&info);

    Dep dep( Dep::PROVIDES );

    // world->foreachProvidingResItem (dependency, require_process_cb, &info);

    invokeOnEach( pool().byCapabilityIndexBegin( dependency.index(), dep ),
		  pool().byCapabilityIndexEnd( dependency.index(), dep ),
		  resfilter::ByCapMatch( dependency ),
		  functor::functorRef<bool,CapAndItem>(info) );
    return info.flag;
}


//---------------------------------------------------------------------------

struct RequirementPossible
{
    ResolverContext_Ptr context;
    bool flag;

    RequirementPossible( ResolverContext_Ptr ctx )
	: context (ctx)
	, flag (false)
    { }

    bool operator()( const CapAndItem & cai )
    {
	PoolItem provider( cai.item );
	ResStatus status = context->getStatus( provider );

	if (! (status.isToBeUninstalled () || status.isImpossible())
	    || status.isToBeUninstalledDueToUnlink())
	{
	    flag = true;
	}

	return ! flag;
    }
};


bool
ResolverContext::requirementIsPossible (const Capability & dependency)
{
    RequirementPossible info( this );

    // world()->foreachProviding (dep, requirement_possible_cb, (void *)&info);

    Dep dep( Dep::PROVIDES );

    invokeOnEach( pool().byCapabilityIndexBegin( dependency.index(), dep ),
		  pool().byCapabilityIndexEnd( dependency.index(), dep ),
		  resfilter::ByCapMatch( dependency ),
		  functor::functorRef<bool,CapAndItem>(info) );
    _XDEBUG("requirementIsPossible( " << dependency << ") = " << (info.flag ? "Y" : "N"));
    return info.flag;
}


bool
ResolverContext::itemIsPossible (PoolItem_Ref item)
{
    CapSet requires = item->dep (Dep::REQUIRES);
    for (CapSet::iterator iter = requires.begin(); iter !=  requires.end(); iter++) {
	if (! requirementIsPossible (*iter)) {
	    return false;
	}
    }

    return true;
}

//---------------------------------------------------------------------------

typedef struct {
    PoolItem_Ref other;
    bool flag;
    PoolItem_Ref foundItem;
} DupNameCheckInfo;

static void
dup_name_check_cb (PoolItem_Ref item, const ResStatus & status, void *data)
{
    DupNameCheckInfo *info = (DupNameCheckInfo *)data;
    if (! info->flag
	&& status.isToBeInstalled ()
	&& info->other->name() == item->name()
	&& item->edition().compare(info->other->edition()) != 0
	&& item->arch() == info->other->arch())
    {
	info->flag = true;
	info->foundItem = item;
    }
}

bool
ResolverContext::isParallelInstall (PoolItem_Ref item) const
{
    DupNameCheckInfo info;

    info.other = item;
    info.flag = false;
    foreachMarked (dup_name_check_cb, (void *)&info);
    if (info.flag) {
	DBG << "isParallelInstall!!(" << item << ", " << info.foundItem << ")" << endl;
    }
    return info.flag;
}

PoolItem_Ref
ResolverContext::getParallelInstall (PoolItem_Ref item) const
{
    DupNameCheckInfo info;

    info.other = item;
    info.flag = false;
    foreachMarked (dup_name_check_cb, (void *)&info);
    return info.foundItem;
}


int
ResolverContext::getSourcePriority (Source_Ref source) const
{
//    bool is_subscribed;
    int priority = 0;
#warning getSourcePriority not implemented
//    is_subscribed = channel->isSubscribed ();
//    priority = channel->getPriority (is_subscribed);

    return priority;
}

//---------------------------------------------------------------------------

static int
num_cmp (double a, double b)
{
    return (b < a) - (a < b);
}

static int
rev_num_cmp (double a, double b)
{
    return (a < b) - (b < a);
}

static double
churn_factor (ResolverContext_Ptr a)
{
    return a->upgradeCount() + (2.0 * a->installCount ()) + (4.0 * a->uninstallCount ());
}

int
ResolverContext::partialCompare (ResolverContext_Ptr context)
{
    int cmp = 0;
    if (this != context) {

	// High numbers are good... we don't want solutions containing low-priority channels.
	cmp = num_cmp (_min_priority, context->_min_priority);

	if (cmp == 0) {

	    // High numbers are bad.  Less churn is better.
	    cmp = rev_num_cmp (churn_factor (this), churn_factor (context));

	    if (cmp == 0) {

		// High numbers are bad.  Bigger #s means more penalties.
		cmp = rev_num_cmp (_other_penalties, context->_other_penalties);
	    }
	}
    }

    return cmp;
}

int
ResolverContext::compare (ResolverContext_Ptr context)
{
    int cmp;

    if (this == context)
	return 0;

    cmp = partialCompare (context);
    if (cmp)
	return cmp;

    /* High numbers are bad.  Smaller downloads are best. */
    cmp = rev_num_cmp (_download_size, context->_download_size);
    if (cmp)
	return cmp;

    /* High numbers are bad.  Less disk space consumed is good. */
    cmp = rev_num_cmp (_install_size, context->_install_size);
    if (cmp)
	return cmp;

    return 0;
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

