/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* Match.cc
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

#include <zypp/solver/detail/Match.h>
#include <zypp/solver/detail/World.h>

///////////////////////////////////////////////////////////////////
namespace ZYPP {
//////////////////////////////////////////////////////////////////

using namespace std;

IMPL_BASE_POINTER(Match);

//---------------------------------------------------------------------------

string
Match::asString ( void ) const
{
    return toString (*this);
}


string
Match::toString ( const Match & lock )
{
    return "<lock/>";
}


ostream &
Match::dumpOn( ostream & str ) const
{
    str << asString();
    return str;
}


ostream&
operator<<( ostream& os, const Match& edition)
{
    return os << edition.asString();
}

//---------------------------------------------------------------------------

Match::Match()
    : _importance (Importance::Undefined)
{
}

Match::Match (XmlNodePtr node)
    : _importance (Importance::Undefined)
{
}

Match::~Match()
{
}

//---------------------------------------------------------------------------

XmlNodePtr
Match::asXmlNode (void) const
{
    return new XmlNode("match");
}


// equality
bool
Match::equals ( const Match & lock ) const {

    // Check the name glob

    if ((_name_glob.empty()) ^ (lock._name_glob.empty()))
	return false;
    if (_name_glob != lock._name_glob)
	return false;

    // Check the channel

    if ((_channel_id.empty()) ^ (lock._channel_id.empty()))
	return false;
    if (_channel_id != lock._channel_id)
	return false;

    // Check the importance

    if (_importance != lock._importance
	|| _importance_gteq != lock._importance_gteq)
    {
	return false;
    }

    // Check the dep
    if ((_dependency == NULL) ^ (lock._dependency == NULL))
	return false;
    if (_dependency && lock._dependency) {
	if (!( ((constSpecPtr) _dependency)->equals((constSpecPtr) lock._dependency))
	    || (_dependency->relation() != lock._dependency->relation())
	    || (_dependency->kind() != lock._dependency->kind()))
	{
    	return false;
	}
    }

    return true;
}


bool
Match::test (constResolvablePtr resolvable, WorldPtr world) const
{
    string name;
    constChannelPtr channel = resolvable->channel ();
  
    if (channel != NULL && !_channel_id.empty()) {
	if (! channel->hasEqualId (_channel_id)) {
	    return false;
	}
    }

    name = resolvable->name ();

// FIXME, implement regexp
#if 0
    if (match->_pattern_spec
	&& ! g_pattern_match_string (match->pattern_spec, name)) {
	return false;
    }
#endif

  /* FIXME: Resolvables don't have ResolvableUpdate right now */
/*   if (match->importance != RC_IMPORTANCE_INVALID && */
/* 	  !rc_resolvable_is_installed (resolvable)) { */
/* 	  RCResolvableUpdate *up = rc_resolvable_get_latest_update (pkg); */
/* 	  if (up) { */
/* 		  if (match->importance_gteq ? up->importance > match->importance */
/* 			  : up->importance < match->importance) */
/* 			  return FALSE; */
/* 	  } */
/*   } */

    if (_dependency) {
	DependencyPtr dependency;
	bool check;

	dependency = new Dependency (resolvable->name(), Relation::Equal, Kind::Package, resolvable->channel(), resolvable->edition());
	check = _dependency->verifyRelation (dependency);
	return check;
    }

    return true;
}

///////////////////////////////////////////////////////////////////
}; // namespace ZYPP
///////////////////////////////////////////////////////////////////

