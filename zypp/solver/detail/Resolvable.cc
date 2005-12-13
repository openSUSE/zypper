/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* Resolvable.cc
 * Copyright (C) 2000-2002 Ximian, Inc.
 * Copyright (C) 2005 SUSE Linux Products GmbH
 *
 * Definition of 'resolvable'
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

#include <zypp/solver/detail/Resolvable.h>

///////////////////////////////////////////////////////////////////
namespace ZYPP {
//////////////////////////////////////////////////////////////////

using namespace std;


IMPL_DERIVED_POINTER(Resolvable,Spec);

//---------------------------------------------------------------------------

string
Resolvable::asString ( bool full ) const
{
    return toString (*this, full);
}


string
Resolvable::toString ( const Resolvable & resolvable, bool full )
{
    string res;

    res += Spec::toString(resolvable);
    if (!resolvable.channel()->system()) {
	res += "[";
	res += (resolvable.channel() == NULL) ? "(channel?)" : resolvable.channel()->name();
	res += "]";
    }
    if (!full) return res;

    if (resolvable.isInstalled()) res += "<installed>";
    if (resolvable.local()) res += "<local>";

    res += "FileSize ";
    res += stringutil::numstring (resolvable.fileSize());
    res += ", InstalledSize ";
    res += stringutil::numstring (resolvable.installedSize());

    if (!resolvable.requires().empty()) {
	res += ", Requires: ";
	res += Dependency::toString(resolvable.requires());
    }

    if (!resolvable.provides().empty()) {
	res += ", Provides: ";
	res += Dependency::toString(resolvable.provides());
    }
    if (!resolvable.conflicts().empty()) {
	res += ", Conflicts: ";
	res += Dependency::toString(resolvable.conflicts());
    }
    if (!resolvable.obsoletes().empty()) {
	res += ", Obsoletes: ";
	res += Dependency::toString(resolvable.obsoletes());
    }

    if (!resolvable.suggests().empty()) {
	res += ", Suggests: ";
	res += Dependency::toString(resolvable.suggests());
    }
    if (!resolvable.recommends().empty()) {
	res += ", Recommends: ";
	res += Dependency::toString(resolvable.recommends());
    }
    if (!resolvable.freshens().empty()) {
	res += ", Freshens: ";
	res += Dependency::toString(resolvable.freshens());
    }
    return res;
}


string
Resolvable::toString ( const CResolvableList & rl, bool full )
{
    string res("[");
    for (CResolvableList::const_iterator iter = rl.begin(); iter != rl.end(); iter++) {
	if (iter != rl.begin()) res += ", ";
	res += (*iter)->asString(full);
    }
    return res + "]";
}


ostream &
Resolvable::dumpOn( ostream & str ) const
{
    str << asString();
    return str;
}


ostream&
operator<<( ostream& os, const Resolvable& edition)
{
    return os << edition.asString();
}

//---------------------------------------------------------------------------

Resolvable::Resolvable (const Kind & kind, const string & name, int epoch, const string & version, const string & release, const Arch * arch)
    :Spec (kind, name, epoch, version, release, arch)
    , _channel (false)
    , _installed (false)
    , _local (false)
    , _locked (false)
    , _file_size (0)
    , _installed_size (0)

{
}


Resolvable::~Resolvable()
{
}

//---------------------------------------------------------------------------

bool
Resolvable::isInstalled () const
{
    if (_channel != NULL
	&& _channel->system()) {
	return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////
}; // namespace ZYPP
///////////////////////////////////////////////////////////////////
