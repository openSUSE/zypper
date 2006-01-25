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

#include "zypp/solver/detail/Ptr.h"
#include "zypp/solver/detail/ResolverContext.h"

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

ostream&
operator<<( ostream& os, const ResolverContext & context)
{
    for (ResolverContext::Context::const_iterator iter = context._context.begin(); iter != context._context.end(); ++iter) {
	os << *iter << endl;
    }
    return os;
}

//---------------------------------------------------------------------------

ResolverContext::ResolverContext (ResolverContext_Ptr parent)
    : _parent (parent)
    , _download_size (0)
    , _install_size (0)
    , _total_priority (0)
    , _min_priority (0)
    , _max_priority (0)
    , _other_penalties (0)
    , _verifying (false)
    , _invalid (false)
{
    if (parent != NULL) {
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
// status change

void
ResolverContext::setStatus (PoolItem & item, ResStatus & status)
{
#if 0
	item.setStatus (status);
#endif
	_context.insert (&item);
}


#if 0
// change state to TO_BE_INSTALLED (does not upgrade)

bool
ResolverContext::installResItem (ResItem_constPtr resItem, bool is_soft, int other_penalty)
{
    ResItemStatus status, new_status;
    int priority;
    std::string msg;

    status = getStatus (resItem);
    _DBG("RC_SPEW") << "ResolverContext[" << this << "]::installResItem(<" << ResolverContext::toString(status)
		    << "> " << resItem->asString() << ")" << endl;

    if (resItem_status_is_to_be_uninstalled (status)
	&& status != RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_UNLINK) {
	ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_INSTALL_TO_BE_UNINSTALLED, resItem, RESOLVER_INFO_PRIORITY_VERBOSE);
	addError (misc_info);
	return false;
    }

    if (resItem_status_is_unneeded (status)) {
	ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_INSTALL_UNNEEDED, resItem, RESOLVER_INFO_PRIORITY_VERBOSE);
	addError (misc_info);
	return false;
    }

    if (resItem_status_is_to_be_installed (status)) {
	return true;
    }

    if (isParallelInstall (resItem)) {
	ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_INSTALL_PARALLEL, resItem, RESOLVER_INFO_PRIORITY_VERBOSE);
	addError (misc_info);
	return false;
    }

    if (is_soft)
	new_status = RESOLVABLE_STATUS_TO_BE_INSTALLED_SOFT;
    else if (status == RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_UNLINK)
	new_status = RESOLVABLE_STATUS_INSTALLED;
    else
	new_status = RESOLVABLE_STATUS_TO_BE_INSTALLED;

    setStatus (resItem, new_status);

    if (status == RESOLVABLE_STATUS_UNINSTALLED) {
	/* FIXME: Incomplete */
	_download_size += resItem->fileSize();
	_install_size += resItem->installedSize();

	if (resItem->local())
	    priority = 0;
	else {
	    priority = getChannelPriority (resItem->channel ());
	}

	if (priority < _min_priority) _min_priority = priority;
	if (priority > _max_priority) _max_priority = priority;

	_other_penalties += other_penalty;

    }

    return true;
}


// change state to TO_BE_INSTALLED (does upgrade)

bool
ResolverContext::upgradeResItem (ResItem_constPtr resItem, ResItem_constPtr old_resItem, bool is_soft, int other_penalty)
{
    ResItemStatus status;
    int priority;

    _DBG("RC_SPEW") << "ResolverContext[" << this << "]::upgradeResItem(" << resItem->asString() << " upgrades "
		    << old_resItem->asString() << ")" << endl;

    status = getStatus (resItem);

    if (resItem_status_is_to_be_uninstalled (status))
	return false;

    if (resItem_status_is_to_be_installed (status))
	return true;

    setStatus (resItem, is_soft ? RESOLVABLE_STATUS_TO_BE_INSTALLED_SOFT : RESOLVABLE_STATUS_TO_BE_INSTALLED);

    if (status == RESOLVABLE_STATUS_UNINSTALLED) {

	_download_size += resItem->fileSize();

	// FIXME: Incomplete
	// We should change installed_size to reflect the difference in
	//   installed size between the old and new versions.

	if (resItem->local())
	    priority = 0;
	else {
	    priority = getChannelPriority (resItem->channel());
	}

	if (priority < _min_priority) _min_priority = priority;
	if (priority > _max_priority) _max_priority = priority;

	_other_penalties += other_penalty;
    }

    return true;
}


// change state to 'TO_BE_UNINSTALLED{, DUE_TO_OBSOLETE, DUE_TO_UNLINK}'

bool
ResolverContext::uninstallResItem (ResItem_constPtr resItem, bool part_of_upgrade, bool due_to_obsolete, bool due_to_unlink)
{
    ResItemStatus status, new_status;
    std::string msg;

    _DBG("RC_SPEW") << "ResolverContext[" << this << "]::uninstallResItem("
		    << resItem->asString() << " " << (part_of_upgrade ? "part_of_upgrade" : "") << " "
		    << (due_to_obsolete ? "due_to_obsolete": "") << " "
		    << (due_to_unlink ? "due_to_unlink" : "") << ")" << endl;

    assert (! (due_to_obsolete && due_to_unlink));

    status = getStatus (resItem);

    if (status == RESOLVABLE_STATUS_TO_BE_INSTALLED) {
	ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_REJECT_INSTALL, resItem, RESOLVER_INFO_PRIORITY_VERBOSE);
	addError (misc_info);
	return false;
    }

    if (resItem_status_is_to_be_uninstalled (status)
	&& status != RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_UNLINK) {
	return true;
    }

    if (status == RESOLVABLE_STATUS_UNINSTALLED
	|| status == RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_UNLINK) {
	ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_UNINSTALLABLE, resItem, RESOLVER_INFO_PRIORITY_VERBOSE);
	addInfo (misc_info);
    }

    if (due_to_obsolete)	new_status = RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_OBSOLETE;
    else if (due_to_unlink)	new_status = RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_UNLINK;
    else			new_status = RESOLVABLE_STATUS_TO_BE_UNINSTALLED;

    setStatus (resItem, new_status);

    if (status == RESOLVABLE_STATUS_INSTALLED) {
	/* FIXME: incomplete */
    }

    return true;
}


// change state to UNNEEDED

bool
ResolverContext::unneededResItem (ResItem_constPtr resItem, int other_penalty)
{
    ResItemStatus status;

    _DBG("RC_SPEW") << "ResolverContext[" << this << "]::unneededResItem(" << resItem->asString() << ")" << endl;

    status = getStatus (resItem);

    if (status == RESOLVABLE_STATUS_INSTALLED
	|| status == RESOLVABLE_STATUS_UNINSTALLED
	|| status == RESOLVABLE_STATUS_SATISFIED) {
	setStatus (resItem, RESOLVABLE_STATUS_UNNEEDED);
    }

    return true;
}


// change state to SATISFIED

bool
ResolverContext::satisfyResItem (ResItem_constPtr resItem, int other_penalty)
{
    ResItemStatus status;

    status = getStatus (resItem);

    _DBG("RC_SPEW") << "ResolverContext[" << this << "]::satisfyResItem(" << resItem->asString() << ":" << toString(status) << ")" << endl;

    if (status == RESOLVABLE_STATUS_INSTALLED
	|| status == RESOLVABLE_STATUS_UNINSTALLED
	|| status == RESOLVABLE_STATUS_UNNEEDED) {
	setStatus (resItem, RESOLVABLE_STATUS_SATISFIED);
    }
    else {
	WAR << "Can't satisfy " << resItem->asString() << " is is already " << toString(status) << endl;
    }

    return true;
}


// change state to INCOMPLETE

bool
ResolverContext::incompleteResItem (ResItem_constPtr resItem, int other_penalty)
{
    _DBG("RC_SPEW") << "ResolverContext[" << this << "]::incompleteResItem(" << resItem->asString() << ")" << endl;

    ResItemStatus status;
    status = getStatus (resItem);

    switch (status) {
	case RESOLVABLE_STATUS_INSTALLED: {
	    ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_INCOMPLETES, resItem, RESOLVER_INFO_PRIORITY_VERBOSE);
	    addError (misc_info);
	    return false;
	}
	break;
	case RESOLVABLE_STATUS_UNNEEDED: {
	    ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_INCOMPLETES, resItem, RESOLVER_INFO_PRIORITY_VERBOSE);
	    addError (misc_info);
	    return false;
	}
	break;
	default:
	break;
    }

    setStatus (resItem, RESOLVABLE_STATUS_INCOMPLETE);

    return true;
}

//---------------------------------------------------------------------------

// is it installed (after transaction) ?
// if yes, install/requires requests are considered done
bool
ResolverContext::resItemIsPresent (ResItem_constPtr resItem)
{
    ResItemStatus status;

    status = getStatus (resItem);
//ERR << "ResolverContext::resItemIsPresent(<" << ResolverContext::toString(status) << ">" << resItem->asString() << ")" << endl;
    if (status == RESOLVABLE_STATUS_UNKNOWN)
	return false;

    return (status == RESOLVABLE_STATUS_INSTALLED)
	    || resItem_status_is_to_be_installed (status)
	    || resItem_status_is_satisfied (status)
	    || resItem_status_is_unneeded (status);
}


// is it uninstalled (after transaction) ?
// if yes, uninstall requests are considered done
bool
ResolverContext::resItemIsAbsent (ResItem_constPtr resItem)
{
    ResItemStatus status;

    status = getStatus (resItem);
    if (status == RESOLVABLE_STATUS_UNKNOWN)
	return false;

    // DONT add incomplete here, uninstall requests for incompletes must be handled

    return (status == RESOLVABLE_STATUS_UNINSTALLED)
	   || resItem_status_is_to_be_uninstalled (status);
}


//---------------------------------------------------------------------------
// marked

void
ResolverContext::foreachMarkedResItem (MarkedResItemFn fn, void *data) const
{
    ResolverContext_constPtr context = this;
    while (context) {
	for (StatusTable::const_iterator iter = context->_status.begin(); iter != context->_status.end(); iter++) {
	    fn (iter->first, iter->second, data);
	}
	context = context->_parent;
    }
}


//---------------------------------------------------------------------------
// collect

typedef struct {
    CResItemList *rl;
    int status;				// <0: uninstalls, ==0: all, >0: installs
} MarkedResolvableInfo;


static void
marked_resItem_collector (ResItem_constPtr resItem, ResItemStatus status, void *data)
{
    MarkedResolvableInfo *info = (MarkedResolvableInfo *)data;
    if (info->status == 0
       || (info->status > 0 && resItem_status_is_to_be_installed(status))
       || (info->status < 0 && resItem_status_is_to_be_uninstalled(status))) {
	info->rl->push_back (resItem);
    }
}


CResItemList
ResolverContext::getMarkedResItems (int which) const
{
    CResItemList rl;
    MarkedResolvableInfo info = { &rl, which };

    foreachMarkedResItem (marked_resItem_collector, &info);

    return rl;
}

//---------------------------------------------------------------------------
// install

typedef struct {
    World_Ptr world;
    MarkedResItemFn fn;
    CResItemList *rl;
    int count;
} InstallInfo;

static void
install_pkg_cb (ResItem_constPtr resItem, ResItemStatus status, void *data)
{
    InstallInfo *info = (InstallInfo *)data;
    if (resItem_status_is_to_be_installed (status)
	&& ! resItem->isInstalled ()
	&& info->world->findInstalledResItem (resItem) == NULL) {

	if (info->fn) info->fn (resItem, status, info->rl);
	++info->count;
    }
}


int
ResolverContext::foreachInstall (MarkedResItemFn fn, void *data) const
{
    CResItemList *rl = (CResItemList *)data;
    InstallInfo info = { world(), fn, rl, 0 };

    foreachMarkedResItem (install_pkg_cb, (void *)&info);

    return info.count;
}


static void
context_resItem_collector (ResItem_constPtr resItem, ResItemStatus status, void *data)
{
    CResItemList *rl = (CResItemList *)data;
    if (resItem_status_is_to_be_installed (status)
	|| (resItem_status_is_to_be_uninstalled (status) && resItem->isInstalled ())) {
	rl->push_front (resItem);
    }
}


CResItemList
ResolverContext::getInstalls (void) const
{
    CResItemList rl;

    foreachInstall (context_resItem_collector, (void *)&rl);

    return rl;
}


//---------------------------------------------------------------------------
// satisfy

typedef struct {
    World_Ptr world;
    MarkedResItemFn fn;
    CResItemList *rl;
    int count;
} SatisfyInfo;

static void
satisfy_pkg_cb (ResItem_constPtr resItem, ResItemStatus status, void *data)
{
    SatisfyInfo *info = (SatisfyInfo *)data;
    if (resItem_status_is_satisfied (status)
       && ! resItem->isInstalled ()
       && info->world->findInstalledResItem (resItem) == NULL) {

       if (info->fn) info->fn (resItem, status, info->rl);
       ++info->count;
    }
}


int
ResolverContext::foreachSatisfy (MarkedResItemFn fn, void *data)
{
    CResItemList *rl = (CResItemList *)data;
    SatisfyInfo info = { world(), fn, rl, 0 };

    foreachMarkedResItem (satisfy_pkg_cb, (void *)&info);

    return info.count;
}


static void
context_resItem_collector_satisfy (ResItem_constPtr resItem, ResItemStatus status, void *data)
{
    CResItemList *rl = (CResItemList *)data;
    if (resItem_status_is_satisfied (status)) {
       rl->push_front (resItem);
    }
}


CResItemList
ResolverContext::getSatisfies (void)
{
    CResItemList rl;

    foreachSatisfy (context_resItem_collector_satisfy, (void *)&rl);

    return rl;
}


//---------------------------------------------------------------------------
// incomplete

typedef struct {
    World_Ptr world;
    MarkedResItemFn fn;
    CResItemList *rl;
    int count;
} IncompleteInfo;

static void
incomplete_pkg_cb (ResItem_constPtr resItem, ResItemStatus status, void *data)
{
    IncompleteInfo *info = (IncompleteInfo *)data;

    if (resItem_status_is_incomplete (status)) {
       if (info->fn) info->fn (resItem, status, info->rl);
       ++info->count;
    }
}


int
ResolverContext::foreachIncomplete (MarkedResItemFn fn, void *data)
{
    CResItemList *rl = (CResItemList *)data;
    IncompleteInfo info = { world(), fn, rl, 0 };

    foreachMarkedResItem (incomplete_pkg_cb, (void *)&info);

    return info.count;
}


static void
context_resItem_collector_incomplete (ResItem_constPtr resItem, ResItemStatus status, void *data)
{
    CResItemList *rl = (CResItemList *)data;
    if (resItem_status_is_incomplete (status)) {
       rl->push_front (resItem);
    }
}


CResItemList
ResolverContext::getIncompletes (void)
{
    CResItemList rl;

    foreachIncomplete (context_resItem_collector_incomplete, (void *)&rl);

    return rl;
}


//---------------------------------------------------------------------------
// upgrade

typedef struct {
    World_Ptr world;
    MarkedResItemPairFn fn;
    void *data;
    ResolverContext_Ptr context;
    int count;
} UpgradeInfo;

static void
upgrade_pkg_cb (ResItem_constPtr resItem, ResItemStatus status, void *data)
{
    UpgradeInfo *info = (UpgradeInfo *)data;

    ResItem_constPtr to_be_upgraded;
    ResItemStatus tbu_status;

    if (resItem_status_is_to_be_installed (status)
	&& ! resItem->isInstalled ()) {

	to_be_upgraded = info->world->findInstalledResItem (resItem);
	if (to_be_upgraded) {
	    tbu_status = info->context->getStatus (to_be_upgraded);
	    if (info->fn) {
		info->fn (resItem, status, to_be_upgraded, tbu_status, info->data);
	    }
	    ++info->count;
	}
    }
}


int
ResolverContext::foreachUpgrade (MarkedResItemPairFn fn, void *data)
{
    UpgradeInfo info = { world(), fn, data, this, 0 };

    foreachMarkedResItem (upgrade_pkg_cb, (void *)&info);

    return info.count;
}


static void
pair_resItem_collector (ResItem_constPtr resItem, ResItemStatus status, ResItem_constPtr old, ResItemStatus old_status, void *data)
{
    CResItemList *rl = (CResItemList *)data;
    rl->push_back (resItem);
}


CResItemList
ResolverContext::getUpgrades (void)
{
    CResItemList rl;

    foreachUpgrade (pair_resItem_collector, (void *)&rl);

    return rl;
}


//---------------------------------------------------------------------------
// uninstall

typedef std::map<std::string,ResItem_constPtr> UpgradeTable;

typedef struct {
    MarkedResItemFn fn;
    CResItemList *rl;
    UpgradeTable upgrade_hash;
    int count;
} UninstallInfo;

static void
uninstall_pkg_cb (ResItem_constPtr resItem, ResItemStatus status, void *data)
{
    UninstallInfo *info = (UninstallInfo *)data;

    UpgradeTable::const_iterator pos = info->upgrade_hash.find(resItem->name());

    if (resItem_status_is_to_be_uninstalled (status)
	&& pos == info->upgrade_hash.end()) {
	if (info->fn)
	    info->fn (resItem, status, info->rl);
	++info->count;
    }
}

static void
build_upgrade_hash_cb (ResItem_constPtr resItem_add, ResItemStatus status_add, ResItem_constPtr resItem_del, ResItemStatus status_del, void *data)
{
    UpgradeTable *upgrade_hash = (UpgradeTable *)data;
    (*upgrade_hash)[resItem_del->name()] = resItem_del;
}


int
ResolverContext::foreachUninstall (MarkedResItemFn fn, void *data)
{
    UninstallInfo info;		// inits upgrade_hash

    info.fn = fn;
    info.rl = (CResItemList *)data;
    info.count = 0;

    foreachUpgrade (build_upgrade_hash_cb, (void *)&(info.upgrade_hash));
    foreachMarkedResItem (uninstall_pkg_cb, (void *)&info);

    return info.count;
}


CResItemList
ResolverContext::getUninstalls (void)
{
    CResItemList rl;

    foreachUninstall (context_resItem_collector, (void *)&rl);

    return rl;
}


//---------------------------------------------------------------------------

static void
install_count_cb (ResItem_constPtr resItem, ResItemStatus status, void *data)
{
    int *count = (int *)data;
    if (! resItem->isInstalled ()) {
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
uninstall_count_cb (ResItem_constPtr resItem, ResItemStatus status, void *data)
{
    int *count = (int *)data;
    if (resItem->isInstalled ()) {
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
    return foreachUpgrade ((MarkedResItemPairFn)NULL, (void *)NULL);
}


static void
satisfy_count_cb (ResItem_constPtr resItem, ResItemStatus status, void *data)
{
    int *count = (int *)data;
    if (! resItem->isInstalled ()) {
	++*count;
    }
}

int
ResolverContext::satisfyCount (void)
{
    int count = 0;

    foreachSatisfy (satisfy_count_cb, (void *)&count);

    return count;
}


int
ResolverContext::incompleteCount (void)
{
    return foreachIncomplete ((MarkedResItemFn)NULL, (void *)NULL);
}



//---------------------------------------------------------------------------
// info

void
ResolverContext::addInfo (ResolverInfo_Ptr info)
{
    _DBG("RC_SPEW") << "ResolverContext[" << this << "]::addInfo(" << info->asString() << ")" << endl;
    _log.push_back (info);

    // _propagated_importance = false;

    if (info->error ()) {

	if (! _invalid) {
	    ResolverInfo_Ptr info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_INVALID_SOLUTION, NULL, RESOLVER_INFO_PRIORITY_VERBOSE);
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

//  We call a resItem mentioned by an error info an "error-resItem".
//  We call a resItem mentioned by an important info an "important-resItem".
//
//  The rules:
//  (1) An info item that mentions an error-resItem is important.
//  (2) An info item is about an important-resItem is important.

static void
mark_important_info (InfoList & il)
{
    // set of all resItems mentioned in the InfoList
    CResItemSet error_set;

    bool did_something;
    int pass_num = 1;

    /* First of all, store all error-resItems in a set. */

    for (InfoList::iterator info_iter = il.begin(); info_iter != il.end(); ++info_iter) {
	ResolverInfo_Ptr info = (*info_iter);
	if (info != NULL						// list items might be NULL
	    && info->error ()) {					// only look at error infos

	    ResItem_constPtr resItem = info->affected();		// get resItem from InfoList
	    if (resItem != NULL) {
		error_set.insert (resItem);
	    }

	    // the info might be a container, check it by doing a dynamic cast

	    CResItemList containerItems;
	    ResolverInfoContainer_constPtr c = dynamic_pointer_cast<const ResolverInfoContainer>(*info_iter);
	    if (c != NULL) containerItems = c->resItems();

	    // containerItems is non-empty if info is really a ResolverInfoContainer

	    for (CResItemList::iterator res_iter = containerItems.begin(); res_iter != containerItems.end(); res_iter++) {
		ResItem_constPtr resItem = (*res_iter);
		if (resItem != NULL) {
		    error_set.insert (resItem);
		}
	    }
	}
    }

    // now collect all important ones

    CResItemSet important_set;

    do {
	++pass_num;
	assert (pass_num < 10000);

	did_something = false;

	for (InfoList::iterator info_iter = il.begin(); info_iter != il.end(); ++info_iter) {
	    ResolverInfo_Ptr info = (*info_iter);
	    if (info != NULL						// list items might be set to NULL
		&& info->important ()) {				// only look at important ones
		bool should_be_important = false;

		for (CResItemSet::const_iterator res_iter = error_set.begin(); res_iter != error_set.end() && ! should_be_important; ++res_iter) {
		    ResolverInfoContainer_constPtr c = dynamic_pointer_cast<const ResolverInfoContainer>(*info_iter);
		    if (c != NULL					// check if it really is a container
			&& c->mentions (*res_iter)) {
			should_be_important = true;
		    }
		}

		for (CResItemSet::const_iterator res_iter = important_set.begin(); res_iter != important_set.end() && ! should_be_important; ++res_iter) {
		    if (info->isAbout (*res_iter)) {
			should_be_important = true;
			break;
		    }
		}

		if (should_be_important) {
		    did_something = true;
		    info->flagAsImportant ();
		    CResItemList resItems;
		    ResolverInfoContainer_constPtr c = dynamic_pointer_cast<const ResolverInfoContainer>(*info_iter);		// check if it really is a container
		    if (c != NULL) resItems = c->resItems();
		    for (CResItemList::iterator res_iter = resItems.begin(); res_iter != resItems.end(); res_iter++) {
			important_set.insert (*res_iter);
		    }
		}
	    }
	}

    } while (did_something);

}


void
ResolverContext::foreachInfo (ResItem_Ptr resItem, int priority, ResolverInfoFn fn, void *data)
{
    InfoList info_list;

    ResolverContext_Ptr context = this;

    // Assemble a list of copies of all of the info objects
    while (context != NULL) {
	for (InfoList::iterator iter = context->_log.begin(); iter != context->_log.end(); iter++) {
	    if ((resItem == NULL || (*iter)->affected() == resItem)
		&& (*iter)->priority() >= priority) {
		info_list.push_back ((*iter)->copy());
	    }
	}
	context = context->_parent;
    }
#if 0
    // Merge info objects
    for (InfoList::iterator iter = info_list.begin(); iter != info_list.end(); iter++) {

	ResolverInfo_Ptr info1 = (*iter);
	InfoList::iterator subiter = iter;
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

    for (InfoList::iterator iter = info_list.begin(); iter != info_list.end(); iter++) {
	if (*iter != NULL) {
	    fn (*iter, data);
	}
    }
}



static void
get_info_foreach_cb (ResolverInfo_Ptr info, void *data)
{
    InfoList *il = (InfoList *)data;

    if (info->important ()) {
	il->push_back (info);
    }
}



InfoList
ResolverContext::getInfo (void)
{
    InfoList il;
    foreachInfo (NULL, -1, get_info_foreach_cb, (void *)&il);
    return il;
}


//---------------------------------------------------------------------------
// spew

static void
spew_pkg_cb (ResItem_constPtr resItem, ResItemStatus status, void *unused)
{
    printf ("  %s (%s)\n", resItem->asString().c_str(), ResolverContext::toString(status).c_str());
}


void
spew_pkg2_cb (ResItem_constPtr resItem1, ResItemStatus status1, ResItem_constPtr resItem2, ResItemStatus status2, void *unused)
{
    const char *s1, *s2;

    s1 = resItem1->asString().c_str();
    s2 = resItem2->asString().c_str();

    printf ("  %s (%s) => %s (%s)\n", s2, ResolverContext::toString(status2).c_str(), s1, ResolverContext::toString(status1).c_str());
}


void
ResolverContext::spew (void)
{
    printf ("TO INSTALL:\n");
    foreachInstall (spew_pkg_cb, NULL);
    printf ("\n");

    printf ("TO REMOVE:\n");
    foreachUninstall (spew_pkg_cb, NULL);
    printf ("\n");

    printf ("TO UPGRADE:\n");
    foreachUpgrade (spew_pkg2_cb, NULL);
    printf ("\n");
}


static void
spew_info_cb (ResolverInfo_Ptr info, void *unused)
{
    const char *msg = info->asString().c_str();
    if (info->error ()) printf ("[ERROR] ");
    else if (info->important()) printf ("[>>>>>] ");
    printf ("%s\n", msg);
}


void
ResolverContext::spewInfo (void)
{
    _DBG("RC_SPEW") << "ResolverContext[" << this << "]::spewInfo" << endl;
    foreachInfo (NULL, -1, spew_info_cb, NULL);
}

//---------------------------------------------------------------------------
// requirements

typedef struct {
    ResolverContext_Ptr context;
    const Capability *dep;
    bool flag;
} RequirementMetInfo;


static bool
requirement_met_cb (ResItem_constPtr resItem, const Capability & cap, void *data)
{
    RequirementMetInfo *info = (RequirementMetInfo *)data;

    // info->dep is set for resItem set children. If it is set, query the
    //   exact version only.
    if ((info->dep == NULL
	 || *(info->dep) == cap)
	&& info->context->resItemIsPresent (resItem))
    {
	info->flag = true;
    }

//ERR << "requirement_met_cb(" <<  resItem->asString() << ", " << cap.asString() << ") [info->dep " <<
//    (info->dep != NULL ? info->dep->asString().c_str() : "(none)") << "] -> " <<  (info->flag ? "true" : "false") << endl;
    return ! info->flag;
}


bool
ResolverContext::requirementIsMet (const Capability & dependency, bool is_child)
{
    RequirementMetInfo info;

    info.context = this;
    info.dep = is_child ? &dependency : NULL;
    info.flag = false;

    world()->foreachProvidingResItem (dependency, requirement_met_cb, (void *)&info);

    return info.flag;
}


//---------------------------------------------------------------------------

static bool
requirement_possible_cb (ResItem_constPtr resItem, const Capability & cap, void *data)
{
    RequirementMetInfo *info = (RequirementMetInfo *)data;

    ResItemStatus status = info->context->getStatus (resItem);

    if (! resItem_status_is_to_be_uninstalled (status)
	|| status == RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_UNLINK) {
	info->flag = true;
    }

    return ! info->flag;
}


bool
ResolverContext::requirementIsPossible (const Capability & dep)
{
    RequirementMetInfo info;

    info.context = this;
    info.flag = false;

    world()->foreachProvidingResItem (dep, requirement_possible_cb, (void *)&info);

    return info.flag;
}


bool
ResolverContext::resItemIsPossible (ResItem_constPtr resItem)
{
    CapSet requires = resItem->requires();
    for (CapSet::iterator iter = requires.begin(); iter !=  requires.end(); iter++) {
	    if (! requirementIsPossible (*iter)) {
		return false;
	    }
	}

    return true;
}

//---------------------------------------------------------------------------

typedef struct {
    ResItem_constPtr res;
    bool flag;
} DupNameCheckInfo;

static void
dup_name_check_cb (ResItem_constPtr resItem, ResItemStatus status, void *data)
{
    DupNameCheckInfo *info = (DupNameCheckInfo *)data;
    if (! info->flag
	&& resItem_status_is_to_be_installed (status)
	&& info->res->name() == resItem->name()
	&& !resItem->equals(info->res)) {
	info->flag = true;
    }
}

bool
ResolverContext::isParallelInstall (ResItem_constPtr resItem)
{
    DupNameCheckInfo info;

    info.res = resItem;
    info.flag = false;
    foreachMarkedResItem (dup_name_check_cb, (void *)&info);

    return info.flag;
}


int
ResolverContext::getChannelPriority (Channel_constPtr channel) const
{
    bool is_subscribed;
    int priority;

    is_subscribed = channel->isSubscribed ();
    priority = channel->getPriority (is_subscribed);

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
#endif
///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////

