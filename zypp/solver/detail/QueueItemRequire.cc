/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <y2util/stringutil.h>

#include <zypp/solver/detail/QueueItemRequire.h>
#include <zypp/solver/detail/QueueItemBranch.h>
#include <zypp/solver/detail/QueueItemUninstall.h>
#include <zypp/solver/detail/QueueItemInstall.h>
#include <zypp/solver/detail/QueueItem.h>
#include <zypp/solver/detail/ResolverContext.h>
#include <zypp/solver/detail/ResolverInfoDependsOn.h>
#include <zypp/solver/detail/ResolverInfoMisc.h>
#include <zypp/solver/detail/ResolverInfoNeededBy.h>
#include <zypp/solver/detail/Version.h>
#include <zypp/solver/detail/World.h>

///////////////////////////////////////////////////////////////////
namespace ZYPP {
//////////////////////////////////////////////////////////////////

using namespace std;

IMPL_DERIVED_POINTER(QueueItemRequire,QueueItem);

//---------------------------------------------------------------------------

string
QueueItemRequire::asString ( void ) const
{
    return toString (*this);
}


string
QueueItemRequire::toString ( const QueueItemRequire & item)
{
    string ret = "[Require: ";
    ret += item._dep->asString();
    if (item._requiring_resolvable != NULL) {
	ret += ", Required by ";
	ret += item._requiring_resolvable->asString();
    }
    if (item._upgraded_resolvable != NULL) {
	ret += ", Upgrades ";
	ret += item._upgraded_resolvable->asString();
    }
    if (item._lost_resolvable != NULL) {
	ret += ", Lost ";
	ret += item._lost_resolvable->asString();
    }
    if (item._remove_only) ret += ", Remove Only";
    if (item._is_child) ret += ", Child";
    ret += "]";

    return ret;
}


ostream &
QueueItemRequire::dumpOn( ostream & str ) const
{
    str << asString();
    return str;
}


ostream&
operator<<( ostream& os, const QueueItemRequire & item)
{
    return os << item.asString();
}

//---------------------------------------------------------------------------

QueueItemRequire::QueueItemRequire (WorldPtr world, constDependencyPtr dep)
    : QueueItem (QUEUE_ITEM_TYPE_REQUIRE, world)
    , _dep (dep)
    , _requiring_resolvable (NULL)
    , _upgraded_resolvable (NULL)
    , _lost_resolvable (NULL)
    , _remove_only (false)
    , _is_child (false)
{
}


QueueItemRequire::~QueueItemRequire()
{
}

//---------------------------------------------------------------------------

void
QueueItemRequire::addResolvable (constResolvablePtr resolvable)
{
    assert (_requiring_resolvable == NULL);
    _requiring_resolvable = resolvable;
}


//---------------------------------------------------------------------------

typedef std::map <constSpecPtr, bool> UniqTable;

typedef struct {
    constResolvablePtr resolvable;
    constSpecPtr dep;
    ResolverContextPtr context;
    WorldPtr world;
    CResolvableList providers;
    UniqTable *uniq;
} RequireProcessInfo;


static bool
require_process_cb (constResolvablePtr resolvable, constSpecPtr spec, void *data)
{
    RequireProcessInfo *info = (RequireProcessInfo *)data;
    ResolvableStatus status;

    status = info->context->getStatus (resolvable);
//fprintf (stderr, "require_process_cb(res: %s, spec %s, status %s)\n", resolvable->asString().c_str(), spec->asString().c_str(), ResolverContext::toString(status).c_str());
//fprintf (stderr, "require_process_cb(info->dep: %s)\n", info->dep ? info->dep->asString().c_str() : "(null)");
//fprintf (stderr, "require_process_cb(resolvableIsPossible -> %d)\n", info->context->resolvableIsPossible (resolvable));
    /* info->dep is set for resolvable set childern only. If it is set
       allow only exactly required version */
    if (info->dep != NULL
	&& !info->dep->equals(spec)) {
	return true;
    }

    if ((! resolvable_status_is_to_be_uninstalled (status))
	&& ! info->context->isParallelInstall (resolvable)
	&& info->uniq->find((constSpecPtr)resolvable) == info->uniq->end()
	&& info->context->resolvableIsPossible (resolvable)
	&& ! info->world->resolvableIsLocked (resolvable)) {

	info->providers.push_front (resolvable);
	(*(info->uniq))[resolvable] = true;
    }

    return true;
}


static bool
no_installable_providers_info_cb (constResolvablePtr resolvable, constSpecPtr spec, void *data)
{
    RequireProcessInfo *info = (RequireProcessInfo *)data;
    ResolvableStatus status;
    string msg_str;

    status = info->context->getStatus (resolvable);

    if (resolvable_status_is_to_be_uninstalled (status)) {
	msg_str = resolvable->name() + " provides " + spec->asString() + ", but is scheduled to be uninstalled.";
    } else if (info->context->isParallelInstall (resolvable)) {
	msg_str = resolvable->name() + " provides " + spec->asString() + ", but another version of that resolvable is already installed.";
    } else if (! info->context->resolvableIsPossible (resolvable)) {
	msg_str = resolvable->name() + " provides " + spec->asString() + ", but it is uninstallable.  Try installing it on its own for more details.";
    } else if (info->world->resolvableIsLocked (resolvable)) {
	msg_str = resolvable->name() + " provides " + spec->asString() + ", but it is locked.";
    }

    if (!msg_str.empty()) {
	info->context->addInfoString (info->resolvable, RESOLVER_INFO_PRIORITY_VERBOSE, msg_str);
    }
    
    return true;
}


static bool
look_for_upgrades_cb (constResolvablePtr resolvable, void *data)
{
    CResolvableList *rl = (CResolvableList *)data;
    rl->push_front (resolvable);
    return true;
}


static bool
codependent_resolvables (constResolvablePtr r1, constResolvablePtr r2)
{
    string name1 = r1->name();
    string name2 = r2->name();
    int len1 = name1.size();
    int len2 = name2.size();

    if (len2 < len1) {
	string swap = name1;
	int swap_len = len1;
	name1 = name2;
	name2 = swap;
	len1 = len2;
	len2 = swap_len;
    }

    // foo and foo-bar are automatically co-dependent
    if (len1 < len2
	&& strncmp (name1.c_str(), name2.c_str(), len1) == 0
	&& name2[len1] == '-') {
	return true;
    }
    
    return false;
}


bool
QueueItemRequire::process (ResolverContextPtr context, QueueItemList & new_items)
{
    if (getenv ("RC_SPEW")) fprintf (stderr, "QueueItemRequire::process(%s)\n", this->asString().c_str());

    if (context->requirementIsMet (_dep, _is_child)) {
	if (getenv ("RC_SPEW")) fprintf (stderr, "requirement is already met in current context\n");
//    rc_queue_item_free (item);
	return true;
    }

    RequireProcessInfo info;

    info.resolvable = _requiring_resolvable;
    info.dep = _is_child ? _dep : NULL;
    info.context = context;
    info.world = world();
    info.uniq = new UniqTable();		//FIXME: op: g_hash_table_new (rc_resolvable_spec_hash, rc_resolvable_spec_equal);

    int num_providers = 0;

    if (! _remove_only) {

	world()->foreachProvidingResolvable (_dep, require_process_cb, &info);
	
	num_providers = info.providers.size();

	if (getenv ("RC_SPEW")) fprintf (stderr, "requirement is met by %d resolvables\n", num_providers);
    }

    std::string msg;

    if (num_providers == 0) {

	if (getenv ("RC_SPEW")) fprintf (stderr, "Unfulfilled requirement, try different solution\n");

	QueueItemUninstallPtr uninstall_item = NULL;
	QueueItemBranchPtr branch_item = NULL;
	bool explore_uninstall_branch = true;

	if (_upgraded_resolvable == NULL) {
	    ResolverInfoPtr err_info;

	    msg = string ("There are no ") + (_remove_only ? "alternative installed" : "installable") + " providers of " + _dep->asString();
	    if (_requiring_resolvable != NULL) {
		msg += " for ";
		msg += _requiring_resolvable->asString();
	    }

	    err_info = new ResolverInfoMisc (_requiring_resolvable, RESOLVER_INFO_PRIORITY_VERBOSE, msg);

	    context->addInfo (err_info);

	    // Maybe we can add some extra info on why none of the providers are suitable.
	    world()->foreachProvidingResolvable (_dep, no_installable_providers_info_cb, (void *)&info);
	}
	
	// If this is an upgrade, we might be able to avoid removing stuff by upgrading it instead.
	if (_upgraded_resolvable != NULL
	    && _requiring_resolvable != NULL) {

	    CResolvableList upgrade_list;

	    world()->foreachUpgrade (_requiring_resolvable, new Channel(CHANNEL_TYPE_ANY), look_for_upgrades_cb, (void *)&upgrade_list);

	    if (!upgrade_list.empty()) {
		string label, req_str, up_str;

		branch_item = new QueueItemBranch (world());

		req_str = _requiring_resolvable->asString();
		up_str  = _upgraded_resolvable->asString();

		label = string ("for requiring ") + _dep->asString() + " for " + req_str + " when upgrading " + up_str;
		branch_item->setLabel (label);
//fprintf (stderr, "Branching: %s\n", label.c_str());
		for (CResolvableList::const_iterator iter = upgrade_list.begin(); iter != upgrade_list.end(); iter++) {
		    constResolvablePtr upgrade_resolvable = *iter;
		    QueueItemInstallPtr install_item;

		    if (context->resolvableIsPossible (upgrade_resolvable)) {
		    
			install_item = new QueueItemInstall (world(), upgrade_resolvable);
		    	install_item->setUpgrades (_requiring_resolvable);
			branch_item->addItem (install_item);

			ResolverInfoNeededByPtr upgrade_info = new ResolverInfoNeededBy (upgrade_resolvable);
			upgrade_info->addRelatedResolvable (_upgraded_resolvable);
			install_item->addInfo (upgrade_info);

			// If an upgrade resolvable has its requirements met, don't do the uninstall branch.
			//   FIXME: should we also look at conflicts here?

			if (explore_uninstall_branch) {
			    CDependencyList requires = upgrade_resolvable->requires();
			    CDependencyList::const_iterator iter = requires.begin();
			    for (; iter != requires.end(); iter++) {
				constDependencyPtr req = *iter;
				if (! context->requirementIsMet (req, false)) {
					break;
				}
			    }
			    if (iter == requires.end()) {
				explore_uninstall_branch = false;
			    }
			}
			
		    } /* if (context->resolvableIsPossible ( ... */
		} /* for (iter = upgrade_list; ... */
	    } /* if (upgrade_list) ... */

	    if (!upgrade_list.empty()
		&& branch_item->isEmpty ()) {

		for (CResolvableList::const_iterator iter = upgrade_list.begin(); iter != upgrade_list.end(); iter++) {
		    string str;
		    string p1, p2;

		    p1 = _requiring_resolvable->asString();
		    p2 = (*iter)->asString();
		    str = string ("Upgrade to ") + p2 + " to avoid removing " + p1 + " is not possible.";

		    ResolverInfoMiscPtr misc_info = new ResolverInfoMisc (NULL, RESOLVER_INFO_PRIORITY_VERBOSE, str);
		    misc_info->addRelatedResolvable (_requiring_resolvable);
		    misc_info->addRelatedResolvable (*iter);
		    context->addInfo (misc_info);

		    explore_uninstall_branch = true;
		}

		//
		//  The exception: we always want to consider uninstalling
		//  when the requirement has resulted from a resolvable losing
		//  one of it's provides.
		
	    } else if (!upgrade_list.empty()
		       && explore_uninstall_branch
		       && codependent_resolvables (_requiring_resolvable, _upgraded_resolvable)
		       && _lost_resolvable == NULL) {
		explore_uninstall_branch = false;
	    }

	} /* if (_upgrade_resolvable && _requiring_resolvable) ... */

	// We always consider uninstalling when in verification mode.

	if (context->verifying()) {
	    explore_uninstall_branch = true;
	}

	if (explore_uninstall_branch && _requiring_resolvable) {
	    ResolverInfoPtr log_info;
	    uninstall_item = new QueueItemUninstall (world(),_requiring_resolvable, "unsatisfied requirements");
	    uninstall_item->setDependency (_dep);
	    
	    if (_lost_resolvable) {
		log_info = new ResolverInfoDependsOn (_requiring_resolvable, _lost_resolvable);
		uninstall_item->addInfo (log_info);
	    }
	    
	    if (_remove_only)
		uninstall_item->setRemoveOnly ();
	}

	if (uninstall_item && branch_item) {
	    branch_item->addItem (uninstall_item);            
	    new_items.push_front (branch_item);
	} else if (uninstall_item) {
	    new_items.push_front (uninstall_item);
	} else if (branch_item) {
	    new_items.push_front (branch_item);
	} else {
	    // We can't do anything to resolve the missing requirement, so we fail.
	    string msg = string ("Can't satisfy requirement '") + _dep->asString() + "'";
	    
	    context->addErrorString (NULL, msg);
	}
	
    } else if (num_providers == 1) {

	if (getenv ("RC_SPEW")) fprintf (stderr, "Found exactly one resolvable, installing it.\n");

	QueueItemInstallPtr install_item = new QueueItemInstall (world(), info.providers.front());
	install_item->addDependency (_dep);

	// The requiring resolvable could be NULL if the requirement was added as an extra dependency.
	if (_requiring_resolvable) {
	    install_item->addNeededBy (_requiring_resolvable);
	}
	new_items.push_front (install_item);

    } else if (num_providers > 1) {

	if (getenv ("RC_SPEW")) fprintf (stderr, "Found more than one resolvable, branching.\n");

//fprintf (stderr, "Found more than one resolvable, branching.\n");
	QueueItemBranchPtr branch_item = new QueueItemBranch (world());

	for (CResolvableList::const_iterator iter = info.providers.begin(); iter != info.providers.end(); iter++) {
	    QueueItemInstallPtr install_item = new QueueItemInstall (world(), *iter);
	    install_item->addDependency (_dep);
	    branch_item->addItem (install_item);

	    // The requiring resolvable could be NULL if the requirement was added as an extra dependency.
	    if (_requiring_resolvable) {
		install_item->addNeededBy (_requiring_resolvable);
	    }
	}

	new_items.push_front (branch_item);

    } else {
	abort ();
    }

   
//    rc_queue_item_free (item);
    return true;
}

//---------------------------------------------------------------------------

QueueItemPtr
QueueItemRequire::copy (void) const
{
    QueueItemRequirePtr new_require = new QueueItemRequire (world(), _dep);
    ((QueueItemPtr)new_require)->copy((constQueueItemPtr)this);

    new_require->_requiring_resolvable = _requiring_resolvable;
    new_require->_upgraded_resolvable  = _upgraded_resolvable;
    new_require->_remove_only          = _remove_only;

    return new_require;
}


int
QueueItemRequire::cmp (constQueueItemPtr item) const
{
    int cmp = this->compare (item);		// assures equal type
    if (cmp != 0)
	return cmp;

    constQueueItemRequirePtr require = item;

    cmp = GVersion.compare ((constSpecPtr) _dep, ((constSpecPtr)(require->dependency())));
    if (cmp)
	return cmp;

    return CMP ((int) _dep->relation().op(), (int) (require->dependency()->relation().op()));
}

///////////////////////////////////////////////////////////////////
}; // namespace ZYPP
///////////////////////////////////////////////////////////////////

