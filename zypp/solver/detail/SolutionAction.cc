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
#include "zypp/Capabilities.h"
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


//---------------------------------------------------------------------------

ostream &
TransactionSolutionAction::dumpOn( ostream& os) const
{
    os << "TransactionSolutionAction: ";
    switch (_action) {
	case KEEP:			os << "Keep " << _item; break;
	case INSTALL:			os << "Install " << _item; break;
	case REMOVE:			os << "Remove " << _item; break;
	case UNLOCK:			os << "Unlock " << _item; break;
    	case LOCK:			os << "Lock " << _item; break;
	case REMOVE_EXTRA_REQUIRE:	os << "Remove require " << _capability; break;
	case REMOVE_EXTRA_CONFLICT:	os << "Remove conflict " << _capability; break;
	case ADD_SOLVE_QUEUE_ITEM:	os << "Add SolveQueueItem " <<  _solverQueueItem; break;
	case REMOVE_SOLVE_QUEUE_ITEM:	os << "Remove SolveQueueItem " <<  _solverQueueItem; break;
    }
    return os;
}


ostream&
operator<<( ostream& os, const SolutionActionList & actionlist)
{
    for (SolutionActionList::const_iterator iter = actionlist.begin(); iter != actionlist.end(); ++iter) {
	os << *(*iter);
	os << endl;
    }
    return os;
}


ostream&
operator<<( ostream& os, const CSolutionActionList & actionlist)
{
    for (CSolutionActionList::const_iterator iter = actionlist.begin(); iter != actionlist.end(); ++iter) {
	os << *(*iter);
	os << endl;
    }
    return os;
}

//---------------------------------------------------------------------------

ostream &
InjectSolutionAction::dumpOn( ostream& os ) const
{
    os << "InjectSolutionAction: ";
    switch (_kind) {
	case WEAK:	os << "Weak"; break;
	default: os << "Wrong kind"; break;
    }
    os << " ";
    os << _item;
    return os;
}

//---------------------------------------------------------------------------


ostream &
SolutionAction::dumpOn( std::ostream & os ) const
{
    os << "SolutionAction<";
    os << "not specified";
    os << "> ";
    return os;
}


bool
TransactionSolutionAction::execute(Resolver & resolver) const
{
    bool ret = true;
    switch (action()) {
	case KEEP:
	    _item.status().resetTransact (ResStatus::USER);
    	    ret = _item.status().setTransact (false, ResStatus::APPL_HIGH); // APPL_HIGH: Locking should not be saved permanently
	    break;
	case INSTALL:
	    if (_item.status().isToBeUninstalled())
		ret = _item.status().setTransact (false, ResStatus::USER);
	    else
		_item.status().setToBeInstalled (ResStatus::USER);
	    break;
	case REMOVE:
	    if (_item.status().isToBeInstalled()) {
		_item.status().setTransact (false,ResStatus::USER);
		_item.status().setLock (true,ResStatus::USER); // no other dependency can set it again
	    } else if (_item.status().isInstalled())
		_item.status().setToBeUninstalled (ResStatus::USER);
	    else
		_item.status().setLock (true,ResStatus::USER); // no other dependency can set it again
	    break;
	case UNLOCK:
	    ret = _item.status().setLock (false, ResStatus::USER);
	    if (!ret) ERR << "Cannot unlock " << _item << endl;
	    break;
	case LOCK:
    	    _item.status().resetTransact (ResStatus::USER);
	    ret = _item.status().setLock (true, ResStatus::APPL_HIGH); // APPL_HIGH: Locking should not be saved permanently
	    if (!ret) ERR << "Cannot lock " << _item << endl;
	    break;
	case REMOVE_EXTRA_REQUIRE:
	    resolver.removeExtraRequire (_capability);
	    break;
	case REMOVE_EXTRA_CONFLICT:
	    resolver.removeExtraConflict (_capability);
	    break;
	case ADD_SOLVE_QUEUE_ITEM:
	    resolver.addQueueItem(_solverQueueItem);
	    break;
	case REMOVE_SOLVE_QUEUE_ITEM:
	    resolver.removeQueueItem(_solverQueueItem);
	    break;
	default:
	    ERR << "Wrong TransactionKind" << endl;
	    ret = false;
    }
    return ret;
}

bool
InjectSolutionAction::execute(Resolver & resolver) const
{
    switch (_kind) {
        case WEAK:
	    // set item dependencies to weak
	    resolver.addWeak (_item);
            break;
        default:
	    ERR << "No valid InjectSolutionAction kind found" << endl;
	    return false;
    }

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
