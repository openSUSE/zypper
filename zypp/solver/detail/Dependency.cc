/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* Dependency.cc
 *
 * Copyright (C) 2000-2002 Ximian, Inc.
 * Copyright (C) 2005 SUSE Linux Products GmbH
 *
 * Definition of 'dependency'
 *  contains name-op-spec
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

#include <zypp/solver/detail/Dependency.h>
#include <zypp/solver/detail/OrDependency.h>
#include <zypp/solver/detail/Version.h>

///////////////////////////////////////////////////////////////////
namespace zypp {
//////////////////////////////////////////////////////////////////

using namespace std;

IMPL_DERIVED_POINTER(Dependency,Spec);

//---------------------------------------------------------------------------

#define RELATION_INVALID -1
#define RELATION_ANY 0
#define RELATION_EQUAL (1 << 0)
#define RELATION_LESS (1 << 1)
#define RELATION_GREATER (1 << 2)
#define RELATION_NONE (1 << 3)

const Relation & Relation::Invalid = Relation (RELATION_INVALID);
const Relation & Relation::Any = Relation (RELATION_ANY);
const Relation & Relation::Equal = Relation (RELATION_EQUAL);
const Relation & Relation::NotEqual = Relation (RELATION_LESS|RELATION_GREATER);
const Relation & Relation::Less = Relation (RELATION_LESS);
const Relation & Relation::LessEqual = Relation (RELATION_LESS|RELATION_EQUAL);
const Relation & Relation::Greater = Relation (RELATION_GREATER);
const Relation & Relation::GreaterEqual = Relation (RELATION_GREATER|RELATION_EQUAL);
const Relation & Relation::None = Relation (RELATION_NONE);


const Relation &
Relation::parse(const char *relation)
{
    if (!strcmp (relation, "(any)"))
	return Any;
    else if (!strcmp (relation, "=") || !strcmp (relation, "eq"))
	return Equal;
    else if (!strcmp (relation, "<") || !strcmp(relation, "lt") || !strcmp(relation, "&lt;"))
	return Less;
    else if (!strcmp (relation, "<=") || !strcmp(relation, "lte") || !strcmp(relation, "&lt;="))
	return LessEqual;
    else if (!strcmp (relation, ">") || !strcmp(relation, "gt") || !strcmp(relation, "&gt;"))
	return Greater;
    else if (!strcmp (relation, ">=") || !strcmp(relation, "gte") || !strcmp(relation, "&gt;="))
	return GreaterEqual;
    else if (!strcmp (relation, "!=") || !strcmp(relation, "ne"))
	return NotEqual;
    else if (!strcmp (relation, "!!") || !strcmp(relation, "none"))
	return None;
    else
	return Invalid;
}


bool
Relation::isEqual () const
{
    return _op == RELATION_EQUAL;
}


string
Relation::asString ( void ) const
{
    return toString (*this);
}


string
Relation::toString ( const Relation & relation )
{
    string res;

    switch (relation.op()) {
	case RELATION_INVALID: res = "(invalid)";
	break;
	case RELATION_ANY: res = "";
	break;
	case RELATION_EQUAL: res = "==";
	break;
	case RELATION_LESS|RELATION_GREATER: res = "!=";
	break;
	case RELATION_LESS: res = "<";
	break;
	case RELATION_LESS|RELATION_EQUAL: res = "<=";
	break;
	case RELATION_GREATER: res = ">";
	break;
	case RELATION_GREATER|RELATION_EQUAL: res = ">=";
	break;
	case RELATION_NONE: res = "!!";
	break;
	default:
	    res = "??";
	    res += stringutil::numstring (relation.op());
	    res += "??";
	break;
    }
    return res;
}


string
Relation::toWord ( const Relation & relation )
{
    string res;

    switch (relation.op()) {
	case RELATION_INVALID: res = "(invalid)";
	break;
	case RELATION_ANY: res = "(any)";
	break;
	case RELATION_EQUAL: res = "equal to";
	break;
	case RELATION_LESS|RELATION_GREATER: res = "not equal to";
	break;
	case RELATION_LESS: res = "less than";
	break;
	case RELATION_LESS|RELATION_EQUAL: res = "less than or equal to";
	break;
	case RELATION_GREATER: res = "greater than";
	break;
	case RELATION_GREATER|RELATION_EQUAL: res = "greater than or equal to";
	break;
	case RELATION_NONE: res = "not installed";
	break;
	default:
	    res = "??";
	    res += stringutil::numstring (relation.op());
	    res += "??";
	break;
    }
    return res;
}


ostream &
Relation::dumpOn( ostream & str ) const
{
    str << asString();
    return str;
}


ostream&
operator<<( ostream & os, const Relation & relation)
{
    return os << relation.asString();
}

//---------------------------------------------------------------------------

string
Dependency::asString ( void ) const
{
    return toString (*this);
}


string
Dependency::toString ( const Dependency & dependency )
{
    string res;

    res += dependency.name();
    if (dependency.relation() != Relation::Any) {
	res += " ";
	res += dependency.relation().asString();
	res += " ";

	res += dependency.edition()->asString();
    }
    if (dependency.orDep()) res += " [OR]";
    if (dependency.preDep()) res += " [PRE]";

    return res;
}


string
Dependency::toString (const CDependencyList & dl)
{
    string res("[");
    for (CDependencyList::const_iterator iter = dl.begin(); iter != dl.end(); iter++) {
	if (iter != dl.begin()) res += ", ";
	res += (*iter)->asString();
    }
    return res + "]";
}




ostream &
Dependency::dumpOn( ostream & str ) const
{
    str << asString();
    return str;
}


ostream&
operator<<( ostream& os, const Dependency& dependency)
{
    return os << dependency.asString();
}

//---------------------------------------------------------------------------

Dependency::Dependency (const string & name, const Relation & relation, const Kind & kind,
	constChannelPtr channel,
	int epoch, const string & version, const string & release, const Arch * arch,
	bool or_dep, bool pre_dep)
    : Spec (kind, name, epoch, version, release, arch)
    , _relation (relation)
    , _channel (channel)
    , _or_dep (or_dep)
    , _pre_dep (pre_dep)
{
}


Dependency::Dependency (const string & name, const Relation & relation, const Kind & kind,
	constChannelPtr channel, constEditionPtr edition, bool or_dep, bool pre_dep)
    : Spec (kind, name, edition)
    , _relation (relation)
    , _channel (channel)
    , _or_dep (or_dep)
    , _pre_dep (pre_dep)
{
}


Dependency::Dependency (OrDependencyPtr or_dep)
    : Spec (Kind::Package, or_dep->name())
    , _relation (Relation::Any)
    , _channel (NULL)
    , _or_dep (false)
    , _pre_dep (true)
{
    or_dep->addCreatedProvide (this);
}


Dependency::Dependency (constXmlNodePtr node)
    : Spec (Kind::Package, "")
    , _relation (Relation::Any)
    , _channel (new Channel(CHANNEL_TYPE_ANY))
    , _or_dep (false)
    , _pre_dep (false)
{
    const char *tmp;

    if (!node->equals("dep")) {
	fprintf (stderr, "Dependency::Dependency bad node\n");
	abort();
    }

    setName(node->getProp ("name"));
    tmp = node->getProp ("op", NULL);
    if (tmp) {
	_relation = Relation::parse(tmp);
	setEpoch (node->getIntValueDefault ("epoch", -1));
	setVersion (node->getProp ("version"));
	setRelease (node->getProp ("release"));
    }

    tmp = node->getProp ("arch", NULL);
    if (tmp) {
	setArch (Arch::create (node->getProp ("arch")));
    } else {
	setArch (Arch::Unknown);
    }
#if 0
    tmp = node->getProp ("kind", NULL);
    if (tmp) {
	setKind (Kind (node->getProp ("kind")));
    }
#endif
    /* FIXME: should get channel from XML */
    /* FIXME: should get arch from XML */
}


Dependency::~Dependency()
{
}

//---------------------------------------------------------------------------

DependencyPtr parseXml (constXmlNodePtr node)
{
    if (node->equals("dep")) {
	return new Dependency (node);
    } else if (node->equals("or")) {
	CDependencyList or_dep_list;

	node = node->children();

	while (node) {
	    if (node->isElement()) {
		or_dep_list.push_back (new Dependency (node));
	    }

	    node = node->next();
	}

	OrDependencyPtr or_dep = OrDependency::fromDependencyList(or_dep_list);
	return new Dependency (or_dep);
    }

    fprintf (stderr, "Unhandled dependency [%s]\n", node->name());

    return NULL;
}


bool
Dependency::verifyRelation (constDependencyPtr prov) const
{
    int compare_ret = 0;
if (getenv ("SPEW_DEP")) fprintf (stderr, "Dependency::verifyRelation(dep: %s, prov: %s)", asString().c_str(), prov->asString().c_str());
    /* No dependency can be met by a different token name */
    if (name() != prov->name()) {
if (getenv ("SPEW_DEP")) fprintf (stderr, "-> wrong name\n");
	return false;
    }

    /* No dependency can be met by a different type */
    if (kind() != prov->kind()) {
if (getenv ("SPEW_DEP")) fprintf (stderr, "-> wrong kind(dep: %s, prov: %s)\n", kind().asString().c_str(), prov->kind().asString().c_str());
	return false;
    }

    /* WARNING: RC_RELATION_NONE is NOT handled */

    /* No specific version in the req, so return */
    if (_relation == Relation::Any) {
if (getenv ("SPEW_DEP")) fprintf (stderr, " (any) -> true\n");
	return true;
    }

    /* No specific version in the prov.  In RPM this means it will satisfy
     * any version, but debian it will not satisfy a versioned dep */
    if (prov->relation() == Relation::Any) {
	if (GVersion.hasProperty (VERSION_PROP_PROVIDE_ANY)) {
if (getenv ("SPEW_DEP")) fprintf (stderr, " provides (any) matches GVersion -> true\n");
	    return true;
	}
	else {
if (getenv ("SPEW_DEP")) fprintf (stderr, " provides (any) does not match GVersion -> false\n");
	    return false;
	}
    }

    if (!channel()->equals (prov->channel()))
    {
if (getenv ("SPEW_DEP")) fprintf (stderr, " wrong channel -> false\n");
	return false;
    }

    SpecPtr newdepspec;
    SpecPtr newprovspec;

    if (epoch() >= 0 && prov->epoch() >= 0) {
	/* HACK: This sucks, but I don't know a better way to compare elements one at a time */
	newdepspec = new Spec(kind(), "", epoch());
	newprovspec = new Spec(prov->kind(), "", prov->epoch());
	compare_ret = GVersion.compare (newprovspec, newdepspec);
    } else if (prov->epoch() > 0 ) {
	if (GVersion.hasProperty (VERSION_PROP_IGNORE_ABSENT_EPOCHS)) {
	    compare_ret = 0;
	}
	else {
	    compare_ret = 1;
	}
    } else if (epoch() > 0 ) {
	compare_ret = -1;
    }
if (getenv ("SPEW_DEP")) fprintf (stderr, "epoch(%d), prov->epoch(%d) -> compare_ret %d\n", epoch(), prov->epoch(), compare_ret);
    if (compare_ret == 0) {
	if (GVersion.hasProperty (VERSION_PROP_ALWAYS_VERIFY_RELEASE)
	    || (!(release().empty() || prov->release().empty()))) {
	    newdepspec = new Spec(kind(), "", -1, version(), release());
	    newprovspec = new Spec(prov->kind(), "", -1, prov->version(), prov->release());
	} else {
	    newdepspec = new Spec(kind(), "", -1, version());
	    newprovspec = new Spec(prov->kind(), "", -1, prov->version());
	}
	compare_ret = GVersion.compare (newprovspec, newdepspec);
    }
if (getenv ("SPEW_DEP")) fprintf (stderr, " (compare result -> %d)", compare_ret);

    if (compare_ret < 0
	&& ((prov->relation().op() & Relation::Greater.op())
	    || (_relation.op() & Relation::Less.op())))
    {
if (getenv ("SPEW_DEP")) fprintf (stderr, " -> true\n");
	return true;
    } else if (compare_ret > 0 
		&& ((prov->relation().op() & Relation::Less.op())
		    || (_relation.op() & Relation::Greater.op())))
    {
if (getenv ("SPEW_DEP")) fprintf (stderr, " -> true\n");
	return true;
    } else if (compare_ret == 0
	       && (((prov->relation().op() & Relation::Equal.op()) && (_relation.op() & Relation::Equal.op()))
		   || ((prov->relation().op() & Relation::Less.op()) && (_relation.op() & Relation::Less.op()))
		   || ((prov->relation().op() & Relation::Greater.op()) && (_relation.op() & Relation::Greater.op()))))
    {
if (getenv ("SPEW_DEP")) fprintf (stderr, " -> true\n");
	return true;
    }
    
if (getenv ("SPEW_DEP")) fprintf (stderr, " -> false\n");
    return false;
}

//---------------------------------------------------------------------------

#if 0
xmlNode *
rc_resItem_dep_or_slist_to_xml_node (RCResItemDepSList *dep)
{
    xmlNode *or_node;
    const RCResItemDepSList *dep_iter;

    or_node = xmlNewNode (NULL, "or");

    dep_iter = dep;
    while (dep_iter) {
	RCResItemDep *dep_item = (RCResItemDep *)(dep_iter->data);
	xmlAddChild (or_node, rc_resItem_dep_to_xml_node (dep_item));
	dep_iter = dep_iter->next;
    }

    return or_node;
} /* rc_resItem_dep_or_slist_to_xml_node */

xmlNode *
rc_resItem_dep_to_xml_node (RCResItemDep *dep_item)
{
    RCResItemSpec *spec = (RCResItemSpec *) dep_item;
    xmlNode *dep_node;

    if (rc_resItem_dep_is_or (dep_item)) {
	RCResItemDepSList *dep_or_slist;
	dep_or_slist = rc_dep_string_to_or_dep_slist
	    (rc_resItem_spec_get_name (spec));
	dep_node = rc_resItem_dep_or_slist_to_xml_node (dep_or_slist);
	rc_resItem_dep_slist_free (dep_or_slist);
	return dep_node;
    }

    dep_node = xmlNewNode (NULL, "dep");

    xmlSetProp (dep_node, "name", rc_resItem_spec_get_name (spec));

    if (rc_resItem_dep_get_relation (dep_item) != Relation::Any) {
	xmlSetProp (dep_node, "op",
		    rc_resItem_relation_to_string (
			rc_resItem_dep_get_relation (dep_item), false));

	if (rc_resItem_spec_has_epoch (spec)) {
	    gchar *tmp;

	    tmp = g_strdup_printf ("%d", rc_resItem_spec_get_epoch (spec));
	    xmlSetProp (dep_node, "epoch", tmp);
	    g_free (tmp);
	}

	if (spec->version) {
	    xmlSetProp (dep_node, "version", spec->version);
	}

	if (spec->release) {
	    xmlSetProp (dep_node, "release", spec->release);
	}
    }

    return (dep_node);
} /* rc_resItem_dep_to_xml_node */

#endif

///////////////////////////////////////////////////////////////////
}; // namespace zypp
///////////////////////////////////////////////////////////////////

