/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* Packman.cc
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

#include <zypp/solver/detail/Packman.h>
#include <zypp/solver/detail/debug.h>

///////////////////////////////////////////////////////////////////
namespace zypp {
//////////////////////////////////////////////////////////////////

using namespace std;

IMPL_BASE_POINTER(Packman);

//---------------------------------------------------------------------------

string
Packman::asString ( void ) const
{
    return toString (*this);
}


string
Packman::toString ( const Packman & store )
{
    string res ("<packman/>");

    return res;
}


ostream &
Packman::dumpOn (ostream & str) const
{
    str << asString();
    return str;
}


ostream &
operator<< (ostream & os, const Packman & store)
{
    return os << store.asString();
}

//---------------------------------------------------------------------------

Packman::Packman ()
{
}


Packman::~Packman()
{
}


///////////////////////////////////////////////////////////////////
}; // namespace zypp
///////////////////////////////////////////////////////////////////

