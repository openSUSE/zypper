/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
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
#include <iostream>
#include "zypp/solver/detail/Importance.h"
#include "zypp/base/Logger.h"

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

const Importance Importance::Undefined (IMPORTANCE_UNDEFINED);
const Importance Importance::Invalid (IMPORTANCE_INVALID);
const Importance Importance::Necessary (IMPORTANCE_NECESSARY);
const Importance Importance::Urgent (IMPORTANCE_URGENT);
const Importance Importance::Suggested (IMPORTANCE_SUGGESTED);
const Importance Importance::Feature (IMPORTANCE_FEATURE);
const Importance Importance::Minor (IMPORTANCE_MINOR);

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
	    WAR << "invalid importance "<< importance.importance() << endl;
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

const Importance
Importance::parse(const string & str)
{
    Importance importance = Invalid;
    if (str == "feature") {
	importance = Feature;
    }
    else if (str == "minor") {
	importance = Minor;
    }
    else if (str == "necessary") {
	importance = Necessary;
    }
    if (str == "suggested") {
	importance = Suggested;
    }
    if (str == "urgent") {
	importance = Urgent;
    }
    else if (str == "undefined") {
	importance = Undefined;
    }

    if (importance == Invalid)
	WAR << "invalid importance '" << str << "'" << endl;

    return importance;
}


Importance::Importance(importance_t importance)
    : _importance (importance)
{
}

Importance::~Importance()
{
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


