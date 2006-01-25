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
#include "zypp/base/Iterator.h"
#include "zypp/base/Algorithm.h"

#include "zypp/ResFilters.h"
#include "zypp/ResStatus.h"

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
using namespace zypp;

//-----------------------------------------------------------------------------

InstallOrder::InstallOrder(const ResPool *pool, const CPoolItemList & toinstall, const CPoolItemList & installed)
    : _pool (pool)
    , _dirty (true)
    , _numrun (0)
{
    for (CPoolItemList::const_iterator iter = toinstall.begin(); iter != toinstall.end(); ++iter)
	_toinstall.insert (*iter);
    for (CPoolItemList::const_iterator iter = installed.begin(); iter != installed.end(); ++iter)
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
	string name = gcit->first->resolvable()->name();
	os << "\"" << name << "\"" << "[label=\"" << name << "\\n" << order << "\"";
	os << "] " << endl;
	for (CPoolItemSet::const_iterator scit = gcit->second.begin(); scit != gcit->second.end(); ++scit)
	{
	    os << "\"" << name << "\" -> \"" << (*scit)->resolvable()->name() << "\"" << endl;
	}
    }
    os << "}" << endl;
}


//-----------------------------------------------------------------------------

// yea, that stuff is suboptimal. there should be a heap sorted by order
CPoolItemList
InstallOrder::computeNextSet()
{
    CPoolItemList newlist;

    if (_dirty) startrdfs();

    for (Nodes::iterator it = _nodes.begin(); it != _nodes.end(); ++it)
    {
	if (it->second.order == 0)
	{
	    _DBG("RC_SPEW") << "InstallOrder::computeNextSet found " << it->second.item << endl;

	    newlist.push_back(it->second.item);
	}
    }

    return newlist;
}


// decrease order of every adjacent node
void
InstallOrder::setInstalled(const PoolItem * item )
{
    _dirty = true;

    CPoolItemSet & adj = _rgraph[item];

    _DBG("RC_SPEW") << "InstallOrder::setInstalled " << item << endl;

    // order will be < 0
    _nodes[item].order--;
    _installed.insert (item);
    _toinstall.erase (item);

    for (CPoolItemSet::iterator it = adj.begin(); it != adj.end(); ++it)
    {
	NodeInfo& info = _nodes[*it];
	info.order--;
	if (info.order < 0)
	{
	    DBG << "order of node " << (*it) << " is < 0" << endl;
	}
    }
}


void
InstallOrder::setInstalled( const CPoolItemList & rl )
{
    for (CPoolItemList::const_iterator it = rl.begin(); it != rl.end(); ++it)
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
    for (CPoolItemSet::iterator it = _toinstall.begin(); it != _toinstall.end(); ++it)
    {
	const PoolItem * item = *it;
	_nodes[item] = NodeInfo (item);
	_rgraph[item] = CPoolItemSet();
	_graph[item] = CPoolItemSet();
    }

    // visit all nodes
    for (CPoolItemSet::iterator it = _toinstall.begin(); it != _toinstall.end(); ++it)
    {
	const PoolItem * item = *it;
	if (_nodes[item].visited == false)
	{
	    _DBG("RC_SPEW") << "start recursion on " << item << endl;
	    rdfsvisit (item);
	}
    }

    _dirty = false;
}


typedef struct {
    const PoolItem * requestor;
    CPoolItemSet *tovisit;   
    CPoolItemSet *toinstall;
    CPoolItemSet *installed;
} CollectProvidersInfo;


// item provides cap which matches a requirement from info->requestor
//   this function gets _all_ providers and filter out those which are
//   either installed or in our toinstall input list
//

static bool
collect_providers (PoolItem * item, const Capability & cap, void *data)
{
    CollectProvidersInfo *info = (CollectProvidersInfo *)data;

    if ((item != info->requestor)  					// package could provide its own requirement
#warning fixme
//	&& (!(item->status().isInstalled())						// only visit if provider is not already installed
	&& (info->toinstall->find(item) != info->toinstall->end()		// only look at packages
	    || info->installed->find(item) != info->installed->end())) {	//   we are currently considering anyways
	info->tovisit->insert (item);
    }

    return true;
}


void
InstallOrder::rdfsvisit (const PoolItem * item)
{
    typedef list<Capability> CapList;
    CapList requires;

    _DBG ("RC_SPEW") << "InstallOrder::rdfsvisit, visiting " << item << endl;

    NodeInfo& nodeinfo = _nodes[item];

    nodeinfo.visited = true;
    nodeinfo.begintime = _rdfstime;
    _rdfstime++;

    // put prerequires first and requires last on list to ensure
    // that prerequires are processed first

    for (CapSet::const_iterator it = (*item)->dep (Dep::PREREQUIRES).begin(); it != (*item)->dep (Dep::PREREQUIRES).end(); ++it)
    {
	const Capability cap = *it;
	requires.push_back(cap);
    }

    for (CapSet::const_iterator it = (*item)->dep (Dep::REQUIRES).begin(); it != (*item)->dep (Dep::REQUIRES).end(); ++it)
    {
	const Capability cap = *it;
	requires.push_back(cap);
    }

    for (CapList::const_iterator iter = requires.begin(); iter != requires.end(); ++iter)
    {
	const Capability requirement = *iter;
	_DBG("RC_SPEW") << "check requirement " << requirement << " of " << item << endl;
	CPoolItemSet tovisit;

	CollectProvidersInfo info = { item, &tovisit, &_toinstall, &_installed };
#warning fixme
//	_world->foreachProvidingResItem (requirement, collect_providers, &info);
	invokeOnEach( _pool->byCapabilityIndexBegin( _capability.index(), dep ),
		      _pool->byCapabilityIndexEnd( _capability.index(), dep ),
		      resfilter::callOnCapMatchIn( dep, _capability, info ) );
//	invokeOnEach( make_filter_begin( ByCapability (requirement, Dep::PROVIDES), *_pool ),
//		      make_filter_end( ByCapability (requirement, Dep::PROVIDES), *_pool ),
//		      Print() );

	for (CPoolItemSet::iterator it = tovisit.begin(); it != tovisit.end(); ++it)
	{
	    const PoolItem *must_visit = *it;
	    if (_nodes[must_visit].visited == false)
	    {
		nodeinfo.order++;
		_rgraph[must_visit].insert (item);
		_graph[item].insert (must_visit);
		rdfsvisit(must_visit);
	    }
	    else if (_nodes[must_visit].endtime == 0)
	    {
		if (must_visit != item)
		{
		    DBG << "dependency loop: " << item << " -> " << must_visit << endl;
		}
	    }
	    else
	    {
		// filter multiple depends on same item (cosmetic)
		CPoolItemSet & lrg = _rgraph[must_visit];
		if (lrg.find(item) == lrg.end())
		{
		    nodeinfo.order++;
		    lrg.insert(item);

		    CPoolItemSet & lg = _graph[item];
		    if (lg.find (must_visit) == lg.end())
			lg.insert (must_visit);
		}
	    }
	}
    }
    _topsorted.push_back(item);
    _nodes[item].endtime = _rdfstime;
    _rdfstime++;

    _DBG("RC_SPEW") << item << " done" << endl;
}


//-----------------------------------------------------------------------------

const CPoolItemList
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
