/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* InstallOrder.cc
 *
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

// stolen from yast2-packagemanager
/*
   File:       InstallOrder.h
   Purpose:    Determine order for installing packages
   Author:     Ludwig Nussel <lnussel@suse.de>
   Maintainer: Ludwig Nussel <lnussel@suse.de>

/-*/

#include "zypp/solver/detail/InstallOrder.h"
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

//-----------------------------------------------------------------------------

InstallOrder::InstallOrder(World_Ptr world, const CResItemList& toinstall, const CResItemList& installed)
    : _world (world)
    , _dirty (true)
    , _numrun (0)
{
    for (CResItemList::const_iterator iter = toinstall.begin(); iter != toinstall.end(); ++iter)
	_toinstall.insert (*iter);
    for (CResItemList::const_iterator iter = installed.begin(); iter != installed.end(); ++iter)
	_installed.insert (*iter);
}


//-----------------------------------------------------------------------------

const void
InstallOrder::printAdj (std::ostream& os, bool reversed) const
{
    const Graph& g = (reversed ? _rgraph : _graph);
    os << "digraph pkgdeps {" << endl;
    for (Graph::const_iterator gcit = g.begin(); gcit != g.end(); ++gcit)
    {
	Nodes::const_iterator niit = _nodes.find(gcit->first);
	int order = niit->second.order;
	string name = gcit->first->name();
	os << "\"" << name << "\"" << "[label=\"" << name << "\\n" << order << "\"";
	os << "] " << endl;
	for (CResItemSet::const_iterator scit = gcit->second.begin(); scit != gcit->second.end(); ++scit)
	{
	    os << "\"" << name << "\" -> \"" << (*scit)->name() << "\"" << endl;
	}
    }
    os << "}" << endl;
}


//-----------------------------------------------------------------------------

// yea, that stuff is suboptimal. there should be a heap sorted by order
CResItemList
InstallOrder::computeNextSet()
{
    CResItemList newlist;

    if (_dirty) startrdfs();

    for (Nodes::iterator it = _nodes.begin(); it != _nodes.end(); ++it)
    {
	if (it->second.order == 0)
	{
	    _DBG("RC_SPEW") << "InstallOrder::computeNextSet found " << it->second.resItem->asString() << endl;

	    newlist.push_back(it->second.resItem);
	}
    }

    return newlist;
}


// decrease order of every adjacent node
void
InstallOrder::setInstalled( ResItem_constPtr resItem )
{
    _dirty = true;

    CResItemSet& adj = _rgraph[resItem];

    _DBG("RC_SPEW") << "InstallOrder::setInstalled " << resItem->asString() << endl;

    // order will be < 0
    _nodes[resItem].order--;
    _installed.insert (resItem);
    _toinstall.erase (resItem);

    for (CResItemSet::iterator it = adj.begin(); it != adj.end(); ++it)
    {
	NodeInfo& info = _nodes[*it];
	info.order--;
	if (info.order < 0)
	{
	    cerr << "order of node " << (*it)->asString() << " is < 0" << endl;
	}
    }
}


void
InstallOrder::setInstalled( const CResItemList& rl )
{
    for (CResItemList::const_iterator it = rl.begin(); it != rl.end(); ++it)
    {
	setInstalled(*it);
    }
}

//-----------------------------------------------------------------------------


void
InstallOrder::startrdfs()
{
    _nodes.clear();
    _rgraph.clear();
    _graph.clear();

    _rdfstime = 1;

    _topsorted.clear();

    _numrun++;
    _DBG("RC_SPEW") << "run #" << _numrun << endl;

    // initialize all nodes
    for (CResItemSet::iterator it = _toinstall.begin(); it != _toinstall.end(); ++it)
    {
	ResItem_constPtr resItem = *it;
	_nodes[resItem] = NodeInfo (resItem);
	_rgraph[resItem] = CResItemSet();
	_graph[resItem] = CResItemSet();
    }

    // visit all nodes
    for (CResItemSet::iterator it = _toinstall.begin(); it != _toinstall.end(); ++it)
    {
	ResItem_constPtr resItem = *it;
	if (_nodes[resItem].visited == false)
	{
	    _DBG("RC_SPEW") << "start recursion on " << resItem->asString() << endl;
	    rdfsvisit (resItem);
	}
    }

    _dirty = false;
}


typedef struct {
    ResItem_constPtr requestor;
    CResItemSet *tovisit;   
    CResItemSet *toinstall;
    CResItemSet *installed;
} CollectProvidersInfo;

// resItem provides cap which matches a requirement from info->requestor

static bool
collect_providers (ResItem_constPtr resItem, const Capability & cap, void *data)
{
    CollectProvidersInfo *info = (CollectProvidersInfo *)data;

    if ((resItem != info->requestor)  					// package could provide its own requirement
	&& (!resItem->isInstalled())						// only visit if provider is not already installed
	&& (info->toinstall->find(resItem) != info->toinstall->end()		// only look at packages
	    || info->installed->find(resItem) != info->installed->end())) {	//   we are currently considering anyways
	info->tovisit->insert (resItem);
    }

    return true;
}


void
InstallOrder::rdfsvisit(ResItem_constPtr resItem)
{
    typedef list<Capability> CapList;
    CapList requires;

    _DBG ("RC_SPEW") << "InstallOrder::rdfsvisit, visiting " << resItem->asString() << endl;

    NodeInfo& nodeinfo = _nodes[resItem];

    nodeinfo.visited = true;
    nodeinfo.begintime = _rdfstime;
    _rdfstime++;

    // put prerequires first and requires last on list to ensure
    // that prerequires are processed first

    for (CapSet::const_iterator it = resItem->prerequires().begin(); it != resItem->prerequires().end(); ++it)
    {
	const Capability cap = *it;
	requires.push_back(cap);
    }

    for (CapSet::const_iterator it = resItem->requires().begin(); it != resItem->requires().end(); ++it)
    {
	const Capability cap = *it;
	requires.push_back(cap);
    }

    for (CapList::const_iterator iter = requires.begin(); iter != requires.end(); ++iter)
    {
	const Capability requirement = *iter;
	_DBG("RC_SPEW") << "check requirement " << requirement.asString() << " of " << resItem->asString() << endl;
	CResItemSet tovisit;

	CollectProvidersInfo info = { resItem, &tovisit, &_toinstall, &_installed };
	_world->foreachProvidingResItem (requirement, collect_providers, &info);

	for (CResItemSet::iterator it = tovisit.begin(); it != tovisit.end(); ++it)
	{
	    ResItem_constPtr must_visit = *it;
	    if (_nodes[must_visit].visited == false)
	    {
		nodeinfo.order++;
		_rgraph[must_visit].insert (resItem);
		_graph[resItem].insert (must_visit);
		rdfsvisit(must_visit);
	    }
	    else if (_nodes[must_visit].endtime == 0)
	    {
		if (must_visit != resItem)
		{
		    cerr << "dependency loop: " << resItem->asString() << " -> " << must_visit->asString() << endl;
		}
	    }
	    else
	    {
		// filter multiple depends on same resItem (cosmetic)
		CResItemSet & lrg = _rgraph[must_visit];
		if (lrg.find(resItem) == lrg.end())
		{
		    nodeinfo.order++;
		    lrg.insert(resItem);

		    CResItemSet& lg = _graph[resItem];
		    if (lg.find (must_visit) == lg.end())
			lg.insert (must_visit);
		}
	    }
	}
    }
    _topsorted.push_back(resItem);
    _nodes[resItem].endtime = _rdfstime;
    _rdfstime++;

    _DBG("RC_SPEW") << resItem->asString() << " done" << endl;
}


//-----------------------------------------------------------------------------

const CResItemList&
InstallOrder::getTopSorted() const
{
    return _topsorted;
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
