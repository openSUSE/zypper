/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* Importance.cc
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

#include <zypp/solver/detail/Importance.h>

#include <zypp/solver/detail/debug.h>

///////////////////////////////////////////////////////////////////
namespace ZYPP {
//////////////////////////////////////////////////////////////////

using namespace std;

const Importance & Importance::Undefined= Importance("undefined");

//---------------------------------------------------------------------------
string
Importance::asString ( void ) const
{
    return toString (*this);
}


string
Importance::toString ( const Importance & importance )
{
    string res;

    switch (importance.importance()) {
	case IMPORTANCE_UNDEFINED:	res = "undefined"; break;
	case IMPORTANCE_INVALID:	res = "invalid"; break;
	case IMPORTANCE_NECESSARY:	res = "necessary"; break;
	case IMPORTANCE_URGENT:		res = "urgent"; break;
	case IMPORTANCE_SUGGESTED:	res = "suggested"; break;
	case IMPORTANCE_FEATURE:	res = "feature"; break;
	case IMPORTANCE_MINOR:		res = "minor"; break;
	default:
	    rc_debug (RC_DEBUG_LEVEL_WARNING, "invalid importance %d\n",  importance.importance());
	    res = "invalid";
    }
    return res;
}


ostream &
Importance::dumpOn( ostream & str ) const
{
    str << asString();
    return str;
}


ostream&
operator<<( ostream& os, const Importance& importance)
{
    return os << importance.asString();
}

//---------------------------------------------------------------------------

Importance::Importance(const char *importance_str)
{
    _importance = IMPORTANCE_INVALID;
    if (importance_str != NULL) {
	switch (*importance_str) {
	    case 'f':
		if (!strcmp (importance_str, "feature")) {
		    _importance = IMPORTANCE_FEATURE;
		}
	    break;
	    case 'm':
		if (!strcmp (importance_str, "minor")) {
		    _importance = IMPORTANCE_MINOR;
		}
	    break;
	    case 'n':
		if (!strcmp (importance_str, "necessary")) {
		    _importance = IMPORTANCE_NECESSARY;
		}
	    break;
	    case 's':
		if (!strcmp (importance_str, "suggested")) {
		    _importance = IMPORTANCE_SUGGESTED;
		}
	    break;
	    case 'u':
		if (!strcmp (importance_str, "urgent")) {
		    _importance = IMPORTANCE_URGENT;
		}
		else if (!strcmp (importance_str, "undefined")) {
		    _importance = IMPORTANCE_UNDEFINED;
		}
	    default:
	    break;
	}
    }
    if (_importance == IMPORTANCE_INVALID)
	rc_debug (RC_DEBUG_LEVEL_WARNING, "invalid importance '%s'\n", importance_str ? importance_str : "<null>");

}


Importance::~Importance()
{
}



///////////////////////////////////////////////////////////////////
}; // namespace ZYPP
///////////////////////////////////////////////////////////////////

