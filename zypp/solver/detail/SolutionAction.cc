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
#include "zypp/CapSet.h"
#include "zypp/base/Logger.h"
#include "zypp/Dependencies.h"


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
	case KEEP:	os << "Keep"; break;
	case INSTALL:	os << "Install"; break;
	case REMOVE:	os << "Remove"; break;
	case UNLOCK:	os << "Unlock"; break;	    
    }
    os << " ";
    os << _item;
    os << endl;
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
    os << _capability;
    os << ", ";
    switch (_kind) {
	case REQUIRES:	os << "Requires"; break;
	case CONFLICTS:	os << "Conflicts"; break;
	case OBSOLETES: os << "Obsoletes"; break;
	case INSTALLED: os << "Installed"; break;
	case ARCHITECTURE: os << "Architecture"; break;	    
	default: os << "Wrong kind"; break;
    }
    os << " ";
    os << _item;	    
    os << endl;
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
	    resolver.addIgnoreInstalledItem( _item );
	    /*FALLTHRU*/
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
	default:
	    ERR << "Wrong TransactionKind" << endl;
	    ret = false;
    }
    return ret;
}

bool
InjectSolutionAction::execute(Resolver & resolver) const
{
    Dependencies dependencies = _item.resolvable()->deps();
    CapSet depList = dependencies[Dep::CONFLICTS];    
    switch (_kind) {
        case CONFLICTS:
	    // removing conflict in both resolvables
	    if (depList.find(_capability) != depList.end())
	    {
		resolver.addIgnoreConflict (_item, _capability);
	    }
	    dependencies = _otherItem.resolvable()->deps();
	    depList = dependencies[Dep::CONFLICTS];
	    if (depList.find(_capability) != depList.end())
	    {
		resolver.addIgnoreConflict (_otherItem, _capability);
	    }
	    break;
        case REQUIRES:
	    // removing the requires dependency from the item
	    resolver.addIgnoreRequires (_item, _capability);
	    break;
        case ARCHITECTURE:
	    // This item is for ALL architectures available
	    resolver.addIgnoreArchitectureItem (_item);
	    break;	    
        case OBSOLETES:
	    // removing the obsoletes dependency from the item
	    resolver.addIgnoreObsoletes (_otherItem, _capability);
	    break;	    
        case INSTALLED:
	    // ignoring already installed items
	    resolver.addIgnoreInstalledItem (_item);
	    resolver.addIgnoreInstalledItem (_otherItem);
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
