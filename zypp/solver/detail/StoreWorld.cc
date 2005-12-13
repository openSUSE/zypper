/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* StoreWorld.cc
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

#include "config.h"

#include <y2util/stringutil.h>

#include <zypp/solver/detail/Version.h>
#include <zypp/solver/detail/Packman.h>
#include <zypp/solver/detail/StoreWorld.h>
#include <zypp/solver/detail/ResolvableAndDependency.h>
#include <zypp/solver/detail/debug.h>

///////////////////////////////////////////////////////////////////
namespace ZYPP {
//////////////////////////////////////////////////////////////////

using namespace std;

IMPL_DERIVED_POINTER(StoreWorld, World);

//---------------------------------------------------------------------------

string
StoreWorld::asString ( void ) const
{
    return toString (*this);
}


string
StoreWorld::toString ( const StoreWorld & storeworld )
{
    string res ("<storeworld/>");

    return res;
}


ostream &
StoreWorld::dumpOn (ostream & str) const
{
    str << asString();
    return str;
}


ostream &
operator<< (ostream & os, const StoreWorld & storeworld)
{
    return os << storeworld.asString();
}

//---------------------------------------------------------------------------

StoreWorld::StoreWorld (WorldType type)
    : World (type)
    , _resolvable_kind(Kind::Unknown)
{
}


StoreWorld::~StoreWorld()
{
    fprintf (stderr, "*** deleting store world[%p]: %s\n", this, World::toString(type()).c_str());
}


//---------------------------------------------------------------------------

// Add/remove resolvables

bool
StoreWorld::addResolvable (constResolvablePtr resolvable)
{
    ArchList compat_arch_list;
    ResolvableAndDependencyPtr r_and_d;
    const char *package_name;
    constChannelPtr channel;
    int arch_score;
    bool actually_added_package = false;

    if (resolvable == NULL) return false;

    compat_arch_list = Arch::System->getCompatList();
//fprintf (stderr, "Arch::System '%s' -> %d compats\n", Arch::System->asString().c_str(), (int) compat_arch_list.size());
    channel = resolvable->channel ();

//    fprintf (stderr, "StoreWorld[%p]::addResolvable(%s) [%s]\n", this, ((constSpecPtr)resolvable)->asString().c_str(), channel?channel->name():"?");

    arch_score = resolvable->arch()->getCompatScore(compat_arch_list);

    /* Before we do anything, check to make sure that a resolvable of the
       same name isn't already in that channel.  If there is a
       duplicate, we keep the one with the most recent version number
       and drop the other.

       This check only applies to resolvables in a channel.  We have
       to allow for multiple installs.  Grrrr...
    */

    if (!resolvable->isInstalled ()) {			// its not a system package

	constResolvablePtr dup_res;
	int dup_arch_score;

	/* Filter out resolvables with totally incompatible arches */
	if (arch_score < 0) {
	    rc_debug (RC_DEBUG_LEVEL_DEBUG, "Ignoring resolvable with incompatible arch: Arch '%s', %s",  resolvable->arch()->asString().c_str(), resolvable->asString(true).c_str());
	    goto finished;
	}

	package_name = resolvable->name().c_str();
	dup_res = findResolvable (channel, package_name);

	/* This shouldn't happen (and would be caught by the check
	   below, because cmp will equal 0), but it never hurts to
	   check and produce a more explicit warning message. */

	if (resolvable == dup_res) {
	    rc_debug (RC_DEBUG_LEVEL_WARNING, "Ignoring re-add of resolvable '%s'", package_name);
	    goto finished;
	}

	if (dup_res != NULL) {
	    int cmp;

	    cmp = GVersion.compare (resolvable, dup_res);
//fprintf (stderr, "res: %s, dup_res %s, cmp %d\n", resolvable->asString().c_str(), dup_res->asString().c_str(), cmp);
	    dup_arch_score = dup_res->arch()->getCompatScore(compat_arch_list);
	

	    /* If the resolvable we are trying to add has a lower 
	       version number, just ignore it. */

	    if (cmp < 0) {
		rc_debug (RC_DEBUG_LEVEL_INFO, "Not adding resolvable '%s'.\n\tA newer version is already in the channel.", resolvable->asString().c_str());
		rc_debug (RC_DEBUG_LEVEL_INFO, "\t%s", dup_res->asString().c_str());
		goto finished;
	    }


	    /* If the version numbers are equal, we ignore the resolvable to
	       add if it has a less-preferable arch.  If both
	       resolvables have the same version # and arch, we favor the
	       first resolvable and just return. */

	    if (cmp == 0 && arch_score > dup_arch_score) {
		rc_debug (RC_DEBUG_LEVEL_INFO, "Not adding resolvable '%s'.\n\tAnother resolvable with the same version but with a preferred arch is already in the channel.", resolvable->asString().c_str());
		rc_debug (RC_DEBUG_LEVEL_INFO, "\t%s", dup_res->asString().c_str());
		goto finished;
	    }


	    /* Otherwise we throw out the old resolvable and proceed with
	       adding the newer one. */

	    rc_debug (RC_DEBUG_LEVEL_INFO, "Replacing resolvable '%s'.\n\tAnother resolvable in the channel has the same name and a superior %s.",  dup_res->asString().c_str(), cmp ? "version" : "arch");
	    rc_debug (RC_DEBUG_LEVEL_INFO, "\t%s", resolvable->asString().c_str());

	    removeResolvable (dup_res);
	}
    }

    actually_added_package = true;

    if (channel && !channel->hidden()) {
	touchResolvableSequenceNumber ();
    }

    /* StoreWorld all of our resolvables in a hash by name. */
    _resolvables_by_name.insert (ResolvableTable::value_type (resolvable->name(), resolvable));

    /* StoreWorld all of the resolvable's provides in a hash by name. */
    for (CDependencyList::const_iterator i = resolvable->provides().begin(); i != resolvable->provides().end(); i++) {
	r_and_d = new ResolvableAndDependency (resolvable, *i);

	_provides_by_name.insert (ResolvableAndDependencyTable::value_type (r_and_d->dependency()->name(), r_and_d));
    }

    /* StoreWorld all of the resolvable's requires in a hash by name. */

    for (CDependencyList::const_iterator i = resolvable->requires().begin(); i != resolvable->requires().end(); i++) {
	r_and_d = new ResolvableAndDependency (resolvable, *i);

	_requires_by_name.insert (ResolvableAndDependencyTable::value_type (r_and_d->dependency()->name(), r_and_d));
    }

    /* "Recommends" are treated as requirements. */
#warning Recommends are treated as requirements

    for (CDependencyList::const_iterator i = resolvable->recommends().begin(); i != resolvable->recommends().end(); i++) {
	r_and_d = new ResolvableAndDependency (resolvable, *i);

	_requires_by_name.insert (ResolvableAndDependencyTable::value_type (r_and_d->dependency()->name(), r_and_d));
    }

    /* StoreWorld all of the resolvable's conflicts in a hash by name. */

    for (CDependencyList::const_iterator i = resolvable->conflicts().begin(); i != resolvable->conflicts().end(); i++) {
	r_and_d = new ResolvableAndDependency (resolvable, *i);
	_conflicts_by_name.insert (ResolvableAndDependencyTable::value_type (r_and_d->dependency()->name(), r_and_d));
    }
    
 finished:

    return actually_added_package;
}


void
StoreWorld::addResolvablesFromList (const CResolvableList & slist)
{
    for (CResolvableList::const_iterator i = slist.begin(); i != slist.end(); i++) {
	if (!addResolvable (*i)) {
	    fprintf (stderr, "addResolvable failed\n");
	    break;
	}
    }
    return;
}

//---------------------------------------------------------------------------

static void
resolvable_table_remove (ResolvableTable & table, constResolvablePtr resolvable)
{
    const string name = resolvable->name();
    for (ResolvableTable::iterator pos = table.lower_bound(name); pos != table.upper_bound(name); pos++) {
	constResolvablePtr res = pos->second;
	if (res == resolvable) {
	    table.erase (pos);
	    break;
	}
    }
    return;
}


static void
resolvable_and_dependency_table_remove (ResolvableAndDependencyTable & table, constResolvablePtr resolvable)
{
    const string name = resolvable->name();
// FIXME: this is inefficient but lower_bound can't to strcasecmp :-(
//    for (ResolvableAndDependencyTable::iterator pos = table.lower_bound(name); pos != table.upper_bound(name); pos++) {
    for (ResolvableAndDependencyTable::iterator pos = table.begin(); pos != table.end(); pos++) {
	constResolvableAndDependencyPtr r_and_d = pos->second;
	if (r_and_d->resolvable() == resolvable) {
	    table.erase (pos);
	    break;
	}
    }
    return;
}

void
StoreWorld::removeResolvable (constResolvablePtr resolvable)
{
    if (getenv("RC_SPEW")) fprintf (stderr, "StoreWorld::removeResolvable (%s)\n", resolvable->asString().c_str());

    constChannelPtr channel = resolvable->channel ();

    if (! (channel && channel->hidden ()))
	touchResolvableSequenceNumber ();

    resolvable_and_dependency_table_remove (_provides_by_name, resolvable);
    resolvable_and_dependency_table_remove (_requires_by_name, resolvable);
    resolvable_and_dependency_table_remove (_conflicts_by_name, resolvable);

    resolvable_table_remove (_resolvables_by_name, resolvable);

    return;
}


void
StoreWorld::removeResolvables (constChannelPtr channel)
{
	fprintf (stderr, "StoreWorld::removeResolvables(%s) not implemented\n", channel->asString().c_str());
}


void
StoreWorld::clear ()
{
	fprintf (stderr, "StoreWorld::clear() not implemented\n");
}

//---------------------------------------------------------------------------
// Single resolvable queries

static bool
installed_version_cb (constResolvablePtr resolvable, void *data)
{
    constResolvablePtr *installed = (constResolvablePtr *)data;

    if (resolvable->isInstalled ()) {
	*installed = resolvable;
	return false;
    }
    return true;
}


constResolvablePtr
StoreWorld::findInstalledResolvable (constResolvablePtr resolvable)
{
    constResolvablePtr installed;
    sync ();

    foreachResolvableByName (resolvable->name(), new Channel(CHANNEL_TYPE_ANY) /* is this right? */, installed_version_cb, &installed);

    return installed;
}


//
// findResolvable 
// @channel: A non-wildcard #Channel.
// @name: The name of a resolvable.
//
// Searches the world for a resolvable in the specified channel
// with the specified name.  @channel must be an actual
// channel, not a wildcard.
//
// Return value: The matching resolvable, or %NULL if no such
// resolvable exists.
//

constResolvablePtr
StoreWorld::findResolvable (constChannelPtr channel, const char *name) const
{
    syncConditional (channel);
    for (ResolvableTable::const_iterator pos = _resolvables_by_name.lower_bound(name); pos != _resolvables_by_name.upper_bound(name); pos++) {
	constResolvablePtr res = pos->second;
	if (res->channel() == channel) {
	    return res;
	}
    }
    return NULL;
}


constResolvablePtr
StoreWorld::findResolvableWithConstraint (constChannelPtr channel, const char *name, constDependencyPtr constraint, bool is_and) const
{
	fprintf (stderr, "StoreWorld::findResolvableWithConstraint() not implemented\n");
    return 0;
}


ChannelPtr
StoreWorld::guessResolvableChannel (constResolvablePtr resolvable) const
{
	fprintf (stderr, "StoreWorld::guessResolvableChannel(%s) not implemented\n", ((constSpecPtr)resolvable)->asString().c_str());
    return 0;
}


//-----------------------------------------------------------------------------
// foreach resolvable

typedef struct {
    ChannelPtr channel;
    CResolvableFn callback;
    void *data;
    int count;
    bool short_circuit;
} ForeachResolvableInfo;


static void
foreach_resolvable_cb (const string &name, constResolvablePtr resolvable, void *data)
{
    ForeachResolvableInfo *info = (ForeachResolvableInfo *)data;

    if (info->short_circuit)
	return;

    /* FIXME: we should filter out dup uninstalled resolvables. */

    if (resolvable && info->channel->equals(resolvable->channel ())) {
	if (info->callback) {
	    if (! info->callback (resolvable, info->data))
		info->short_circuit = true;
	}
	++info->count;
    }
}


int
StoreWorld::foreachResolvable (ChannelPtr channel, CResolvableFn fn, void *data)
{
    return foreachResolvableByName ("", channel, fn, data);
}


int
StoreWorld::foreachResolvableByName (const std::string & name, ChannelPtr channel, CResolvableFn fn, void *data)
{
    if (name.empty()) {

	ForeachResolvableInfo info;

	info.channel = channel;
	info.callback = fn;
	info.data = data;
	info.count = 0;
	info.short_circuit = false;

	for (ResolvableTable::const_iterator iter = _resolvables_by_name.begin(); iter != _resolvables_by_name.end(); iter++) {
	    foreach_resolvable_cb (iter->first, iter->second, (void *)&info);
	}

	return info.short_circuit ? -1 : info.count;
    }

	
    ResolvableTable installed;			// FIXME: <Spec, Resolvable> rc_resolvable_spec_equal
    int count = 0;

    for (ResolvableTable::const_iterator iter = _resolvables_by_name.lower_bound(name); iter != _resolvables_by_name.upper_bound(name); iter++) {
	constResolvablePtr resolvable = iter->second;
	if (resolvable->isInstalled()) {
	    const string str = ((constSpecPtr)resolvable)->asString();
	    installed.insert (ResolvableTable::value_type(str,resolvable));
	}
    }    

    for (ResolvableTable::const_iterator iter = _resolvables_by_name.lower_bound(name); iter != _resolvables_by_name.upper_bound(name); iter++) {
	constResolvablePtr resolvable = iter->second;
	if (channel->equals (resolvable->channel())) {
	    if (resolvable->isInstalled()
		|| installed.find(((constSpecPtr)resolvable)->asString()) == installed.end()) {
		if (fn) {
		    if (! fn(resolvable, data)) {
			count = -1;
			goto finished;
		    }
		}
		++count;
	    }
	}
    }

finished:

    return count;
}


int
StoreWorld::foreachResolvableByMatch (constMatchPtr match, CResolvableFn fn, void *data)
{
	fprintf (stderr, "StoreWorld::foreachResolvableByMatch () not implemented\n");
    return 0;
}


//-----------------------------------------------------------------------------
// iterater over resolvables with dependency

typedef std::map<constSpecPtr, constResolvableAndDependencyPtr> InstalledTable;

int
StoreWorld::foreachProvidingResolvable (constDependencyPtr dep, ResolvableAndSpecFn fn, void *data)
{
    int count = 0;
    InstalledTable installed;
//fprintf (stderr, "StoreWorld::foreachProvidingResolvable(%s)\n", dep->asString().c_str());
    for (ResolvableAndDependencyTable::const_iterator iter = _provides_by_name.lower_bound(dep->name()); iter != _provides_by_name.upper_bound(dep->name()); iter++) {
	constResolvableAndDependencyPtr r_and_d = iter->second;
	constResolvablePtr res = r_and_d->resolvable();
//fprintf (stderr, "StoreWorld::foreachProvidingResolvable(): %s\n", res->asString(true).c_str());
	if (res != NULL && res->isInstalled ()) {
	    installed[res] = r_and_d;
	}
    }

    for (ResolvableAndDependencyTable::const_iterator iter = _provides_by_name.lower_bound(dep->name()); iter != _provides_by_name.upper_bound(dep->name()); iter++) {
	constResolvableAndDependencyPtr r_and_d = iter->second;

	if (r_and_d && r_and_d->verifyRelation (dep)) {
//fprintf (stderr, "found: %s\n", r_and_d->resolvable()->asString(true).c_str());
	    /* If we have multiple identical resolvables in RCWorld,
	       we want to only include the resolvable that is installed and
	       skip the rest. */
	    if (r_and_d->resolvable()->isInstalled()
		|| installed.find(r_and_d->resolvable()) == installed.end()) {

		if (fn) {
		    if (! fn(r_and_d->resolvable(), r_and_d->dependency(), data)) {
			count = -1;
			goto finished;
		    }
		}
		++count;
	    }
	}
    }

 finished:

    return count;
}

int
StoreWorld::foreachRequiringResolvable (constDependencyPtr dep, ResolvableAndDepFn fn, void *data)
{
    int count = 0;
    InstalledTable installed;


    for (ResolvableAndDependencyTable::const_iterator iter = _requires_by_name.lower_bound(dep->name()); iter != _requires_by_name.upper_bound(dep->name()); iter++) {
	constResolvableAndDependencyPtr r_and_d = iter->second;
	constResolvablePtr res = r_and_d->resolvable();
	if (res != NULL && res->isInstalled ()) {
//fprintf (stderr, "is installed: %s\n", res->asString(true).c_str());
	    installed[res] = r_and_d;
	}
    }

    for (ResolvableAndDependencyTable::const_iterator iter = _requires_by_name.lower_bound(dep->name()); iter != _requires_by_name.upper_bound(dep->name()); iter++) {
	constResolvableAndDependencyPtr r_and_d = iter->second;

	if (r_and_d && r_and_d->dependency()->verifyRelation (dep)) {

	    /* Skip dups if one of them in installed. */
	    if (r_and_d->resolvable()->isInstalled()
		|| installed.find(r_and_d->resolvable()) == installed.end()) {

		if (fn) {
		    if (! fn(r_and_d->resolvable(), r_and_d->dependency(), data)) {
			count = -1;
			goto finished;
		    }
		}
		++count;
	    }
	}
    }

 finished:

    return count;
}


int
StoreWorld::foreachConflictingResolvable (constDependencyPtr dep, ResolvableAndDepFn fn, void *data)
{
    int count = 0;
    InstalledTable installed;
//fprintf (stderr, "StoreWorld::foreachConflictingResolvable (%s)\n", dep->name().c_str());
    for (ResolvableAndDependencyTable::const_iterator iter = _conflicts_by_name.lower_bound(dep->name()); iter != _conflicts_by_name.upper_bound(dep->name()); iter++) {
	constResolvableAndDependencyPtr r_and_d = iter->second;
	constResolvablePtr res = r_and_d->resolvable();
//fprintf (stderr, "==> %s\n", res->asString().c_str());
	if (res != NULL && res->isInstalled ()) {
	    installed[res] = r_and_d;
	}
    }

    for (ResolvableAndDependencyTable::const_iterator iter = _conflicts_by_name.lower_bound(dep->name()); iter != _conflicts_by_name.upper_bound(dep->name()); iter++) {
	constResolvableAndDependencyPtr r_and_d = iter->second;

	if (r_and_d)
//fprintf (stderr, "==> %s verify %s ? %s\n", r_and_d->asString().c_str(), dep->asString().c_str(), r_and_d->verifyRelation (dep) ? "Y" : "N");
	if (r_and_d && r_and_d->verifyRelation (dep)) {

	    /* Skip dups if one of them in installed. */
	    if (r_and_d->resolvable()->isInstalled()
		|| installed.find(r_and_d->resolvable()) == installed.end()) {

		if (fn) {
		    if (! fn(r_and_d->resolvable(), r_and_d->dependency(), data)) {
			count = -1;
			goto finished;
		    }
		}
		++count;
	    }
	}
    }

 finished:

    return count;
}

//-----------------------------------------------------------------------------
// channel functions

void
StoreWorld::addChannel (ChannelPtr channel)
{
    if (channel == NULL) return;

    channel->setWorld (this);
    channel->setImmutable (true);

    _channels.push_back (channel);

    touchChannelSequenceNumber ();
}


void
StoreWorld::removeChannel (constChannelPtr channel)
{
    if (channel == NULL
	|| ! containsChannel (channel))
	return;

    removeResolvables (channel);

    for (ChannelList::iterator iter = _channels.begin(); iter != _channels.end(); iter++) {
	if ((*iter)->equals (channel)) {
	    _channels.erase (iter);
	    touchChannelSequenceNumber ();
	    break;
	}
    }
}


bool
StoreWorld::containsChannel (constChannelPtr channel) const
{
    for (ChannelList::const_iterator iter = _channels.begin(); iter != _channels.end(); iter++) {
	if ((*iter)->equals (channel)) {
	    return true;
	}
    }
    return false;
}


ChannelPtr
StoreWorld::getChannelByName (const char *channel_name) const
{
    if (channel_name == NULL
	|| *channel_name == 0) {
	return NULL;
    }

    for (ChannelList::const_iterator iter = _channels.begin(); iter != _channels.end(); iter++) {
	if (strcasecmp ((*iter)->name(), channel_name) == 0) {
	    return *iter;
	}
    }
    return NULL;
}


ChannelPtr
StoreWorld::getChannelByAlias (const char *alias) const
{
    if (alias == NULL
	|| *alias == 0) {
	return NULL;
    }

    for (ChannelList::const_iterator iter = _channels.begin(); iter != _channels.end(); iter++) {
	if (strcasecmp ((*iter)->alias(), alias) == 0) {
	    return *iter;
	}
    }
    return NULL;
}


ChannelPtr
StoreWorld::getChannelById (const char *channel_id) const
{
    if (channel_id == NULL
	|| *channel_id  == 0) {
	return NULL;
    }

    for (ChannelList::const_iterator iter = _channels.begin(); iter != _channels.end(); iter++) {
	if (strcasecmp ((*iter)->id(), channel_id) == 0) {
	    return *iter;
	}
    }
    return NULL;
}


int
StoreWorld::foreachChannel (ChannelFn fn, void *data) const
{
    int count = 0;
    for (ChannelList::const_iterator iter = _channels.begin(); iter != _channels.end(); iter++) {
	if (!(*fn) (*iter, data))
	    return -1;
	count++;
    }
    return count;
}

///////////////////////////////////////////////////////////////////
}; // namespace ZYPP
///////////////////////////////////////////////////////////////////

