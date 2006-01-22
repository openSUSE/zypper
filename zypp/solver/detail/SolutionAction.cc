/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* SolutionAction.cc
 *
 * Easy-to use interface to the ZYPP dependency resolver
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

#include "zypp/solver/detail/Resolver.h"
#include "zypp/solver/detail/SolutionAction.h"

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

IMPL_PTR_TYPE(SolutionAction);
IMPL_PTR_TYPE(TransactionSolutionAction);
IMPL_PTR_TYPE(InjectSolutionAction);

//---------------------------------------------------------------------------

SolutionAction::SolutionAction()
{
}


SolutionAction::~SolutionAction()
{
}


std::string
SolutionAction::toString (const SolutionActionList & actionlist)
{
    string ret;
    for (SolutionActionList::const_iterator iter = actionlist.begin(); iter != actionlist.end(); ++iter) {
	ret += (*iter)->asString();
	ret += "\n";
    }
    return ret;
}

std::string
SolutionAction::toString (const CSolutionActionList & actionlist)
{
    string ret;
    for (CSolutionActionList::const_iterator iter = actionlist.begin(); iter != actionlist.end(); ++iter) {
	ret += (*iter)->asString();
	ret += "\n";
    }
    return ret;
}

//---------------------------------------------------------------------------

string
TransactionSolutionAction::asString ( void ) const
{
    return toString (*this);
}


string
TransactionSolutionAction::toString ( const TransactionSolutionAction & action )
{
    string ret ("TransactionSolutionAction: ");
    switch (action._action) {
	case Keep:	ret += "Keep"; break;
	case Install:	ret += "Install"; break;
	case Update:	ret += "Update"; break;
	case Remove:	ret += "Remove"; break;
    }
    ret += " ";
    ret += action._resolvable->asString();
    ret += "\n";
    return ret;
}


ostream &
TransactionSolutionAction::dumpOn( ostream & str ) const
{
    str << asString();
    return str;
}


ostream&
operator<<( ostream& os, const TransactionSolutionAction & action)
{
    return os << action.asString();
}

//---------------------------------------------------------------------------

string
InjectSolutionAction::asString ( void ) const
{
    return toString (*this);
}


string
InjectSolutionAction::toString ( const InjectSolutionAction & action )
{
    string ret ("InjectSolutionAction: ");
    ret += action._capability.asString();
    ret += "\n";
    return ret;
}


ostream &
InjectSolutionAction::dumpOn( ostream & str ) const
{
    str << asString();
    return str;
}


ostream&
operator<<( ostream& os, const InjectSolutionAction & action)
{
    return os << action.asString();
}

//---------------------------------------------------------------------------

bool 
TransactionSolutionAction::execute()
{
    return true;
}

bool
InjectSolutionAction::execute()
{
    return true;
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
