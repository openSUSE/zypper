/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ResolvableAndDependency.cc
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

#include <zypp/solver/detail/ResolvableAndDependency.h>
#include <zypp/solver/detail/debug.h>

///////////////////////////////////////////////////////////////////
namespace ZYPP {
//////////////////////////////////////////////////////////////////

using namespace std;

IMPL_BASE_POINTER(ResolvableAndDependency);

//---------------------------------------------------------------------------

ResolvableAndDependency::ResolvableAndDependency (constResolvablePtr resolvable, constDependencyPtr dependency)
    : _resolvable(resolvable)
    , _dependency(dependency)
{
}

//---------------------------------------------------------------------------

string
ResolvableAndDependency::asString (bool full) const
{
    return toString (*this, full);
}


string
ResolvableAndDependency::toString ( const ResolvableAndDependency & r_and_d, bool full )
{
    string res ("{");
    res += r_and_d._resolvable->asString(full);
    res += ", ";
    res += r_and_d._dependency->asString();
    res += "}";
    return res;
}


ostream &
ResolvableAndDependency::dumpOn (ostream & str) const
{
    str << asString();
    return str;
}


ostream &
operator<< (ostream & os, const ResolvableAndDependency & r_and_d)
{
    return os << r_and_d.asString();
}

//---------------------------------------------------------------------------

/* This function also checks channels in addition to just dep relations */
/* FIXME: rc_resolvable_dep_verify_relation already checks the channel */

bool
ResolvableAndDependency::verifyRelation (constDependencyPtr dep) const
{
#if PHI
    // don't check the channel, thereby honoring conflicts from installed resolvables to to-be-installed resolvables
    return dep->verifyRelation (_dependency);
#else
    if (!dep->verifyRelation (_dependency)) {
	return false;
    }
    if (getenv ("SPEW_DEP")) fprintf (stderr, "ResolvableAndDependency::verifyRelation _resolvable->channel() %s, dep->channel() %s\n", _resolvable->channel()->asString().c_str(), dep->channel()->asString().c_str());
    return _resolvable->channel()->equals (dep->channel());
#endif
}

///////////////////////////////////////////////////////////////////
}; // namespace ZYPP
///////////////////////////////////////////////////////////////////

