/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ResolverInfo.cc
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

#include <zypp/solver/detail/ResolverInfo.h>

///////////////////////////////////////////////////////////////////
namespace ZYPP {
//////////////////////////////////////////////////////////////////

using namespace std;

IMPL_BASE_POINTER(ResolverInfo);

//---------------------------------------------------------------------------

static struct {
    ResolverInfoType type;
    const char        *str;
} type_str_table[] = {
    { RESOLVER_INFO_TYPE_NEEDED_BY,      "needed_by" },
    { RESOLVER_INFO_TYPE_CONFLICTS_WITH, "conflicts_with" },
    { RESOLVER_INFO_TYPE_OBSOLETES,      "obsoletes" },
    { RESOLVER_INFO_TYPE_DEPENDS_ON,     "depended_on" },
    { RESOLVER_INFO_TYPE_CHILD_OF,       "child_of" },
    { RESOLVER_INFO_TYPE_MISSING_REQ,    "missing_req" },
    { RESOLVER_INFO_TYPE_MISC,           "misc" },
    { RESOLVER_INFO_TYPE_INVALID,        "invalid" },
    { RESOLVER_INFO_TYPE_INVALID,        NULL }
};

static const char *
info_type_to_string (ResolverInfoType type)
{
    int i;

    for (i = 0; type_str_table[i].str != NULL; ++i) {
	if (type == type_str_table[i].type)
	    return type_str_table[i].str;
    }

    return NULL;
}


ResolverInfoType
resolver_info_type_from_string (const char *str)
{
    int i;

    if (str == NULL) return RESOLVER_INFO_TYPE_INVALID;

    for (i = 0; type_str_table[i].str != NULL; ++i) {
	if (strcasecmp (str, type_str_table[i].str) == 0)
	    return type_str_table[i].type;
    }

    return RESOLVER_INFO_TYPE_INVALID;
}

//---------------------------------------------------------------------------

string
ResolverInfo::asString ( void ) const
{
    return toString (*this);
}


string
ResolverInfo::toString ( const ResolverInfo & resolverinfo, bool full )
{
    string res;

    if (full) {
	res += "<";
	res += info_type_to_string (resolverinfo._type);
	res += "> ";
    }
    if (resolverinfo._resItem != NULL) {
	res += resolverinfo._resItem->asString();
	res += ": ";
    }

    if (resolverinfo._error) res += " Error!";
    if (resolverinfo._important) res += " Important!";

    return res;
}


ostream &
ResolverInfo::dumpOn( ostream & str ) const
{
    str << asString();
    return str;
}


ostream&
operator<<( ostream& os, const ResolverInfo & resolver)
{
    return os << resolver.asString();
}

//---------------------------------------------------------------------------

ResolverInfo::ResolverInfo (ResolverInfoType type, constResItemPtr resItem, int priority)
    : _type (type)
    , _resItem (resItem)
    , _priority (priority)
    , _error (false)
    , _important (false)
{
}


ResolverInfo::~ResolverInfo()
{
}

//---------------------------------------------------------------------------

bool
ResolverInfo::merge (ResolverInfoPtr to_be_merged)
{
    if (to_be_merged == NULL) return false;

    if (_type != to_be_merged->_type
	|| _resItem != to_be_merged->_resItem) {
	return false;
    }

    return true;
}

void
ResolverInfo::copy (constResolverInfoPtr from)
{
    _error = from->_error;
    _important = from->_important;
}


ResolverInfoPtr
ResolverInfo::copy (void) const
{
    ResolverInfoPtr cpy = new ResolverInfo(_type, _resItem, _priority);

    cpy->copy (this);
 
    return cpy;
}


//---------------------------------------------------------------------------

bool
ResolverInfo::isAbout (constResItemPtr resItem) const
{
    if (_resItem == NULL)
	return false;

    return _resItem->name() == resItem->name();
}

///////////////////////////////////////////////////////////////////
}; // namespace ZYPP
///////////////////////////////////////////////////////////////////

