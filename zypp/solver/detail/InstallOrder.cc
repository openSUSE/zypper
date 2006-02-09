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

InstallOrder::InstallOrder(const ResPool & pool, const PoolItemList & toinstall, const PoolItemList & installed)
    : _pool (pool)
    , _toinstall (toinstall.begin(), toinstall.end())
    , _installed (installed.begin(), installed.end())
    , _dirty (true)
    , _numrun (0)
{
    _DEBUG("InstallOrder::InstallOrder(_toinstall " << _toinstall.size() << " items, _installed " << _installed.size() << " items)");
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
	string name = gcit->first.resolvable()->name();
	os << "\"" << name << "\"" << "[label=\"" << name << "\\n" << order << "\"";
	os << "] " << endl;
	for (PoolItemSet::const_iterator scit = gcit->second.begin(); scit != gcit->second.end(); ++scit)
	{
	    os << "\"" << name << "\" -> \"" << (*scit).resolvable()->name() << "\"" << endl;
	}
    }
    os << "}" << endl;
}


//-----------------------------------------------------------------------------

// yea, that stuff is suboptimal. there should be a heap sorted by order
PoolItemList
InstallOrder::computeNextSet()
{
    PoolItemList newlist;

    if (_dirty) startrdfs();

    for (Nodes::iterator it = _nodes.begin(); it != _nodes.end(); ++it)
    {
	DBG << "Looking at node " << it->first << " order " << it->second.order << ":" << it->second.item << endl;
	if (it->second.order == 0)
	{
	    DBG << "InstallOrder::computeNextSet found " << it->second.item << endl;

	    newlist.push_back(it->second.item);
	}
    }

    return newlist;
}


// decrease order of every adjacent node
void
InstallOrder::setInstalled(PoolItem_Ref item )
{
    DBG << "InstallOrder::setInstalled " << item << endl;

    Graph::const_iterator git( _rgraph.find(item) );
    if (git == _rgraph.end()) {
	ERR << "Not in _rgraph" << endl;
	return;
    }

    _dirty = true;


    PoolItemSet adj = git->second;		//    adj = _rgraph[item];

    // order will be < 0
    Nodes::iterator nit( _nodes.find(item) );
    if (nit == _nodes.end()) {
	ERR << "Not in _nodes" << endl;
	return;
    }
    nit->second.order--;			//    _nodes[item].order--;
    _installed.insert (item);
    _toinstall.erase (item);

    for (PoolItemSet::iterator it = adj.begin(); it != adj.end(); ++it)
    {
	nit = _nodes.find(*it);
	if (nit == _nodes.end()) {
	    ERR << "Not in _nodes:" << *it << endl;
	    continue;
	}
	NodeInfo& info = nit->second;		// _nodes[*it];
	info.order--;
	if (info.order < 0)
	{
	    DBG << "order of node " << (*it) << " is < 0" << endl;
	}
    }
}


void
InstallOrder::setInstalled( const PoolItemList & rl )
{
    for (PoolItemList::const_iterator it = rl.begin(); it != rl.end(); ++it)
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
    DBG << "startrdfs run #" << _numrun << endl;

    // initialize all nodes
    for (PoolItemSet::iterator it = _toinstall.begin(); it != _toinstall.end(); ++it)
    {
	PoolItem_Ref item = *it;
	_nodes.insert( Nodes::value_type( item, NodeInfo (item) ) );
	_rgraph[item] = PoolItemSet();
	_graph[item] = PoolItemSet();
    }

    // visit all nodes
    for (PoolItemSet::iterator it = _toinstall.begin(); it != _toinstall.end(); ++it)
    {
	const PoolItem_Ref item = *it;
	Nodes::const_iterator nit( _nodes.find(item) );
	if (nit == _nodes.end()) {
	    ERR << "Can't find " << item << " in _nodes" << endl;
	    continue;
	}
	if (nit->second.visited == false)
	{
	    DBG << "start recursion on " << item << endl;
	    rdfsvisit (item);
	}
    }

    _dirty = false;
}




struct CollectProviders : public resfilter::OnCapMatchCallbackFunctor
{
    const PoolItem_Ref requestor;
    PoolItemSet & tovisit;   
    PoolItemSet & toinstall;
    PoolItemSet & installed;

    CollectProviders (const PoolItem_Ref pi, PoolItemSet & tv, PoolItemSet & ti, PoolItemSet i)
	: requestor (pi)
	, tovisit (tv)
	, toinstall (ti)
	, installed (i)
    { }


    bool operator()( PoolItem_Ref provider, const Capability & match )
    {
	// item provides cap which matches a requirement from info->requestor
	//   this function gets _all_ providers and filter out those which are
	//   either installed or in our toinstall input list
	//

	if ((provider.resolvable() != requestor.resolvable())		// resolvable could provide its own requirement
	    && (!provider.status().staysInstalled())			// only visit if provider is not already installed
	    && (toinstall.find(provider) != toinstall.end()		// only look at resolvables
		|| installed.find(provider) != installed.end())) {	//   we are currently considering anyways
	    DBG << "tovisit " << provider << endl;
	    tovisit.insert (provider);
	}

	return true;
    }

};


void
InstallOrder::rdfsvisit (const PoolItem_Ref  item)
{
    typedef list<Capability> CapList;
    CapList requires;

    DBG << "InstallOrder::rdfsvisit, visiting " << item << endl;

    Nodes::iterator nit( _nodes.find( item ) );
    if (nit == _nodes.end()) {
	ERR << "Can't find " << item << " in _nodes" << endl;
	return;
    }
    NodeInfo& nodeinfo = nit->second;

    nodeinfo.visited = true;
    nodeinfo.begintime = _rdfstime;
    _rdfstime++;

    // put prerequires first and requires last on list to ensure
    // that prerequires are processed first

    for (CapSet::const_iterator it = item->dep (Dep::PREREQUIRES).begin(); it != item->dep (Dep::PREREQUIRES).end(); ++it)
    {
	const Capability cap = *it;
	requires.push_back(cap);
    }

    for (CapSet::const_iterator it = item->dep (Dep::REQUIRES).begin(); it != item->dep (Dep::REQUIRES).end(); ++it)
    {
	const Capability cap = *it;
	requires.push_back(cap);
    }

    for (CapList::const_iterator iter = requires.begin(); iter != requires.end(); ++iter)
    {
	const Capability requirement = *iter;
	DBG << "check requirement " << requirement << " of " << item << endl;
	PoolItemSet tovisit;

	CollectProviders info ( item, tovisit, _toinstall, _installed );


	// _world->foreachProvidingResItem (requirement, collect_providers, &info);
#if 0
	Dep dep (Dep::PROVIDES);
	invokeOnEach( _pool.byCapabilityIndexBegin( requirement.index(), dep ),
		      _pool.byCapabilityIndexEnd( requirement.index(), dep ),
		      resfilter::callOnCapMatchIn( dep, requirement, functor::functorRef<bool,PoolItem,Capability>(info) ) );
#endif
	ResPool::const_indexiterator pend = _pool.providesend(requirement.index());
	for (ResPool::const_indexiterator it = _pool.providesbegin(requirement.index()); it != pend; ++it) {
	    if (it->second.second->arch() == Arch_src)
		continue;
	    if (requirement.matches (it->second.first) == CapMatch::yes) {
		if (!info( it->second.second, it->second.first))
		    break;
	    }
	}

	for (PoolItemSet::iterator it = tovisit.begin(); it != tovisit.end(); ++it)
	{
	    const PoolItem_Ref must_visit = *it;
	    nit = _nodes.find( must_visit );
	    if (nit == _nodes.end()) {
		ERR << "Can't find " << must_visit << " in _nodes" << endl;
		continue;
	    }
	    if (nit->second.visited == false)
	    {
		nodeinfo.order++;
		_rgraph[must_visit].insert (item);
		_graph[item].insert (must_visit);
		rdfsvisit(must_visit);
	    }
	    else if (nit->second.endtime == 0)
	    {
		if (must_visit != item)
		{
		    ERR << "*************************************************************" << endl;
		    ERR << "** dependency loop: " << item << " -> " << must_visit << endl;
		    ERR << "*************************************************************" << endl;
		}
	    }
	    else
	    {
		// filter multiple depends on same item (cosmetic)
		PoolItemSet & lrg = _rgraph[must_visit];
		if (lrg.find(item) == lrg.end())
		{
		    nodeinfo.order++;
		    lrg.insert(item);

		    PoolItemSet & lg = _graph[item];
		    if (lg.find (must_visit) == lg.end())
			lg.insert (must_visit);
		}
	    }
	}
    }
    DBG << "_topsorted.push_back(" << item << ")" << endl;
    _topsorted.push_back(item);
    nit = _nodes.find( item );
    if (nit == _nodes.end())  {
	ERR << "Can't find " << item << " in nodes" << endl;
    }
    else {
	nit->second.endtime = _rdfstime;
    }
    _rdfstime++;

    DBG << item << " done" << endl;
}


//-----------------------------------------------------------------------------

const PoolItemList
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
