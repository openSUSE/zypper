/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* Package.cc
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

#include "zypp/Package.h"
#include "zypp/detail/PackageImpl.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/CapFactory.h"
#include "zypp/CapSet.h"

#include "zypp/solver/temporary/utils.h"
#include "zypp/solver/temporary/Package.h"
#include "zypp/solver/temporary/PackageUpdate.h"
#include "zypp/solver/temporary/MultiWorld.h"
#include "zypp/solver/temporary/XmlNode.h"

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

IMPL_PTR_TYPE(Package);

struct DepTable {
    CapSet requires;
    CapSet provides;
    CapSet conflicts;
    CapSet obsoletes;
    CapSet children;
    CapSet suggests;
    CapSet recommends;
};

//---------------------------------------------------------------------------

static Capability
parseXmlDep (XmlNode_constPtr node) {
    string tmp;
    string epoch,version,release,name = "";
    Arch arch = Arch_noarch;
    Rel relation;
    CapFactory  factory;

    if (!node->equals("dep")) {
        fprintf (stderr, "parseXmlDep bad node\n");
        abort();
    }

    name = node->getProp ("name");
    tmp = node->getProp ("op");
    if (!tmp.empty()) {
        relation = Rel(tmp);
        epoch = str::numstring(node->getIntValueDefault ("epoch", Edition::noepoch));
        version = node->getProp ("version");
        release = node->getProp ("release");
    }

    tmp = node->getProp ("arch");
    if (!tmp.empty()) {
        arch = Arch(node->getProp ("arch"));
    } else {
        arch =  Arch();
    }

    return  factory.parse ( ResTraits<zypp::Package>::kind,
                            name,
                            relation,
                            Edition (version, release, epoch));
}

static void
extract_dep_info (XmlNode_constPtr iter, struct DepTable & dep_table)
{
    if (iter->equals("requires")) {
	XmlNode_constPtr iter2;

	iter2 = iter->children();

	while (iter2) {
	    if (!iter2->isElement()) {
		iter2 = iter2->next();
		continue;
	    }

	    dep_table.requires.insert(parseXmlDep (iter2));
	    iter2 = iter2->next();
	}

    } else if (iter->equals("recommends")) {
	XmlNode_constPtr iter2;

	iter2 = iter->children();

	while (iter2) {
	    if (!iter2->isElement()) {
		iter2 = iter2->next();
		continue;
	    }

	    dep_table.recommends.insert (parseXmlDep (iter2));
	    iter2 = iter2->next();
	}

    } else if (iter->equals("suggests")) {
	XmlNode_constPtr iter2;

	iter2 = iter->children();

	while (iter2) {
	    if (!iter2->isElement()) {
		iter2 = iter2->next();
		continue;
	    }

	    dep_table.suggests.insert (parseXmlDep (iter2));
	    iter2 = iter2->next();
	}

    } else if (iter->equals("conflicts")) {
	XmlNode_Ptr iter2;
	bool all_are_obs = false, this_is_obs = false;
	string obs;

	iter2 = iter->children();

	obs = iter->getProp ("obsoletes");
	if (!obs.empty()) {
	    all_are_obs = true;
	}

	while (iter2) {

	    if (!iter2->isElement()) {
		iter2 = iter2->next();
		continue;
	    }

	    Capability dep = parseXmlDep(iter2);

	    if (! all_are_obs) {
		this_is_obs = false;
		obs = iter2->getProp ("obsoletes");
		if (!obs.empty()) {
		    this_is_obs = true;
		}
	    }

	    if (all_are_obs || this_is_obs) {
		dep_table.obsoletes.insert (dep);
	    } else {
		dep_table.conflicts.insert (dep);
	    }

	    iter2 = iter2->next();
	}

    } else if (iter->equals("obsoletes")) {
	XmlNode_constPtr iter2;

	iter2 = iter->children();

	while (iter2) {
	    if (!iter2->isElement()) {
		iter2 = iter2->next();
		continue;
	    }

	    dep_table.obsoletes.insert (parseXmlDep (iter2));
	    iter2 = iter2->next();
	}

    } else if (iter->equals("provides")) {
	XmlNode_constPtr iter2;

	iter2 = iter->children();

	while (iter2) {
	    if (!iter2->isElement()) {
		iter2 = iter2->next();
		continue;
	    }

	    dep_table.provides.insert (parseXmlDep (iter2));
	    iter2 = iter2->next();
	}

    } else if (iter->equals("children")) {
	XmlNode_constPtr iter2;

	iter2 = iter->children();

	while (iter2) {
	    if (!iter2->isElement()) {
		iter2 = iter2->next();
		continue;
	    }

	    dep_table.children.insert (parseXmlDep (iter2));
	    iter2 = iter2->next();
	}
    }
}

//---------------------------------------------------------------------------


string
Package::asString ( bool full ) const
{
    return toString (*this, full);
}


string
Package::toString ( const PackageUpdateList & l, bool full )
{
    string ret ("[");
    for (PackageUpdateList::const_iterator i = l.begin(); i != l.end(); i++) {
	if (i != l.begin()) ret += ", ";
	ret += (*i)->asString(full);
    }
    return ret + "]";
}

string
Package::toString ( const Package & package, bool full )
{
    string ret;
    ret += ResItem::toString(package, full);
    if (full) {
//	if (package._section != NULL) ret += (string ("<section '") + package._section->asString() + "'/>");
//	if (!package._pretty_name.empty()) ret += (string ("<pretty_name '") + package._pretty_name + "'/>");
//	if (!package._summary.empty()) ret += (string ("<summary '") + package._summary + "'/>");
//	if (!package._description.empty()) ret += (string ("<description '") + package._description + "'/>");
	ret += (string ("<history '") + toString(package._history) + "'/>");
    }
    return ret;
}


ostream &
Package::dumpOn( ostream & str ) const
{
    str << asString();
    return str;
}


ostream&
operator<<( ostream& os, const Package& package)
{
    return os << package.asString();
}

//---------------------------------------------------------------------------

Package::Package (Channel_constPtr channel, const Resolvable::Kind & kind)
    : ResItem (kind, "")
    , _pretty_name ("")
    , _summary ("")
    , _description ("")
    , _package_filename ("")
    , _signature_filename ("")
    , _install_only (false)
    , _package_set (false)
    , _id ("")
{
    setChannel (channel);
}


Package::Package (Channel_constPtr channel,
		  const Resolvable::Kind & kind,
                  const string & name,
                  const Edition & edition,
                  const Arch arch)

    : ResItem (kind, name, edition, arch)
    , _pretty_name ("")
    , _summary ("")
    , _description ("")
    , _package_filename ("")
    , _signature_filename ("")
    , _install_only (false)
    , _package_set (false)
    , _id ("")
{

    setChannel (channel);
}


Package::Package (XmlNode_constPtr node, Channel_constPtr channel, const Resolvable::Kind & kind)
    : ResItem (kind, "")
    , _pretty_name ("")
    , _summary ("")
    , _description ("")
    , _package_filename ("")
    , _signature_filename ("")
    , _install_only (false)
    , _package_set (false)
    , _id ("")
{
    string name = "";
    int epoch = Edition::noepoch;
    string version = "";
    string release = "";
    Arch arch = Arch_noarch;

    if (!node->equals("package")) {
        fprintf (stderr, "Package::Package() not a package node\n");
        exit (1);
    }

    struct DepTable dep_table;

    setChannel (channel);

    XmlNode_constPtr iter = node->children();

    while (iter) {
        bool extracted_deps = false;

        if (iter->equals("name")) {	  		name = iter->getContent();
        } else if (iter->equals("epoch")) {		epoch = atoi (iter->getContent().c_str());
        } else if (iter->equals("version")) {		version = iter->getContent();
        } else if (iter->equals("release")) {		release = iter->getContent();
        } else if (iter->equals("summary")) {		_summary = iter->getContent();
        } else if (iter->equals("description")) {	_description = iter->getContent();
        } else if (iter->equals("arch")) {		arch = Arch(iter->getContent());
        } else if (iter->equals("filesize")) {
            setFileSize (iter->getIntValueDefault("filesize", 0));
        } else if (iter->equals("installedsize")) {
            setInstalledSize (iter->getIntValueDefault("installedsize", 0));
        } else if (iter->equals("install_only")) {	_install_only = true;
        } else if (iter->equals("package_set")) {	_package_set = true;
        } else if (iter->equals("history")) {
            XmlNode_constPtr iter2;

            iter2 = iter->children();

            while (iter2) {
                if (!iter2->isElement()) {
                    iter2 = iter2->next();
                    continue;
                }

                PackageUpdate_Ptr update = new PackageUpdate (iter2, this);
                addUpdate (update);

                iter2 = iter2->next();
            }
        } else if (iter->equals("deps")) {
            XmlNode_constPtr iter2;

            for (iter2 = iter->children(); iter2; iter2 = iter2->next()) {
                if (!iter2->isElement())
                    continue;

                extract_dep_info (iter2, dep_table);
            }

            extracted_deps = true;
        }
        else {
            if (!extracted_deps)
                extract_dep_info (iter, dep_table);
            else {
                /* FIXME: Bitch to the user here? */
            }
        }

        iter = iter->next();
    }

    if (!dep_table.children.empty()) {
        // children are used in package sets
        // treat them as normal requires
        //
#warning Children are handled as requires
        CapSet::const_iterator iter;
        for (iter = dep_table.children.begin(); iter != dep_table.children.end(); iter++)
        {
            dep_table.requires.insert (*iter);
        }
    }


    // check if we're already listed in the provides
    // if not, provide ourself
    CapFactory  factory;
    Capability selfdep = factory.parse ( kind,
                                       name,
                                       Rel::EQ,
				       Edition( version, release, zypp::str::numstring(epoch)));


    CapSet::const_iterator piter;
    for (piter = dep_table.provides.begin(); piter != dep_table.provides.end(); piter++) {
        if ((*piter) == selfdep)
        {
            break;
        }
    }
    if (piter == dep_table.provides.end()) {			// no self provide found, construct one
        _DBG("RC_SPEW") << "Adding self-provide [" << selfdep.asString() << "]" << endl;
        dep_table.provides.insert (selfdep);
    }

    Dependencies deps;
    deps.requires          = dep_table.requires;
    deps.provides          = dep_table.provides;
    deps.conflicts         = dep_table.conflicts;
    deps.obsoletes         = dep_table.obsoletes;
    deps.suggests          = dep_table.suggests;
    deps.recommends        = dep_table.recommends;
    deprecatedSetDependencies (deps);

    if (!_history.empty()) {

        /* If possible, we grab the version info from the most
           recent update. */

        PackageUpdate_Ptr update = _history.front();

        epoch = update->package()->epoch();
        version = update->package()->version();
        release = update->package()->release();

    }
#if 0 //Is this really needed ?
    else {

        /* Otherwise, try to find where the package provides itself,
           and use that version info. */

        if (!provides().empty())
            for (CapSet::const_iterator iter = provides().begin(); iter != provides().end(); iter++) {
                if ((*iter)->relation() == Rel::EQ &&
                    ((*iter)->name() == name))
                {
                    epoch = (*iter)->epoch();
                    version = (*iter)->version();
                    release = (*iter)->release();
                    break;
                }
            }
    }
#endif

    Edition     _edition( version, release, zypp::str::numstring(epoch) );
    shared_ptr<zypp::detail::PackageImpl> pkgImpl;
    zypp::Package::Ptr pkg( zypp::detail::makeResolvableAndImpl( NVRAD(name, _edition, arch),
                                                                 pkgImpl ) );
    _resObject = pkg;
}

Package::~Package()
{
}

//---------------------------------------------------------------------------


void
Package::addUpdate (PackageUpdate_Ptr update)
{
    if (update == NULL) return;

    assert (update->package() == NULL
             || update->package() == const_cast<const Package *>(this));

    update->setPackage(this);

    if (_history.empty()) {
	_history.push_back (update);
    } else {
#warning addUpdate incomplete
#if 1
	for (PackageUpdateList::iterator iter = _history.begin(); iter != _history.end(); iter++) {
	    int result = Spec::compare ((Spec_Ptr)update, (Spec_Ptr)(*iter));

	    if (result > 0 || (result == 0 && update->parent() != NULL)) {
		_history.insert (iter, update);					// = g_slist_insert_before (package->history, l, update);
		break;
	    } else if (iter == _history.end() ||					// FIXME list.last() ?
		       (result == 0 && update->parent() == NULL)) {
		_history.insert (++iter, update);					// = g_slist_insert_before (package->history, l->next, update);
		break;
	    }
	}
#endif
    }
}


PackageUpdate_Ptr
Package::getLatestUpdate (void) const
{
    World_Ptr world;

    if (_history.empty()) {
	return NULL;
    }

    PackageUpdate_Ptr latest = _history.back();
    /* if the absolute latest is not a patch, just return that */
    if (latest->parent() == NULL) {
	return latest;
    }

    world = World::globalWorld();

    for (PackageUpdateList::const_iterator l = _history.begin(); l != _history.end(); l++) {
	PackageUpdate_Ptr update = *l;
	ResItem_constPtr installed;

	if (!update->equals (latest)) {
	    return NULL;
	}

	/* found a non-patch package equal to the latest, so use that */
	if (update->parent() == NULL) {
	    return update;
	}

	/* see if the required parent for this patch is installed */

	installed = world->findInstalledResItem (update->parent());

	if (installed != NULL &&
	    installed->equals(update->parent()))
	    return update;
    }

    /* no suitable update found */
    return NULL;
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

