/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResolverQueue.cc
 *
 * Copyright (C) 2007 SUSE Linux Products GmbH
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

#include "zypp/base/Logger.h"
#include "zypp/solver/detail/ContextPool.h"
#include "zypp/solver/detail/ResolverContext.h"

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

IMPL_PTR_TYPE(ContextPool);
	
#define MAXCONTEXT 10

//---------------------------------------------------------------------------

void  dumpTaskList(const PoolItemList &list )
{
    for (PoolItemList::const_iterator iter = list.begin();
	 iter != list.end(); iter++) {
	DBG << "          " << *iter << endl;
    }
}

//---------------------------------------------------------------------------

ostream&
operator<<( ostream& os, const ContextPool & contextPool)
{
    os << str::form ("ContextPool [entries: %d]", contextPool.contextListSize()) << endl;
    return os;
}

//---------------------------------------------------------------------------

ContextPool::ContextPool (const int maxCount)
    : maxContext (maxCount)
{
    _XDEBUG("ContextPool::ContextPool(" << maxContext << ")");
}


ContextPool::ContextPool ()
    : maxContext (MAXCONTEXT)
{
    _XDEBUG("ContextPool::ContextPool(" << MAXCONTEXT << ")");
}

ContextPool::~ContextPool()
{
}

//---------------------------------------------------------------------------
void ContextPool::addContext (ResolverContext_Ptr context,
			      const PoolItemList & installItems,
			      const PoolItemList & deleteItems,
			      const PoolItemList & lockUninstalledItems)
{
    if ((installItems.size() == 0
	&& deleteItems.size() == 0)
	|| context == NULL )
	return; // empty context is useless

    // make an copy
    ResolverContext_Ptr new_context = new ResolverContext (context->pool(), context->architecture(),
							   context);
    
    new_context->setUserInstallItems (installItems);
    new_context->setUserDeleteItems (deleteItems);
    new_context->setUserLockUninstalledItems (lockUninstalledItems);    
    
    if (contextList.size() <= 0) {
	contextList.push_front (new_context);
    } else {
	// does context already exists in the list ?
	bool exists = false;

	for ( ResolverContextList::iterator it = contextList.begin(); it != contextList.end(); ++it ) {
	    // checking installed items
	    PoolItemList left = (*it)->userInstallItems();
	    PoolItemList right = context->userInstallItems();
	    if (left.size() != right.size())
		continue;
	    bool found = true;	    
	    for (PoolItemList::iterator itleft = left.begin();
		 (itleft != left.end()) && found ; ++itleft) {
		found = false;
		for (PoolItemList::iterator itright = right.begin(); itright != right.end(); ++itright) {
		    if (*itleft == *itright) {
			found = true;
			break;
		    }
		}
	    }

	    if (!found ) continue;
	    
	    // checking deleted items
	    left = (*it)->userDeleteItems();
	    right = context->userDeleteItems();
	    if (left.size() != right.size())
		continue;
	    found = true;	    
	    for (PoolItemList::iterator itleft = left.begin();
		 (itleft != left.end()) && found ; ++itleft) {
		found = false;
		for (PoolItemList::iterator itright = right.begin(); itright != right.end(); ++itright) {
		    if (*itleft == *itright) {
			found = true;
			break;
		    }
		}
	    }

	    if (!found ) continue;
	    
	    // checking locked items
	    left = (*it)->userLockUninstalledItems();
	    right = context->userLockUninstalledItems();
	    if (left.size() != right.size())
		continue;
	    found = true;	    
	    for (PoolItemList::iterator itleft = left.begin();
		 (itleft != left.end()) && found ; ++itleft) {
		found = false;
		for (PoolItemList::iterator itright = right.begin(); itright != right.end(); ++itright) {
		    if (*itleft == *itright) {
			found = true;
			break;
		    }
		}
	    }
	    
	    if (found) {
		exists = true;
		break;
	    }
	}

	if (!exists) {
	    contextList.push_front (new_context);
	} else {
	    _XDEBUG("----------------------");    
	    _XDEBUG("ContextPool::addContext Context ALREADY INSERTED ");
	    _XDEBUG("----------------------");
	}
    }

    if (contextList.size() > 0
	&& contextList.size() > maxContext)
    {
	// keep max size
	contextList.remove (contextList.back());
    }

    _XDEBUG("----------------------");    
    _XDEBUG("ContextPool::addContext latest context with ");
    _XDEBUG("   installed:");
    dumpTaskList (new_context->userInstallItems());
    _XDEBUG("   deleted:");
    dumpTaskList (new_context->userDeleteItems());
    _XDEBUG("   locked:");
    dumpTaskList (new_context->userLockUninstalledItems());    
#if 0
    _XDEBUG("CONTEXT : " << endl << *new_context );
#endif
    _XDEBUG("----------------------");
#if 0
    int count = 0;
    for ( ResolverContextList::iterator it = contextList.begin(); it != contextList.end(); ++it ) {

	PoolItemList contextInstall = (*it)->userInstallItems();
	_XDEBUG(" inserted CONTEXT Nr " << count++ << " of " << contextList.size() << " : " << endl << **it );    	
	_XDEBUG(" with entries");
	dumpTaskList (contextInstall);
    _XDEBUG("----------------------");    	
    }
#endif
    
}

ResolverContext_Ptr ContextPool::findContext (PoolItemList & installItems,
					      PoolItemList & deleteItems,
					      const PoolItemList & lockUninstalledItems)
{
    // searching for context with same entries
    int counter = 1;
    for ( ResolverContextList::iterator it = contextList.begin(); it != contextList.end(); ++it ) {

	PoolItemList contextInstall = (*it)->userInstallItems();
	PoolItemList contextDelete = (*it)->userDeleteItems();
	PoolItemList contextLockUninstalled = (*it)->userLockUninstalledItems();		
	
	_XDEBUG("ContextPool::findContext() trying " << counter++ << ". of " <<  contextList.size() );	
	_XDEBUG("   comparing");
	_XDEBUG("      installed:");	
	dumpTaskList (contextInstall);
	_XDEBUG("      deleted:");	
	dumpTaskList (contextDelete);
	_XDEBUG("      lockedUninstalled:");	
	dumpTaskList (contextLockUninstalled);
	
	_XDEBUG("   with needed");
	_XDEBUG("      installed:");
	dumpTaskList (installItems);	
	_XDEBUG("      deleted:");	
	dumpTaskList (deleteItems);
	_XDEBUG("      lockedUninstalled:");	
	dumpTaskList (lockUninstalledItems);
	
	if (contextInstall.size() > installItems.size()
	    || contextDelete.size() > deleteItems.size()
	    || contextLockUninstalled.size() != lockUninstalledItems.size())
	    continue; // cannot fit at all

	bool found = true;	    	
	
	found = true;
	// check if the lock of unistalled items are the same.
	// If not --> try the next context
	for (PoolItemList::iterator itContext = contextLockUninstalled.begin();
	     (itContext != contextLockUninstalled.end()) && found; ++itContext) {
	    found = false;
	    for (PoolItemList::const_iterator itInstall = lockUninstalledItems.begin();
		 itInstall != lockUninstalledItems.end(); ++itInstall) {
		if (*itContext == *itInstall) {
		    found = true;
		    break;
		}
	    }
	}
	if (!found) continue;

	// checking items which will be installed
	PoolItemList addInsItems = installItems;
	for (PoolItemList::iterator itContext = contextInstall.begin();
	     (itContext != contextInstall.end()) && found; ++itContext) {
	    found = false;
	    for (PoolItemList::iterator itInstall = addInsItems.begin();
		 itInstall != addInsItems.end(); ++itInstall) {
		if (*itContext == *itInstall) {
		    found = true;
		    addInsItems.remove (*itInstall); 
		    break;
		}
	    }
	}

	PoolItemList addDelItems = deleteItems;	
	if (found) {
	    // checking items which will be deleted
	    PoolItemList addDelItems = deleteItems;
	    for (PoolItemList::iterator itContext = contextDelete.begin();
		 (itContext != contextDelete.end()) && found; ++itContext) {
		found = false;
		for (PoolItemList::iterator itInstall = addDelItems.begin();
		     itInstall != addDelItems.end(); ++itInstall) {
		    if (*itContext == *itInstall) {
			found = true;
			addDelItems.remove (*itInstall); 
			break;
		    }
		}
	    }
	}
	
	if (found) {
	    if (addInsItems.size() > 0
		|| addDelItems.size() > 0) {
		_XDEBUG("ContextPool::findContext() found one with following additional install/delete items: ");
		_XDEBUG("      installed:");
		dumpTaskList (addInsItems);	
		_XDEBUG("      deleted:");	
		dumpTaskList (addDelItems);
	    } else {
		_XDEBUG("ContextPool::findContext() found with the SAME selected entries. ");
	    }
	    installItems = addInsItems; // Rest of items which has to be installed additionally
	    deleteItems = addDelItems; // Rest of items which has to be deleted additionally	    
#if 0
	    _XDEBUG("----------------------");    	    
	    _XDEBUG(" returned CONTEXT : " << endl << **it );
	    _XDEBUG("----------------------");
#endif
	    return *it;
	}
    }
    // nothing found
    _XDEBUG("ContextPool::findContext() have not found a proper context. Solving completely");
    return NULL;
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

