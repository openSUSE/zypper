/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResolverInfoMisc.cc
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

#include <map>
#include <sstream>

#include "zypp/solver/detail/ResolverInfo.h"
#include "zypp/solver/detail/ResolverInfoMisc.h"
#include "zypp/base/String.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Gettext.h"

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

IMPL_PTR_TYPE(ResolverInfoMisc);

//---------------------------------------------------------------------------


std::ostream &
ResolverInfoMisc::dumpOn( std::ostream & os ) const
{
    ResolverInfo::dumpOn (os);		// this already shows the affected item

    os << ">>" << message() << "<<";
    os << itemsToString(true);
    if (!_action.empty()) {
	os << _(", Action: ") << _action << endl;
    }
    os << _(", Trigger: ");
    switch (_trigger) {
	case ResolverInfoMisc::NONE:		os << "none"; break;
	case ResolverInfoMisc::OBSOLETE:	os << "obsoletes"; break;
	case ResolverInfoMisc::REQUIRE:		os << "requires"; break;	    
	case ResolverInfoMisc::CONFLICT:	os << "conflicts"; break;
    }
    os << endl;
    return os;
}

//---------------------------------------------------------------------------

ResolverInfoMisc::ResolverInfoMisc  (ResolverInfoType detailedtype, PoolItem_Ref affected, int priority, const Capability & capability)
    : ResolverInfoContainer (detailedtype, affected, priority)
      , _capability (capability)
      , _trigger (NONE)
{
}


ResolverInfoMisc::~ResolverInfoMisc ()
{
}

//---------------------------------------------------------------------------

static string
translateResTraits (const Resolvable::Kind & kind)
{
    if (kind == ResTraits<Package>::kind) {
	// Translator: Notation for (RPM) package
	return _( "package" );
    }
    else if (kind == ResTraits<Selection>::kind) {
	// Translator: Notation for SuSE package selection (set of packages)
	return _( "selection" );
    }
    else if (kind == ResTraits<Pattern>::kind) {
	// Translator: Notation for SuSE installation pattern (set of packages, describing use of system)
	return _( "pattern" );
    }
    else if (kind == ResTraits<Product>::kind) {
	// Translator: Notation for product
	return _( "product" );
    }
    else if (kind == ResTraits<Patch>::kind) {
	// Translator: Notation for patch
	return _( "patch" );
    }
    else if (kind == ResTraits<Script>::kind) {
	// Translator: Notation for script (part of a patch)
	return _( "script" );
    }
    else if (kind == ResTraits<Message>::kind) {
	// Translator: Notation for message (part of a patch)
	return _( "message" );
    }
    else if (kind == ResTraits<Atom>::kind) {
	// Translator: Notation for atom (part of a patch)
	return _( "atom" );
    }
    else if (kind == ResTraits<System>::kind) {
	// Translator: Notation for computer system
	return _( "system" );
    }

    // Translator: Generic term for an item with dependencies, please leave untranslated for now
    return _("Resolvable");
}


std::string
ResolverInfoMisc::message (void) const
{
    string msg;

    string affected_str = ResolverInfo::toString(affected());

    switch (type()) {

	//===================
	// from ResolverContext

	case RESOLVER_INFO_TYPE_INVALID_SOLUTION: {
	    // TranslatorExplanation: Additional information to dependency solver result, no solution could be found
	    msg = _("Marking this resolution attempt as invalid.");
	}
	break;

	case RESOLVER_INFO_TYPE_UNINSTALLABLE: {
	    // Translator: %s = name of packages,patch,...
	    // TranslatorExplanation: Additional information to dependency solver result.
	    msg = str::form (_("Marking resolvable %s as uninstallable"),
			affected_str.c_str());
	}
	break;

	case RESOLVER_INFO_TYPE_REJECT_INSTALL: {
	    // Translator: %s = name of packages,patch,...
	    // TranslatorExplanation: Additional information to dependency solver result.
	    msg = str::form (_("%s is scheduled to be installed, but this is not possible because of dependency problems."),
			affected_str.c_str());
	}
	break;

	case RESOLVER_INFO_TYPE_INSTALL_TO_BE_UNINSTALLED: {
	    // Translator: %s = name of package,patch,...
	    // TranslatorExplanation: Additional information to dependency solver result.
	    msg = str::form (_("Can't install %s since it is already marked as needing to be uninstalled"),
			 affected_str.c_str());
	}
	break;

	case RESOLVER_INFO_TYPE_INSTALL_UNNEEDED: {
	    // Translator: %s = name of patch
	    // TranslatorExplanation: A patch which is not needed (does not apply) cant be installed
	    // TranslatorExplanation: Patches contain updates (bug fixes) to packages. Such fixes
	    // TranslatorExplanation: do only apply if the package to-be-fixed is actually installed.
	    // TranslatorExplanation: Here a patch was selected for installation but the to-be-fixed
	    // TranslatorExplanation: package is not installed.
	    msg = str::form (_("Can't install %s since it is does not apply to this system."),
			affected_str.c_str());
	}
	break;

	case RESOLVER_INFO_TYPE_INSTALL_PARALLEL: {
	    // affected() = item 1 which has to be installed
	    // _capability =
	    // other() = item 2 which has to be installed
	    // other_capability() =
	    // Translator: %s = name of package,patch,...
	    msg = str::form (_("Can't install %s, since %s is already marked as needing to be installed"),
			     affected_str.c_str(),
			     toString (other()).c_str());
	}
	break;

	case RESOLVER_INFO_TYPE_INCOMPLETES: {
	    // Translator: %s = name of patch,product
	    msg = str::form (_("This would invalidate %s."),
			affected_str.c_str());
	}
	break;

	//===================
	// from QueueItemEstablish

	//-------------------
	// Establishing p
	case RESOLVER_INFO_TYPE_ESTABLISHING: {
	    // Translator: %s = name of patch, pattern, ...
	    // TranslatorExplanation: Establishing is the process of computing which patches are needed
	    // TranslatorExplanation: This is just a progress indicator
	    // TranslatorExplanation: It is also used for other types of resolvables in order to verify
	    // TranslatorExplanation: the completeness of their dependencies
	    msg = str::form (_("Establishing %s"), affected_str.c_str());
	}
	break;


	//===================
	// from QueueItemInstall

	//-------------------
	// Installing p

	case RESOLVER_INFO_TYPE_INSTALLING: {
	    // affected() = resolvable to be installed
	    // _capability =
	    // other() =
	    // other_capability() =

	    // Translator: %s = name of package,patch,...
	    // TranslatorExample: Installing foo
	    // TranslatorExplanation: Just a progress indicator that something is scheduled for installation
	    // Translator: %s = packagename
	    msg = str::form (_("Installing %s"),
			     affected_str.c_str());
	}
	break;

	//-------------------
	// Updating q to p

	case RESOLVER_INFO_TYPE_UPDATING: {
	    // affected() = updated resolvable
	    // _capability =
	    // other() = currently installed, being updated resolvable
	    // other_capability() =

	    // Translator: 1.%s and 2.%s = name of package
	    // TranslatorExample: Updating foo-1.1 to foo-1.2
	    // TranslatorExplanation: Just a progress indicator that something is scheduled for upgrade
	    msg = str::form (_("Updating %s to %s"),
			     ResolverInfo::toString (other()).c_str(),
			     affected_str.c_str());
	}
	break;

	//-------------------
	// skipping p, already installed

	case RESOLVER_INFO_TYPE_SKIPPING: {
	    // affected() =
	    // _capability =
	    // other() =
	    // other_capability() =

	    // Translator: %s = name of package,patch,...
	    // TranslatorExample: Skipping foo: already installed
	    // TranslatorExplanation: An installation request for foo is skipped since foo is already installed
	    msg = str::form (_("Skipping %s: already installed"), affected_str.c_str());
	}
	break;

	//===================
	// from QueueItemRequire

	//-------------------
	// There are no alternative installed providers of c [for p]

	case RESOLVER_INFO_TYPE_NO_OTHER_PROVIDER: {
	    // affected() = is set
	    // _capability = is set
	    // other() =
	    // other_capability() =

	    // Translator: 1.%s = dependency
	    // TranslatorExample: There are no alternative installed providers of foo
	    // TranslatorExplanation: A resolvable is to be uninstalled. It provides 'foo' which is needed by others
	    // TranslatorExplanation: We just found out that 'foo' is not provided by anything else (an alternative)
	    // TranslatorExplanation: removal of this resolvable would therefore break dependency
	    // TranslatorExplanation: This is an error message explaining that the resolvable cannot be uninstalled
	    msg = str::form (_("There are no alternative installed providers of %s"), ResolverInfo::toString (_capability).c_str());
	    if (affected()) {
		msg += " ";
		// Translator: 1.%s = name of package,patch....
		// TranslatorExample: for bar
		// TranslatorExplanation: extension to previous message if we know what the resolvable is
		msg += str::form (_("for %s"), affected_str.c_str());
	    }
	}
	break;

	//-------------------
	// There are no installable providers of c [for p]

	case RESOLVER_INFO_TYPE_NO_PROVIDER: {
	    // affected() =
	    // _capability =
	    // other() =
	    // other_capability() =

	    // Translator: 1.%s = dependency
	    // TranslatorExample: There are no installable providers of foo
	    // TranslatorExplanation: A resolvable is to be installed which requires foo
	    // TranslatorExplanation: But there is nothing available to fulfill this requirement
	    // TranslatorExplanation: This is an error message explaining that the resolvable cannot be installed
	    msg = str::form (_("There are no installable providers of %s"), ResolverInfo::toString (_capability).c_str());
	    if (affected()) {
		msg += " ";
		// Translator: 1.%s = name of package,patch....
		// TranslatorExample: for bar
		// TranslatorExplanation: extension to previous message if we know what the resolvable is
		msg += str::form (_("for %s"), affected_str.c_str());
	    }
	}
	break;

	//-------------------
	// Upgrade to q to avoid removing p is not possible

	case RESOLVER_INFO_TYPE_NO_UPGRADE: {
	    // affected() = resolvable to be removed
	    // _capability =
	    // other() = failed upgrade to affected()
	    // other_capability() =

	    // Translator: 1.%s = name of package,patch,..., 2.%s = name of package,patch,...
	    // TranslatorExample: Upgrade to foo to avoid removing bar is not possible
	    // TranslatorExplanation: bar requires something from foo
	    msg = str::form (_("Upgrade to %s to avoid removing %s is not possible."),
				    ResolverInfo::toString (other()).c_str(),
				    affected_str.c_str());
	}
	break;

	//-------------------
	// p provides c but is scheduled to be uninstalled

	case RESOLVER_INFO_TYPE_UNINSTALL_PROVIDER: {
	    // affected() = requirer of capability
	    // _capability = provided by other()
	    // other() = provider of capability
	    // other_capability() = - empty -

	    // Translator: 1.%s = name of package,patch,...; 2.%s = dependency;
	    msg = str::form (_("%s provides %s, but is scheduled to be uninstalled."),
			     ResolverInfo::toString (other()).c_str(),
			     ResolverInfo::toString (_capability).c_str());
	}
	break;

	//-------------------
	// p provides c but another version is already installed

	case RESOLVER_INFO_TYPE_PARALLEL_PROVIDER: {
	    // affected() = requirer of capability
	    // _capability = provided by other()
	    // other() = provider of capability
	    // other_capability() = - empty -

	    // Translator: 1.%s = name of package,patch,...; 2.%s = dependency; 3.%s type (package, patch, ...)
	    msg = str::form (_("%s provides %s, but another version of that %s is already installed."),
			     other()->name().c_str(),
			     ResolverInfo::toString (_capability).c_str(),
			     translateResTraits(other()->kind()).c_str());
	}
	break;

	//-------------------
	// p provides c but is uninstallable

	case RESOLVER_INFO_TYPE_NOT_INSTALLABLE_PROVIDER: {
	    // affected() = requirer of capability
	    // _capability = provided by other()
	    // other() = provider of capability
	    // other_capability() = - empty -

	    // Translator: 1.%s = name of package,patch,...; 2.%s = dependency;
	    msg = str::form (_("%s provides %s, but it is uninstallable.  Try installing it on its own for more details."),
			     other()->name().c_str(),
			     ResolverInfo::toString (_capability).c_str());
	}
	break;

	//-------------------
	// p provides c but is locked

	case RESOLVER_INFO_TYPE_LOCKED_PROVIDER: {
	    // affected() = requirer of capability
	    // _capability = provided by other()
	    // other() = provider of capability
	    // other_capability() = - empty -

	    // Translator: 1.%s = name of package,patch,...; 2.%s = dependency;
	    msg = str::form (_("%s provides %s, but it is locked."),
			     other()->name().c_str(),
			     ResolverInfo::toString (_capability).c_str());
	}
	break;

	//-------------------
	// Can't satisfy requirement

	case RESOLVER_INFO_TYPE_CANT_SATISFY: {
	    // affected() = requirer of capability
	    // _capability = required capability
	    // other() = - empty -
	    // other_capability() = - empty -

	    // Translator: 1.%s = dependency. 2.%s name of package, patch, ...
	    msg = str::form (_("Can't satisfy requirement %s for %s"),
				ResolverInfo::toString (_capability).c_str(),
				affected_str.c_str());
	}
	break;

	//===================
	// from QueueItemUninstall

	//-------------------
	// p is to-be-installed, so it won't be unlinked.

	case RESOLVER_INFO_TYPE_UNINSTALL_TO_BE_INSTALLED: {
	    // affected() = to-be-installed resolvable which was scheduled to be uninstalled
	    // _capability =
	    // other() =
	    // other_capability() =

	    // Translator: %s = name of package,patch,...
	    // TranslatorExample: foo is required by other to-be-installed resolvable, so it won't be unlinked.
	    // TranslatorExplanation: Cant uninstall foo since it is required by an to-be-installed resolvable
	    msg = str::form (_("%s is required by other to-be-installed resolvable, so it won't be unlinked."),
			     affected()->name().c_str());
	}
	break;

	//-------------------
	// p is required by another installed resolvable q, so it won't be unlinked.

	case RESOLVER_INFO_TYPE_UNINSTALL_INSTALLED: {
	    // affected() = provider of cap
	    // _capability =
	    // other() =
	    // other_capability() =

	    // Translator: %s = name of package,patch,...
	    // TranslatorExample: foo is required by other installed resolvable, so it won't be unlinked.
	    // TranslatorExplanation: Cant uninstall foo since it is required by an installed resolvable
	    msg = str::form (_("%s is required by other installed resolvable, so it won't be unlinked."),
			     affected()->name().c_str());
	}
	break;

	//-------------------
	// cant uninstall, its locked

	case RESOLVER_INFO_TYPE_UNINSTALL_LOCKED: {
	    // affected() = to-be-uninstalled resolvable which is locked
	    // _capability =
	    // other() =
	    // other_capability() =

	    // Translator: %s = name of package,patch,...
	    // TranslatorExample: foo is locked and cannot be uninstalled.
	    // TranslatorExplanation: foo is to-be-uninstalled but it is locked
	    msg = str::form (_("%s is locked and cannot be uninstalled."),
			     affected()->name().c_str());
	}
	break;

	//===================
	// from QueueItemConflict

	//-------------------
	// to-be-installed p conflicts with q due to c

	case RESOLVER_INFO_TYPE_CONFLICT_CANT_INSTALL: {
	    // affected() = provider of capability
	    // _capability = provided by provider
	    // other() = conflict issuer
	    // other_capability() = conflict capability

	    // Translator: 1.%s and 2.%s = Dependency; 4.%s = name of package,patch,...
	    // TranslatorExample: A conflict over foo (bar) requires the removal of to-be-installed xyz
	    msg = str::form(_("A conflict over %s (%s) requires the removal of to-be-installed %s"),
			    ResolverInfo::toString (_capability).c_str(),
			    ResolverInfo::toString (_other_capability).c_str(),
			    affected_str.c_str());
	}
	break;

	//-------------------
	// uninstalled p is marked uninstallable it conflicts [with q] due to c

	case RESOLVER_INFO_TYPE_CONFLICT_UNINSTALLABLE:	{
	    // affected() = provider of capability
	    // _capability = provided by provider
	    // other() = conflict issuer
	    // other_capability() = conflict capability from issuer

	    // Translator: 1.%s = name of package,patch,...; 3.%s and 4.%s = Dependency;
	    // TranslatorExample: Marking xyz as uninstallable due to conflicts over foo (bar)
	    msg = str::form (_("Marking %s as uninstallable due to conflicts over %s"),
				affected_str.c_str(),
				ResolverInfo::toString (_capability).c_str());
	    PoolItem_Ref  issuer = other();
	    if (issuer) {
		msg += " ";
		// Translator: %s = name of package,patch
		// TranslatorExample: from abc
		msg += str::form (_("from %s"),
			ResolverInfo::toString (issuer).c_str());
	    }
	}
	break;

	//===================
	//-------------------

	default:
	    WAR << "Not an InfoMisc type: " << type() << endl;
//	    msg = "Unknown";
	break;
    }
    return msg;
}

//---------------------------------------------------------------------------

bool
ResolverInfoMisc::merge (ResolverInfo_Ptr info)
{
    bool res;
    ResolverInfoMisc_Ptr to_be_merged = dynamic_pointer_cast<ResolverInfoMisc>(info);

    res = ResolverInfo::merge(to_be_merged);
    if (!res) return res;

    if (type() == to_be_merged->type()
	&& affected() == to_be_merged->affected()
	&& _capability == to_be_merged->_capability) {

	return true;
    }

    return false;
}


ResolverInfo_Ptr
ResolverInfoMisc::copy (void) const
{
    ResolverInfoMisc_Ptr cpy = new ResolverInfoMisc(type(), affected(), priority(), _capability);

    ((ResolverInfoContainer_Ptr)cpy)->copy (this);
    cpy->_other_item = _other_item;
    cpy->_other_capability = _other_capability;
    cpy->_action = _action;
    cpy->_trigger = _trigger;
    
    return cpy;
}

//---------------------------------------------------------------------------

void
ResolverInfoMisc::addAction (const std::string & action_msg)
{
    _action = action_msg;
}


void
ResolverInfoMisc::addTrigger (const TriggerReason & trigger)
{
    _trigger = trigger;
}

void
ResolverInfoMisc::setOtherPoolItem (PoolItem_Ref other)
{
    _other_item = other;
}

void
ResolverInfoMisc::setOtherCapability (const Capability & capability)
{
    _other_capability = capability;
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

