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

ResolverContext::ResolverContext (const ResPool & pool, ResolverContext_Ptr parent)
    : _parent (parent)
    , _pool (pool)
    , _download_size (0)
    , _install_size (0)
    , _total_priority (0)
    , _min_priority (0)
    , _max_priority (0)
    , _other_penalties (0)
    , _verifying (false)
    , _invalid (false)
{
DBG << "ResolverContext[" << this << "]::ResolverContext(" << parent << ")" << endl;
    if (parent != NULL) {
	_pool		 = parent->_pool;
	_download_size   = parent->_download_size;
	_install_size    = parent->_install_size;
	_total_priority  = parent->_total_priority;
	_max_priority    = parent->_max_priority;
	_min_priority    = parent->_min_priority;
	_other_penalties = parent->_other_penalties;
	_verifying	 = parent->_verifying;
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
ResolverContext::getStatus (PoolItem_Ref item) const
{
DBG << "[" << this << "]getStatus(" << item << ")" << endl;
    Context::const_iterator it;
    ResolverContext_constPtr context = this;

    while (context) {					// go through the _parent chain

	it = context->_context.find(item);		// part of local context ?
	if (it != context->_context.end()) {
DBG << "[" << context << "]:" << it->second << endl;
	    return it->second;				// Y: return
	}
	context = context->_parent;			// N: go up the chain
    }

DBG << "[NULL]:" << item.status() << endl;
    return item.status();				// Not part of context, return Pool status
}


//---------------------------------------------------------------------------
// status change

void
ResolverContext::setStatus (PoolItem_Ref item, const ResStatus & status)
{
    if (_invalid) return;
DBG << "[" << this << "]setStatus(" << item << ", " << status << ")" << endl;
    if (status == item.status()) {		// same as original status
	Context::iterator it;
	it = _context.find(item);		// part of local context ?
	if (it != _context.end()) {
	    _context.erase (it);		// erase it !
	}
    }
    else {
	ResStatus old_status = getStatus (item);

	if (old_status != status) {		// new status ?
	    _context[item] = status;		// set it !
	}
    }
    return;
}


// change state to TO_BE_INSTALLED (does not upgrade)

bool
ResolverContext::install (PoolItem_Ref item, bool is_soft, int other_penalty)
{
    ResStatus status, new_status;
    std::string msg;

    status = getStatus(item);
    DBG << "ResolverContext[" << this << "]::install(<" << status  << "> " << item << ")" << endl;

    if (status.isToBeUninstalled()
	&& !status.isToBeUninstalledDueToUnlink()) {
	ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_INSTALL_TO_BE_UNINSTALLED, item, RESOLVER_INFO_PRIORITY_VERBOSE);
	addError (misc_info);
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
	ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_INSTALL_PARALLEL, item, RESOLVER_INFO_PRIORITY_VERBOSE);
	addError (misc_info);
	return false;
    }

    if (is_soft)
	setStatus (item, ResStatus::toBeInstalledSoft);
    else if (status.isToBeUninstalledDueToUnlink())
	setStatus (item, ResStatus(true));
    else
	setStatus (item, ResStatus::toBeInstalled);

    if (status.isUninstalled()) {
#if 0
	Package::constPtr pkg = asKind<Package>(item.resolvable());	// try to access it as a package
	if (pkg) {							// if its !=NULL, get size information

	    _download_size += pkg->archivesize();
	    _install_size += pkg->sourcesize();

	}
#endif
#warning Needs source backref
#if 0
	int priority;
	if (item->local())
	    priority = 0;
	else {
	    priority = getChannelPriority (item->channel ());
	}

	if (priority < _min_priority) _min_priority = priority;
	if (priority > _max_priority) _max_priority = priority;
#endif
	_other_penalties += other_penalty;

    }

    return true;
}


// change state to TO_BE_INSTALLED (does upgrade)

bool
ResolverContext::upgrade (PoolItem_Ref item, PoolItem_Ref old_item, bool is_soft, int other_penalty)
{
    ResStatus status;

    DBG << "ResolverContext[" << this << "]::upgrade(" << item << " upgrades " << old_item << ")" << endl;

    status = getStatus(item);

    if (status.isToBeUninstalled())
	return false;

    if (status.isToBeInstalled())
	return true;

    if (is_soft) {
	setStatus (item, ResStatus::toBeInstalledSoft);
    }
    else {
	setStatus (item, ResStatus::toBeInstalled);
    }

    if (status.isUninstalled()) {
#if 0
	Package::constPtr pkg = asKind<Package>(item.resolvable());	// try to access it as a package
	if (pkg) {							// if its !=NULL, get size information

	    _download_size += pkg->archivesize();
	    _install_size += pkg->sourcesize();

	// FIXME: Incomplete
	// We should change installed_size to reflect the difference in
	//   installed size between the old and new versions.
	}
#endif
#warning Needs source backref
#if 0
	int priority;
	if (item->local())
	    priority = 0;
	else {
	    priority = getChannelPriority (item->channel());
	}

	if (priority < _min_priority) _min_priority = priority;
	if (priority > _max_priority) _max_priority = priority;
#endif
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

    DBG << "ResolverContext[" << this << "]::uninstall("
		    << item << " " << (part_of_upgrade ? "part_of_upgrade" : "") << " "
		    << (due_to_obsolete ? "due_to_obsolete": "") << " "
		    << (due_to_unlink ? "due_to_unlink" : "") << ")" << endl;

    assert (! (due_to_obsolete && due_to_unlink));

    status = getStatus(item);

    if (status.isToBeInstalled()) {
	ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_REJECT_INSTALL, item, RESOLVER_INFO_PRIORITY_VERBOSE);
	addError (misc_info);
	return false;
    }

    if (status.isToBeUninstalled()
	&& !status.isToBeUninstalledDueToUnlink()) {
	return true;
    }

    if (status.isUninstalled()
	|| status.isToBeUninstalledDueToUnlink()) {
	ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_UNINSTALLABLE, item, RESOLVER_INFO_PRIORITY_VERBOSE);
	addInfo (misc_info);
    }

    if (due_to_obsolete) {
	setStatus (item, ResStatus::toBeUninstalledDueToObsolete);
    }
    else if (due_to_unlink) {
	setStatus (item, ResStatus::toBeUninstalledDueToUnlink);
    }
    else {
	setStatus (item, ResStatus::toBeUninstalled);
    }

    if (status.isInstalled()) {
#if 0
	Package::constPtr pkg = asKind<Package>(item.resolvable());	// try to access it as a package
	if (pkg) {							// if its !=NULL, get size information
	    _install_size -= pkg->sourcesize();
	}
#endif
    }

    return true;
}


// change state to UNNEEDED

bool
ResolverContext::unneeded (PoolItem_Ref item, int other_penalty)
{
    ResStatus status;

    DBG << "ResolverContext[" << this << "]::unneeded(" << item << ")" << endl;

    status = getStatus(item);

    if (status.isInstalled()) {
	setStatus (item, ResStatus::satisfied);
    }
    else {
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

    DBG << "ResolverContext[" << this << "]::satisfy(" << item << ":" << status << ")" << endl;

    if (status.isInstalled()) {
	setStatus (item, ResStatus::complete);
    }
    else {
	setStatus (item, ResStatus::satisfied);
    }

    return true;
}


// change state to INCOMPLETE

bool
ResolverContext::incomplete (PoolItem_Ref item, int other_penalty)
{
    DBG << "ResolverContext[" << this << "]::incomplete(" << item << ")" << endl;

    ResStatus status;
    status = getStatus(item);

    if (status.isInstalled()) {
	ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_INCOMPLETES, item, RESOLVER_INFO_PRIORITY_VERBOSE);
	addError (misc_info);
	return false;
    }
    else if (status.isUnneeded()) {
	ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_INCOMPLETES, item, RESOLVER_INFO_PRIORITY_VERBOSE);
	addError (misc_info);
	return false;
    }

    if (status.isInstalled()) {
	setStatus (item, ResStatus::incomplete);
    }
    else {
	setStatus (item, ResStatus::needed);
    }

    return true;
}

//---------------------------------------------------------------------------

// is it installed (after transaction) ?
// if yes, install/requires requests are considered done
bool
ResolverContext::isPresent (PoolItem_Ref item) const
{
    ResStatus status = getStatus(item);

ERR << "ResolverContext::itemIsPresent(<" << status << ">" << item << ")" << endl;

    return (status.staysInstalled()
	    || status.isToBeInstalled()
	    || status.isToBeInstalledSoft()
	    || status.isSatisfied ());
}


// is it uninstalled (after transaction) ?
// if yes, uninstall requests are considered done

bool
ResolverContext::isAbsent (PoolItem_Ref item) const
{
    ResStatus status;

    status = getStatus(item);

    // DONT add incomplete here, uninstall requests for incompletes must be handled

    return (status.isUninstalled()
	   || status.isToBeUninstalled());
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
install_pkg_cb (PoolItem_Ref item, const ResStatus & status, void *data)
{
    InstallInfo *info = (InstallInfo *)data;
    if (status.isToBeInstalled()
	&& ! status.isInstalled ()
	&& !Helper::findInstalledItem (info->pool, item))
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

    foreachMarked (install_pkg_cb, (void *)&info);

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
satisfy_pkg_cb (PoolItem_Ref item, const ResStatus & status, void *data)
{
    SatisfyInfo *info = (SatisfyInfo *)data;
    if (status.isSatisfied()
       && ! status.isInstalled ()
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

    foreachMarked (satisfy_pkg_cb, (void *)&info);

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
incomplete_pkg_cb (PoolItem_Ref item, const ResStatus & status, void *data)
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

    foreachMarked (incomplete_pkg_cb, (void *)&info);

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
    ResolverContext_constPtr context;
    int count;
} UpgradeInfo;

static void
upgrade_pkg_cb (PoolItem_Ref item, const ResStatus & status, void *data)
{
    UpgradeInfo *info = (UpgradeInfo *)data;

    PoolItem_Ref to_be_upgraded;

    if (status.isToBeInstalled()
	&& ! status.isInstalled ())
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
ResolverContext::foreachUpgrade (MarkedPoolItemPairFn fn, void *data) const
{
    UpgradeInfo info = { _pool, fn, data, this, 0 };

    foreachMarked (upgrade_pkg_cb, (void *)&info);

    return info.count;
}


static void
pair_item_collector (PoolItem_Ref item, const ResStatus & status, PoolItem_Ref old_item, const ResStatus & old_status, void *data)
{
    PoolItemList *rl = (PoolItemList *)data;
    rl->push_back (item);
}


PoolItemList
ResolverContext::getUpgrades (void) const
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
uninstall_pkg_cb (PoolItem_Ref item, const ResStatus & status, void *data)
{
    UninstallInfo *info = (UninstallInfo *)data;

    UpgradeTable::const_iterator pos = info->upgrade_hash.find(item->name());

    if (status.isToBeUninstalled ()
	&& pos == info->upgrade_hash.end())
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
ResolverContext::foreachUninstall (MarkedPoolItemFn fn, void *data) const
{
    UninstallInfo info;		// inits upgrade_hash

    info.fn = fn;
    info.rl = (PoolItemList *)data;
    info.count = 0;

    foreachUpgrade (build_upgrade_hash_cb, (void *)&(info.upgrade_hash));
    foreachMarked (uninstall_pkg_cb, (void *)&info);

    return info.count;
}


PoolItemList
ResolverContext::getUninstalls (void) const
{
    PoolItemList rl;

    foreachUninstall (context_item_collector, (void *)&rl);

    return rl;
}


//---------------------------------------------------------------------------

static void
install_count_cb (PoolItem_Ref item, const ResStatus & status, void *data)
{
    int *count = (int *)data;
    if (! item.status().isInstalled ()) {
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
ResolverContext::uninstallCount (void) const
{
    int count = 0;

    foreachUninstall (uninstall_count_cb, (void *)&count);

    return count;
}


int
ResolverContext::upgradeCount (void) const
{
    return foreachUpgrade ((MarkedPoolItemPairFn)NULL, (void *)NULL);
}


static void
satisfy_count_cb (PoolItem_Ref item, const ResStatus & status, void *data)
{
    int *count = (int *)data;
    if (! item.status().isInstalled ()) {
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
    DBG << "ResolverContext[" << this << "]::addInfo(" << *info << ")" << endl;
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
	    if ((item || (*iter)->affected() == item)
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
ResolverContext::spew (void) const
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
    MIL << info << endl;
}


void
ResolverContext::spewInfo (void) const
{
    DBG << "ResolverContext[" << this << "]::spewInfo" << endl;
    foreachInfo (PoolItem_Ref(), -1, spew_info_cb, NULL);
}

//---------------------------------------------------------------------------
// requirements

struct RequirementMet : public resfilter::OnCapMatchCallbackFunctor
{
    ResolverContext_constPtr context;
    const Capability & capability;
    bool flag;

    RequirementMet (ResolverContext_constPtr ctx, const Capability & c)
	: context (ctx)
	, capability (c)
	, flag (false)
    { }


    bool operator()( PoolItem_Ref provider, const Capability & match )
    {
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
ResolverContext::requirementIsMet (const Capability & dependency, bool is_child) const
{
    RequirementMet info (this, is_child ? dependency : Capability::noCap);

    //    world()->foreachProviding (dependency, requirement_met_cb, (void *)&info);

    Dep dep( Dep::PROVIDES );

    // world->foreachProvidingResItem (dependency, require_process_cb, &info);

    invokeOnEach( pool().byCapabilityIndexBegin( dependency.index(), dep ),
		  pool().byCapabilityIndexEnd( dependency.index(), dep ),
		  resfilter::callOnCapMatchIn( dep, dependency, functor::functorRef<bool,PoolItem,Capability>(info) ) );

    return info.flag;
}


//---------------------------------------------------------------------------

struct RequirementPossible : public resfilter::OnCapMatchCallbackFunctor
{
    bool flag;

    RequirementPossible ()
	: flag (false)
    { }

    bool operator()( PoolItem_Ref provider, const Capability & match )
    {
	ResStatus status = provider.status();

	if (! status.isToBeUninstalled ()
	    || status.isToBeUninstalledDueToUnlink())
	{
	    flag = true;
	}

	return ! flag;
    }
};


bool
ResolverContext::requirementIsPossible (const Capability & dependency) const
{
    RequirementPossible info;

    // world()->foreachProviding (dep, requirement_possible_cb, (void *)&info);

    Dep dep( Dep::PROVIDES );

    // world->foreachProvidingResItem (dependency, require_process_cb, &info);

    invokeOnEach( pool().byCapabilityIndexBegin( dependency.index(), dep ),
		  pool().byCapabilityIndexEnd( dependency.index(), dep ),
		  resfilter::callOnCapMatchIn( dep, dependency, functor::functorRef<bool,PoolItem,Capability>(info) ) );

    return info.flag;
}


bool
ResolverContext::itemIsPossible (PoolItem_Ref item) const
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
} DupNameCheckInfo;

static void
dup_name_check_cb (PoolItem_Ref item, const ResStatus & status, void *data)
{
    DupNameCheckInfo *info = (DupNameCheckInfo *)data;
    if (! info->flag
	&& status.isToBeInstalled ()
	&& info->other->name() == item->name()
	&& !compareByNVR (item.resolvable(), info->other.resolvable()))
    {
	info->flag = true;
    }
}

bool
ResolverContext::isParallelInstall (PoolItem_Ref item) const
{
    DupNameCheckInfo info;

    info.other = item;
    info.flag = false;
    foreachMarked (dup_name_check_cb, (void *)&info);

    return info.flag;
}

#warning needs source backref
#if 0
int
ResolverContext::getChannelPriority (Channel_constPtr channel) const
{
    bool is_subscribed;
    int priority;

    is_subscribed = channel->isSubscribed ();
    priority = channel->getPriority (is_subscribed);

    return priority;
}
#endif

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
churn_factor (ResolverContext_constPtr a)
{
    return a->upgradeCount() + (2.0 * a->installCount ()) + (4.0 * a->uninstallCount ());
}

int
ResolverContext::partialCompare (ResolverContext_constPtr context) const
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
ResolverContext::compare (ResolverContext_constPtr context) const
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

