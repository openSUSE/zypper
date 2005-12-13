/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* Section.cc
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

#include <zypp/solver/detail/Section.h>

#include <zypp/solver/detail/debug.h>

///////////////////////////////////////////////////////////////////
namespace ZYPP {
//////////////////////////////////////////////////////////////////

using namespace std;

string
Section::asString ( void ) const
{
    return toString (*this);
}


string
Section::asUserString ( void ) const
{
    return toUserString (*this);
}


string
Section::toString ( const Section & section )
{
    switch (section.section()) {
    case SECTION_OFFICE:	return ("office");
    case SECTION_IMAGING:	return ("imaging");
    case SECTION_PIM:		return ("pim");
    case SECTION_GAME:		return ("game");
    case SECTION_MISC:		return ("misc");
    case SECTION_MULTIMEDIA:	return ("multimedia");
    case SECTION_INTERNET:	return ("internet");
    case SECTION_UTIL:		return ("util");
    case SECTION_SYSTEM:	return ("system");
    case SECTION_DOC:		return ("doc");
    case SECTION_DEVEL:		return ("devel");
    case SECTION_DEVELUTIL:	return ("develutil");
    case SECTION_LIBRARY:	return ("library");
    case SECTION_XAPP:		return ("xapp");
    default:
	rc_debug (RC_DEBUG_LEVEL_WARNING, "invalid section number %d\n", section.section());
    }
    return ("misc");
}

string
Section::toUserString ( const Section & section )
{
    switch (section.section()) {
    case SECTION_OFFICE:	return ("Productivity Applications");
    case SECTION_IMAGING:	return ("Imaging");
    case SECTION_PIM:		return ("Personal Information Management");
    case SECTION_GAME:		return ("Games");
    case SECTION_MISC:		return ("Miscellaneous");
    case SECTION_MULTIMEDIA:	return ("Multimedia");
    case SECTION_INTERNET:	return ("Internet Applications");
    case SECTION_UTIL:		return ("Utilities");
    case SECTION_SYSTEM:	return ("System Packages");
    case SECTION_DOC:		return ("Documentation");
    case SECTION_DEVEL:		return ("Development Packages");
    case SECTION_DEVELUTIL:	return ("Development Utilities");
    case SECTION_LIBRARY:	return ("Libraries");
    case SECTION_XAPP:		return ("X Applications");
    default:
	rc_debug (RC_DEBUG_LEVEL_WARNING, "invalid section number %d\n", section.section());
    }

    return ("Miscellaneous");
}

ostream &
Section::dumpOn( ostream & str ) const
{
    str << asString();
    return str;
}


ostream&
operator<<( ostream& os, const Section& section)
{
    return os << section.asString();
}

//---------------------------------------------------------------------------

Section::Section(const char *section_str)
{
    _section = SECTION_MISC;

    if (section_str != NULL) {
    switch (*section_str) {
    case 'd':
	if (!strcmp (section_str, "develutil")) {
	    _section = SECTION_DEVELUTIL;
	}
	else if (!strcmp (section_str, "devel")) {
	    _section = SECTION_DEVEL;
	}
	else if (!strcmp (section_str, "doc")) {
	    _section = SECTION_DOC;
	}
	break;
    case 'g':
	if (!strcmp (section_str, "game")) {
	    _section = SECTION_GAME;
	}
	break;
    case 'i':
	if (!strcmp (section_str, "imaging")) {
	    _section = SECTION_IMAGING;
	}
	else if (!strcmp (section_str, "internet")) {
	    _section = SECTION_INTERNET;
	}
	break;
    case 'l':
	if (!strcmp (section_str, "library")) {
	    _section = SECTION_LIBRARY;
	}
	break;
    case 'm':
	if (!strcmp (section_str, "misc")) {
	    _section = SECTION_MISC;
	}
	else if (!strcmp (section_str, "multimedia")) {
	    _section = SECTION_MULTIMEDIA;
	}
	break;
    case 'o':
	if (!strcmp (section_str, "office")) {
	    _section = SECTION_OFFICE;
	}
	break;
    case 'p':
	if (!strcmp (section_str, "pim")) {
	    _section = SECTION_PIM;
	}
	break;
    case 's':
	if (!strcmp (section_str, "system")) {
	    _section = SECTION_SYSTEM;
	}
	break;
    case 'u':
	if (!strcmp (section_str, "util")) {
	    _section = SECTION_UTIL;
	}
	break;
    case 'x':
	if (!strcmp (section_str, "xapp")) {
	    _section = SECTION_XAPP;
	}
	break;
    default:
	rc_debug (RC_DEBUG_LEVEL_WARNING, "invalid section name %s\n", section_str);
	break;
    }
    } // if != NULL

}


Section::~Section()
{
}



///////////////////////////////////////////////////////////////////
}; // namespace ZYPP
///////////////////////////////////////////////////////////////////

