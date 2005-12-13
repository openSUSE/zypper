/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <y2util/stringutil.h>

#include <zypp/solver/detail/ResolverContext.h>
#include <zypp/solver/detail/ResolverInfoMisc.h>
#include <zypp/solver/detail/World.h>

///////////////////////////////////////////////////////////////////
namespace ZYPP {
//////////////////////////////////////////////////////////////////

using namespace std;

IMPL_BASE_POINTER(ResolverContext);

//---------------------------------------------------------------------------

string
ResolverContext::toString (const ResolvableStatus & status)
{
    string ret;
    switch (status) {
    case RESOLVABLE_STATUS_UNKNOWN:				ret = "unknown"; break;
    case RESOLVABLE_STATUS_INSTALLED:				ret = "installed"; break;
    case RESOLVABLE_STATUS_UNINSTALLED:				ret = "uninstalled"; break;
    case RESOLVABLE_STATUS_TO_BE_INSTALLED:			ret = "to be installed"; break;
    case RESOLVABLE_STATUS_TO_BE_INSTALLED_SOFT:		ret = "to be installed (soft)"; break;
    case RESOLVABLE_STATUS_TO_BE_UNINSTALLED:			ret = "to be uninstalled"; break;
    case RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_OBSOLETE:	ret = "to be uninstalled due to obsolete"; break;
    case RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_UNLINK:	ret = "to be uninstalled due to unlink"; break;
    default:							ret = "Huh ?"; break;
    }

    return ret;
}

//---------------------------------------------------------------------------

string
ResolverContext::asString ( void ) const
{
    return toString (*this);
}


string
ResolverContext::toString ( const ResolverContext & context )
{
    string ret;
    if (context._parent != NULL) {
	ret += "Parent: [";
	ret += stringutil::form("<@%p> ", (const void *)(context._parent));
	ret += context._parent->asString();
	ret += "],\n\t";
    }
    ret += stringutil::form ("Download Size: %lld",  context._download_size);
    ret += stringutil::form (", Install Size: %lld", context._install_size);
    ret += stringutil::form (", Total Priority: %d", context._total_priority);
    ret += stringutil::form (", Min Priority: %d", context._min_priority);
    ret += stringutil::form (", Max Priority: %d", context._max_priority);
    ret += stringutil::form (", Other Penalties: %d", context._other_penalties);
    if (context._current_channel != 0) {
	ret += ", Current Channel";
	ret += context._current_channel->asString();
    }
    if (context._verifying) ret += ", Verifying";
    if (context._invalid) ret += ", Invalid";

    return ret;
}


ostream &
ResolverContext::dumpOn( ostream & str ) const
{
    str << asString();
    return str;
}


ostream&
operator<<( ostream& os, const ResolverContext & ResolverContext)
{
    return os << ResolverContext.asString();
}

//---------------------------------------------------------------------------

ResolverContext::ResolverContext (ResolverContextPtr parent)
    : _parent (parent)
    , _refs (0)
    , _world (NULL)
    , _last_checked_resolvable (NULL)
    , _last_checked_status (RESOLVABLE_STATUS_UNKNOWN)
    , _download_size (0)
    , _install_size (0)
    , _total_priority (0)
    , _min_priority (0)
    , _max_priority (0)
    , _other_penalties (0)
    , _current_channel (NULL)
    , _verifying (false)
    , _invalid (false)
{
    if (parent != NULL) {
	_world           = parent->_world;
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

WorldPtr
ResolverContext::world (void) const
{
    if (_world == NULL) {
	return World::globalWorld();
    }
    return _world;
}


void
ResolverContext::setStatus (constResolvablePtr resolvable, ResolvableStatus status)
{
    if (_invalid) return;

    ResolvableStatus old_status = getStatus (resolvable);

    if (status != old_status) {
	_status[resolvable] = status;
    }

    // Update our cache if we changed the status of the last checked resolvable.

    if (_last_checked_resolvable == resolvable)
	_last_checked_status = status;
}


ResolvableStatus
ResolverContext::getStatus (constResolvablePtr resolvable)
{
    ResolvableStatus status = RESOLVABLE_STATUS_UNKNOWN;

    // We often end up getting the status of the same resolvable several times
    // in a row.  By caching the status of the last checked resolvable, we can
    // in practice eliminate the need for any hash table lookups in about
    // 50% of our calls to get_status.

    if (resolvable == _last_checked_resolvable)
    {
	return _last_checked_status;
    }

    ResolverContextPtr context = this;

    while (status == RESOLVABLE_STATUS_UNKNOWN
	   && context != NULL) {
	StatusTable::const_iterator pos = context->_status.find (resolvable);
	if (pos != context->_status.end()) {
	    status = (*pos).second;
	}
	context = context->_parent;
    }

    if (status == RESOLVABLE_STATUS_UNKNOWN) {
	status = resolvable->isInstalled() ? RESOLVABLE_STATUS_INSTALLED : RESOLVABLE_STATUS_UNINSTALLED;
    }

    _last_checked_resolvable = resolvable;
    _last_checked_status = status;

    return status;
}


bool
ResolverContext::installResolvable (constResolvablePtr resolvable, bool is_soft, int other_penalty)
{
    ResolvableStatus status, new_status;
    int priority;
    std::string msg;

    status = getStatus (resolvable);
    if (getenv ("RC_SPEW")) fprintf (stderr, "ResolverContext[%p]::installResolvable(<%s>%s)\n", this, ResolverContext::toString(status).c_str(), resolvable->asString().c_str());

    if (resolvable_status_is_to_be_uninstalled (status)
	&& status != RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_UNLINK) {
	msg = string ("Can't install ") + ((constSpecPtr)resolvable)->asString() + " since it is already marked as needing to be uninstalled";

	addErrorString (resolvable, msg);
	return false;
    }

    if (resolvable_status_is_to_be_installed (status)) {
	return true;
    }

    if (isParallelInstall (resolvable)) {
	msg = string ("Can't install ") + ((constSpecPtr)resolvable)->asString() + ", since a resolvable of the same name is already marked as needing to be installed";
	addErrorString (resolvable, msg);
	return false;
    }

    if (is_soft)
	new_status = RESOLVABLE_STATUS_TO_BE_INSTALLED_SOFT;
    else if (status == RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_UNLINK)
	new_status = RESOLVABLE_STATUS_INSTALLED;
    else
	new_status = RESOLVABLE_STATUS_TO_BE_INSTALLED;

    setStatus (resolvable, new_status);

    if (status == RESOLVABLE_STATUS_UNINSTALLED) {
	/* FIXME: Incomplete */
	_download_size += resolvable->fileSize();
	_install_size += resolvable->installedSize();

	if (resolvable->local())
	    priority = 0;
	else {
	    priority = getChannelPriority (resolvable->channel ());
	}

	if (priority < _min_priority) _min_priority = priority;
	if (priority > _max_priority) _max_priority = priority;

	_other_penalties += other_penalty;

    }
	
    return true;
}


bool
ResolverContext::upgradeResolvable (constResolvablePtr resolvable, constResolvablePtr old_resolvable, bool is_soft, int other_penalty)
{
    ResolvableStatus status;
    int priority;

    if (getenv ("RC_SPEW")) fprintf (stderr, "ResolverContext[%p]::upgradeResolvable(%s upgrades %s)\n", this, resolvable->asString().c_str(), old_resolvable->asString().c_str());

    status = getStatus (resolvable);

    if (resolvable_status_is_to_be_uninstalled (status))
	return false;

    if (resolvable_status_is_to_be_installed (status))
	return true;

    setStatus (resolvable, is_soft ? RESOLVABLE_STATUS_TO_BE_INSTALLED_SOFT : RESOLVABLE_STATUS_TO_BE_INSTALLED);

    if (status == RESOLVABLE_STATUS_UNINSTALLED) {
      
	_download_size += resolvable->fileSize();

	// FIXME: Incomplete
	// We should change installed_size to reflect the difference in
	//   installed size between the old and new versions.

	if (resolvable->local())
	    priority = 0;
	else {
	    priority = getChannelPriority (resolvable->channel());
	}

	if (priority < _min_priority) _min_priority = priority;
	if (priority > _max_priority) _max_priority = priority;

	_other_penalties += other_penalty;
    }

    return true;
}


bool
ResolverContext::uninstallResolvable (constResolvablePtr resolvable, bool part_of_upgrade, bool due_to_obsolete, bool due_to_unlink)
{
    ResolvableStatus status, new_status;
    std::string msg;

    if (getenv ("RC_SPEW")) fprintf (stderr, "ResolverContext[%p]::uninstallResolvable(%s %s %s %s)\n", this, resolvable->asString().c_str(), part_of_upgrade ? "part_of_upgrade" : "", due_to_obsolete ? "due_to_obsolete": "", due_to_unlink ? "due_to_unlink" : "");

    assert (! (due_to_obsolete && due_to_unlink));

    status = getStatus (resolvable);

    if (status == RESOLVABLE_STATUS_TO_BE_INSTALLED) {
	msg = ((constSpecPtr)resolvable)->asString() + " is scheduled to be installed, but this is not possible because of dependency problems.";
	addErrorString (resolvable, msg);
	return false;
    }

    if (resolvable_status_is_to_be_uninstalled (status)
	&& status != RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_UNLINK) {
	return true;
    }
    
    if (status == RESOLVABLE_STATUS_UNINSTALLED
	|| status == RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_UNLINK) {
	msg = string ("Marking resolvable ") + ((constSpecPtr)resolvable)->asString() + " as uninstallable";
	addInfoString (resolvable, RESOLVER_INFO_PRIORITY_VERBOSE, msg);
    }


    if (due_to_obsolete)	new_status = RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_OBSOLETE;
    else if (due_to_unlink)	new_status = RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_UNLINK;
    else			new_status = RESOLVABLE_STATUS_TO_BE_UNINSTALLED;

    setStatus (resolvable, new_status);
    
    if (status == RESOLVABLE_STATUS_INSTALLED) {
	/* FIXME: incomplete */
    }

    return true;
}


bool
ResolverContext::resolvableIsPresent (constResolvablePtr resolvable)
{
    ResolvableStatus status;

    status = getStatus (resolvable);
//fprintf (stderr, "ResolverContext::resolvableIsPresent(<%s>%s)\n", ResolverContext::toString(status).c_str(), resolvable->asString().c_str());
    if (status == RESOLVABLE_STATUS_UNKNOWN)
	return false;

    return (status == RESOLVABLE_STATUS_INSTALLED) || resolvable_status_is_to_be_installed (status);
}


bool
ResolverContext::resolvableIsAbsent (constResolvablePtr resolvable)
{
    ResolvableStatus status;

    status = getStatus (resolvable);
    if (status == RESOLVABLE_STATUS_UNKNOWN)
	return false;

    return status == RESOLVABLE_STATUS_UNINSTALLED || resolvable_status_is_to_be_uninstalled (status);
}


//---------------------------------------------------------------------------
// marked

void
ResolverContext::foreachMarkedResolvable (MarkedResolvableFn fn, void *data) const
{
    constResolverContextPtr context = this;
    while (context) {
	for (StatusTable::const_iterator iter = context->_status.begin(); iter != context->_status.end(); iter++) {
	    fn (iter->first, iter->second, data);
	}
	context = context->_parent;
    }
}


//---------------------------------------------------------------------------
// collect

static void
marked_resolvable_collector (constResolvablePtr resolvable, ResolvableStatus status, void *data)
{
    CResolvableList *rl = (CResolvableList *)data;
    rl->push_back (resolvable);
}


CResolvableList
ResolverContext::getMarkedResolvables (void) const
{
    CResolvableList rl;

    foreachMarkedResolvable (marked_resolvable_collector, &rl);

    return rl;
}

//---------------------------------------------------------------------------
// install

typedef struct {
    WorldPtr world;
    MarkedResolvableFn fn;
    CResolvableList *rl;
    int count;
} InstallInfo;

static void
install_pkg_cb (constResolvablePtr resolvable, ResolvableStatus status, void *data)
{
    InstallInfo *info = (InstallInfo *)data;
    if (resolvable_status_is_to_be_installed (status)
	&& ! resolvable->isInstalled ()
	&& info->world->findInstalledResolvable (resolvable) == NULL) {

	if (info->fn) info->fn (resolvable, status, info->rl);
	++info->count;
    }
}


int
ResolverContext::foreachInstall (MarkedResolvableFn fn, void *data) const
{
    CResolvableList *rl = (CResolvableList *)data;
    InstallInfo info = { world(), fn, rl, 0 };

    foreachMarkedResolvable (install_pkg_cb, (void *)&info);

    return info.count;
}


static void
context_resolvable_collector (constResolvablePtr resolvable, ResolvableStatus status, void *data)
{
    CResolvableList *rl = (CResolvableList *)data;
    if (resolvable_status_is_to_be_installed (status)
	|| (resolvable_status_is_to_be_uninstalled (status) && resolvable->isInstalled ())) {
	rl->push_front (resolvable);
    } 
}


CResolvableList
ResolverContext::getInstalls (void) const
{
    CResolvableList rl;

    foreachInstall (context_resolvable_collector, (void *)&rl);

    return rl;
}


//---------------------------------------------------------------------------
// upgrade

typedef struct {
    WorldPtr world;
    MarkedResolvablePairFn fn;
    void *data;
    ResolverContextPtr context;
    int count;
} UpgradeInfo;

static void
upgrade_pkg_cb (constResolvablePtr resolvable, ResolvableStatus status, void *data)
{
    UpgradeInfo *info = (UpgradeInfo *)data;

    constResolvablePtr to_be_upgraded;
    ResolvableStatus tbu_status;

    if (resolvable_status_is_to_be_installed (status)
	&& ! resolvable->isInstalled ()) {
	
	to_be_upgraded = info->world->findInstalledResolvable (resolvable);
	if (to_be_upgraded) {
	    tbu_status = info->context->getStatus (to_be_upgraded);
	    if (info->fn) {
		info->fn (resolvable, status, to_be_upgraded, tbu_status, info->data);
	    }
	    ++info->count;
	}
    }
}


int
ResolverContext::foreachUpgrade (MarkedResolvablePairFn fn, void *data)
{
    UpgradeInfo info = { world(), fn, data, this, 0 };

    foreachMarkedResolvable (upgrade_pkg_cb, (void *)&info);

    return info.count;
}


static void
pair_resolvable_collector (constResolvablePtr resolvable, ResolvableStatus status, constResolvablePtr old, ResolvableStatus old_status, void *data)
{
    CResolvableList *rl = (CResolvableList *)data;
    rl->push_back (resolvable);
}


CResolvableList
ResolverContext::getUpgrades (void)
{
    CResolvableList rl;

    foreachUpgrade (pair_resolvable_collector, (void *)&rl);

    return rl;
}


//---------------------------------------------------------------------------
// uninstall

typedef std::map<std::string,constResolvablePtr> UpgradeTable;

typedef struct {
    MarkedResolvableFn fn;
    CResolvableList *rl;
    UpgradeTable upgrade_hash;
    int count;
} UninstallInfo;

static void
uninstall_pkg_cb (constResolvablePtr resolvable, ResolvableStatus status, void *data)
{
    UninstallInfo *info = (UninstallInfo *)data;

    UpgradeTable::const_iterator pos = info->upgrade_hash.find(resolvable->name());

    if (resolvable_status_is_to_be_uninstalled (status)
	&& pos == info->upgrade_hash.end()) {
	if (info->fn)
	    info->fn (resolvable, status, info->rl);
	++info->count;
    }
}

static void
build_upgrade_hash_cb (constResolvablePtr resolvable_add, ResolvableStatus status_add, constResolvablePtr resolvable_del, ResolvableStatus status_del, void *data)
{
    UpgradeTable *upgrade_hash = (UpgradeTable *)data;
    (*upgrade_hash)[resolvable_del->name()] = resolvable_del;
}


int
ResolverContext::foreachUninstall (MarkedResolvableFn fn, void *data)
{
    UninstallInfo info;		// inits upgrade_hash

    info.fn = fn;
    info.rl = (CResolvableList *)data;
    info.count = 0;

    foreachUpgrade (build_upgrade_hash_cb, (void *)&(info.upgrade_hash));
    foreachMarkedResolvable (uninstall_pkg_cb, (void *)&info);

    return info.count;
}


CResolvableList
ResolverContext::getUninstalls (void)
{
    CResolvableList rl;

    foreachUninstall (context_resolvable_collector, (void *)&rl);

    return rl;
}


//---------------------------------------------------------------------------

static void
install_count_cb (constResolvablePtr resolvable, ResolvableStatus status, void *data)
{
    int *count = (int *)data;
    if (! resolvable->isInstalled ()) {
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
uninstall_count_cb (constResolvablePtr resolvable, ResolvableStatus status, void *data)
{
    int *count = (int *)data;
    if (resolvable->isInstalled ()) {
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
    return foreachUpgrade ((MarkedResolvablePairFn)NULL, (void *)NULL);
}


//---------------------------------------------------------------------------
// info

void
ResolverContext::addInfo (ResolverInfoPtr info)
{
    if (getenv ("RC_SPEW")) fprintf (stderr, "ResolverContext[%p]::addInfo(%s)\n", this, info->asString().c_str());
    _log.push_back (info);

    // _propagated_importance = false;

    if (info->error ()) {

	if (! _invalid) {
	    ResolverInfoPtr info = new ResolverInfoMisc (NULL, RESOLVER_INFO_PRIORITY_VERBOSE, "Marking this resolution attempt as invalid.");
	    info->flagAsError ();
	    _log.push_back (info);
	}

	_invalid = true;
    }
}


void
ResolverContext::addInfoString (constResolvablePtr resolvable, int priority, string msg)
{
//    if (getenv ("RC_SPEW")) fprintf (stderr, "ResolverContext::addInfoString(%s) %s\n", resolvable ? resolvable->asString().c_str() : "", msg.c_str());
    ResolverInfoPtr info = new ResolverInfoMisc (resolvable, priority, msg);
    addInfo (info);
}


void
ResolverContext::addErrorString (constResolvablePtr resolvable, string msg)
{
    ResolverInfoPtr info = new ResolverInfoMisc (resolvable, RESOLVER_INFO_PRIORITY_VERBOSE, msg);
    info->flagAsError ();
    addInfo (info);
}


//---------------------------------------------------------------------------
// foreach info

//  We call a resolvable mentioned by an error info an "error-resolvable".
//  We call a resolvable mentioned by an important info an "important-resolvable".
//
//  The rules:
//  (1) An info item that mentions an error-resolvable is important.
//  (2) An info item is about an important-resolvable is important.

static void
mark_important_info (InfoList & il)
{
    CResolvableList error_list;		// FIXME, a map is faster

    bool did_something;
    int pass_num = 1;

    /* First of all, store all error-resolvables in a list. */

    for (InfoList::iterator iter = il.begin(); iter != il.end(); iter++) {
	if ((*iter) != NULL					// list items might be set to NULL
	    && (*iter)->error ()) {
	    constResolvablePtr resolvable = (*iter)->resolvable();
	    if (resolvable != NULL) {
		CResolvableList::iterator pos;
		for (pos = error_list.begin(); pos != error_list.end(); pos++) {
		    if (*pos == resolvable)
			break;
		}
		if (pos == error_list.end()) {
		    error_list.push_front (resolvable);
		}
	    }

	    CResolvableList resolvables;

	    constResolverInfoContainerPtr c = *iter;			// check if it really is a container
	    if (c != NULL) resolvables = c->resolvables();

	    for (CResolvableList::iterator res_iter = resolvables.begin(); res_iter != resolvables.end(); res_iter++) {
		CResolvableList::iterator pos;
		for (pos = error_list.begin(); pos != error_list.end(); pos++) {
		    if (*pos == *iter)
			break;
		}
		if (pos == error_list.end()) {
		    error_list.push_front (*res_iter);
		}
	    }
	}
    }

    CResolvableList important_list;	// FIXME, hash is faster

    do {
	++pass_num;
	assert (pass_num < 10000);

	did_something = false;

	for (InfoList::iterator iter = il.begin(); iter != il.end(); iter++) {
	    if ((*iter) != NULL					// list items might be set to NULL
		&& (*iter)->important ()) {
		bool should_be_important = false;

		for (CResolvableList::const_iterator res_iter = error_list.begin(); res_iter != error_list.end() && ! should_be_important; res_iter++) {
		    constResolverInfoContainerPtr c = *iter;
		    if (c != NULL					// check if it really is a container
			&& c->mentions (*res_iter)) {
			should_be_important = true;
		    }
		}

		for (CResolvableList::const_iterator res_iter = important_list.begin(); res_iter != important_list.end() && ! should_be_important; res_iter++) {
		    if ((*iter)->isAbout (*res_iter)) {
			should_be_important = true;
			break;
		    }
		}

		if (should_be_important) {
		    did_something = true;
		    (*iter)->flagAsImportant ();
		    CResolvableList resolvables;
		    constResolverInfoContainerPtr c = *iter;		// check if it really is a container
		    if (c != NULL) resolvables = c->resolvables();
		    for (CResolvableList::iterator res_iter = resolvables.begin(); res_iter != resolvables.end(); res_iter++) {
			CResolvableList::iterator pos;
			for (pos = important_list.begin(); pos != important_list.end(); pos++) {
			    if (*pos == *res_iter)
				break;
			}
			if (pos == important_list.end()) {
			    important_list.push_front (*res_iter);
			}
		    }
		}
	    }
	}

    } while (did_something);

}


void
ResolverContext::foreachInfo (ResolvablePtr resolvable, int priority, ResolverInfoFn fn, void *data)
{
    InfoList info_list;

    ResolverContextPtr context = this;


    // Assemble a list of copies of all of the info objects
    while (context != NULL) {
	for (InfoList::iterator iter = context->_log.begin(); iter != context->_log.end(); iter++) {
	    if ((resolvable == NULL || (*iter)->resolvable() == resolvable)
		&& (*iter)->priority() >= priority) {
		info_list.push_back ((*iter)->copy());
	    }
	}
	context = context->_parent;
    }
#if 0
    // Merge info objects
    for (InfoList::iterator iter = info_list.begin(); iter != info_list.end(); iter++) {

	ResolverInfoPtr info1 = (*iter);
	InfoList::iterator subiter = iter;
	if (info1 != NULL) {
	    for (subiter++; subiter != info_list.end(); subiter++) {
		ResolverInfoPtr info2 = *subiter;
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
get_info_foreach_cb (ResolverInfoPtr info, void *data)
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
spew_pkg_cb (constResolvablePtr resolvable, ResolvableStatus status, void *unused)
{
    printf ("  %s (%s)\n", resolvable->asString().c_str(), ResolverContext::toString(status).c_str());
}


void
spew_pkg2_cb (constResolvablePtr resolvable1, ResolvableStatus status1, constResolvablePtr resolvable2, ResolvableStatus status2, void *unused)
{
    const char *s1, *s2;

    s1 = resolvable1->asString().c_str();
    s2 = resolvable2->asString().c_str();

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
spew_info_cb (ResolverInfoPtr info, void *unused)
{
    const char *msg = info->asString().c_str();
    if (info->error ()) printf ("[ERROR] ");
    else if (info->important()) printf ("[>>>>>] ");
    printf ("%s\n", msg);
}


void
ResolverContext::spewInfo (void)
{
    if (getenv ("RC_SPEW")) fprintf (stderr, "ResolverContext[%p]::spewInfo()\n", this);
    foreachInfo (NULL, -1, spew_info_cb, NULL);
}

//---------------------------------------------------------------------------
// requirements

typedef struct {
    ResolverContextPtr context;
    constSpecPtr dep;
    bool flag;
} RequirementMetInfo;


static bool
requirement_met_cb (constResolvablePtr resolvable, constSpecPtr spec, void *data)
{
    RequirementMetInfo *info = (RequirementMetInfo *)data;

    // info->dep is set for resolvable set children. If it is set, query the
    //   exact version only.
    if ((info->dep == NULL || info->dep->equals(spec))
	&& info->context->resolvableIsPresent (resolvable))
    {
	info->flag = true;
    }

//fprintf (stderr, "requirement_met_cb(%s, %s) [info->dep %s] -> %s\n", resolvable->asString().c_str(), spec->asString().c_str(), info->dep != NULL ? info->dep->asString().c_str() : "(none)", info->flag ? "true" : "false");
    return ! info->flag;
}


bool
ResolverContext::requirementIsMet (constDependencyPtr dependency, bool is_child)
{
    RequirementMetInfo info;

    info.context = this;
    info.dep = is_child ? dependency : NULL;
    info.flag = false;

    world()->foreachProvidingResolvable (dependency, requirement_met_cb, (void *)&info);

    return info.flag;
}


//---------------------------------------------------------------------------

static bool
requirement_possible_cb (constResolvablePtr resolvable, constSpecPtr spec, void *data)
{
    RequirementMetInfo *info = (RequirementMetInfo *)data;

    ResolvableStatus status = info->context->getStatus (resolvable);

    if (! resolvable_status_is_to_be_uninstalled (status)
	|| status == RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_UNLINK) {
	info->flag = true;
    }

    return ! info->flag;
}


bool
ResolverContext::requirementIsPossible (constDependencyPtr dep)
{
    RequirementMetInfo info;

    info.context = this;
    info.flag = false;

    world()->foreachProvidingResolvable (dep, requirement_possible_cb, (void *)&info);
    
    return info.flag;
}


bool
ResolverContext::resolvableIsPossible (constResolvablePtr resolvable)
{
    CDependencyList requires = resolvable->requires();
    for (CDependencyList::iterator iter = requires.begin(); iter !=  requires.end(); iter++) {
	    if (! requirementIsPossible (*iter)) {
		return false;
	    }
	}

    return true;
}

//---------------------------------------------------------------------------

typedef struct {
    constSpecPtr spec;
    bool flag;
} DupNameCheckInfo;

static void
dup_name_check_cb (constResolvablePtr resolvable, ResolvableStatus status, void *data)
{
    DupNameCheckInfo *info = (DupNameCheckInfo *)data;
    if (! info->flag
	&& resolvable_status_is_to_be_installed (status)
	&& info->spec->name() == resolvable->name()
	&& !info->spec->equals(resolvable)) {
	info->flag = true;
    }
}

bool
ResolverContext::isParallelInstall (constResolvablePtr resolvable)
{
    DupNameCheckInfo info;

    info.spec = resolvable;
    info.flag = false;
    foreachMarkedResolvable (dup_name_check_cb, (void *)&info);

    return info.flag;
}


int
ResolverContext::getChannelPriority (constChannelPtr channel) const
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
churn_factor (ResolverContextPtr a)
{
    return a->upgradeCount() + (2.0 * a->installCount ()) + (4.0 * a->uninstallCount ());
}

int
ResolverContext::partialCompare (ResolverContextPtr context)
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
ResolverContext::compare (ResolverContextPtr context)
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
}; // namespace ZYPP
///////////////////////////////////////////////////////////////////

