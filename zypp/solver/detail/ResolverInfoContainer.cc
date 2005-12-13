/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ResolverInfoContainer.cc
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

#include <zypp/solver/detail/ResolverInfoContainer.h>

///////////////////////////////////////////////////////////////////
namespace ZYPP {
//////////////////////////////////////////////////////////////////

using namespace std;

IMPL_DERIVED_POINTER(ResolverInfoContainer, ResolverInfo);

//---------------------------------------------------------------------------


string
ResolverInfoContainer::asString ( void ) const
{
    return toString (*this);
}


string
ResolverInfoContainer::toString ( const ResolverInfoContainer & container )
{
    string res = "<resolverinfocontainer '";

    res += ResolverInfo::toString (container);
    for (CResolvableList::const_iterator iter = container._resolvable_list.begin(); iter != container._resolvable_list.end(); iter++) {
	if (iter != container._resolvable_list.begin()) res += ", ";
	res += ((constSpecPtr)(*iter))->asString();
    }
    res += "'>";

    return res;
}


ostream &
ResolverInfoContainer::dumpOn( ostream & str ) const
{
    str << asString();
    return str;
}


ostream&
operator<<( ostream& os, const ResolverInfoContainer & container)
{
    return os << container.asString();
}

//---------------------------------------------------------------------------

ResolverInfoContainer::ResolverInfoContainer (ResolverInfoType type, constResolvablePtr resolvable, int priority, constResolvablePtr child)
    : ResolverInfo (type, resolvable, priority)
{
    if (child != NULL)
	_resolvable_list.push_back (child);
}


ResolverInfoContainer::~ResolverInfoContainer ()
{
}

//---------------------------------------------------------------------------

bool
ResolverInfoContainer::merge (ResolverInfoContainerPtr to_be_merged)
{
    bool res;

    res = ((ResolverInfoPtr)this)->merge ((ResolverInfoPtr)to_be_merged);
    if (!res) return res;

    typedef std::map<constResolvablePtr, bool> SeenTable;
    SeenTable seen_packages;

    for (CResolvableList::const_iterator iter = _resolvable_list.begin(); iter != _resolvable_list.end(); iter++) {
	seen_packages[*iter] = true;
    }

    CResolvableList rl = to_be_merged->resolvables();
    for (CResolvableList::const_iterator iter = rl.begin(); iter != rl.end(); iter++) {
	SeenTable::const_iterator pos = seen_packages.find(*iter);
	if (pos == seen_packages.end()) {
	    _resolvable_list.push_front (*iter);
	    seen_packages[*iter] = true;
	}
    }

    return true;
}


void
ResolverInfoContainer::copy (constResolverInfoContainerPtr from)
{
    ((ResolverInfoPtr)this)->copy(from);

    for (CResolvableList::const_iterator iter = from->_resolvable_list.begin(); iter != from->_resolvable_list.end(); iter++) {
	_resolvable_list.push_back (*iter);
    }
}


ResolverInfoPtr
ResolverInfoContainer::copy (void) const
{
    ResolverInfoContainerPtr cpy = new ResolverInfoContainer(type(), resolvable(), priority());

    cpy->copy (this);

    return cpy;
}

//---------------------------------------------------------------------------

string
ResolverInfoContainer::resolvablesToString (bool names_only) const
{
    string res;

    if (_resolvable_list.empty())
	return res;

    res += " [";
    for (CResolvableList::const_iterator iter = _resolvable_list.begin(); iter != _resolvable_list.end(); iter++) {
	if (iter != _resolvable_list.begin())
	    res += ", ";

	res += (names_only ? (*iter)->name() : ((constSpecPtr)(*iter))->asString());
    }
    res += "]";

    return res;
}


bool
ResolverInfoContainer::mentions (constResolvablePtr resolvable) const
{
    if (isAbout(resolvable))
	return true;

    // Search resolvable_list for any mention of the resolvable.

    for (CResolvableList::const_iterator iter = _resolvable_list.begin(); iter != _resolvable_list.end(); iter++) {
	if ((*iter)->name() == resolvable->name()) {
	    return true;
	}
    }
    
    return false;
}


void
ResolverInfoContainer::addRelatedResolvable (constResolvablePtr resolvable)
{
    if (!mentions(resolvable)) {
	_resolvable_list.push_front (resolvable);
    }
}


void
ResolverInfoContainer::addRelatedResolvableList (const CResolvableList & resolvables)
{
    for (CResolvableList::const_iterator iter = resolvables.begin(); iter != resolvables.end(); iter++) {
	_resolvable_list.push_front (*iter);
    }
}


///////////////////////////////////////////////////////////////////
}; // namespace ZYPP
///////////////////////////////////////////////////////////////////

