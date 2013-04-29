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
#include "zypp/base/LogTools.h"
#include "zypp/base/Iterator.h"
#include "zypp/base/Algorithm.h"

#include "zypp/solver/detail/SATResolver.h"

#include "zypp/ResFilters.h"
#include "zypp/ResStatus.h"
#include "zypp/sat/Pool.h"
#include "zypp/sat/WhatObsoletes.h"

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

#define ITEMNAME(item) (item)->name()
//-----------------------------------------------------------------------------

InstallOrder::InstallOrder( const ResPool & pool, const PoolItemSet & toinstall, const PoolItemSet & installed )
    : _pool( pool )
    , _toinstall( toinstall )
    , _installed( installed )
    , _dirty (true)
    , _numrun (0)
{
    _DEBUG("InstallOrder::InstallOrder(_toinstall " << _toinstall.size() << " items, _installed " << _installed.size() << " items)");
}


//-----------------------------------------------------------------------------

void
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
	for (PoolItemList::const_iterator scit = gcit->second.begin(); scit != gcit->second.end(); ++scit)
	{
	    os << "\"" << name << "\" -> \"" << (*scit)->name() << "\"" << endl;
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
	if (it->second.order == 0
	    && it->second.item)			// the default Nodes constructor leaves this empty
	{
	    XXX << "InstallOrder::computeNextSet found " << ITEMNAME(it->second.item) << endl;

	    newlist.push_back(it->second.item);
	}
    }

    return newlist;
}


// decrease order of every adjacent node
void
InstallOrder::setInstalled(PoolItem item )
{
    _dirty = true;

    PoolItemList adj = _rgraph[item];

    XXX << "InstallOrder::setInstalled " << ITEMNAME(item) << endl;

    // order will be < 0
    _nodes[item].order--;
    _installed.insert( item );
    _toinstall.erase( item );

    for (PoolItemList::iterator it = adj.begin(); it != adj.end(); ++it)
    {
	NodeInfo& info = _nodes[*it];
	info.order--;
	if (info.order < 0)
	{
	    WAR << "order of node " << (*it) << " is < 0" << endl;
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

bool
InstallOrder::doesProvide( const Capability requirement, PoolItem item ) const
{
    Capabilities::const_iterator pend = item->dep( Dep::PROVIDES ).end();
    for( Capabilities::const_iterator pit = item->dep( Dep::PROVIDES ).begin(); pit != pend; ++pit) {
	if( pit->matches( requirement ) == CapMatch::yes ) {
	    return true;
	}
    }
    return false;
}


PoolItem
InstallOrder::findProviderInSet( const Capability requirement, const PoolItemSet & candidates ) const
{
    for( PoolItemSet::const_iterator citer = candidates.begin(); citer != candidates.end(); citer++) {
	if( doesProvide( requirement, *citer ) ) {
	    return *citer;
	}
    }

    return PoolItem();
}

//-----------------------------------------------------------------------------


void
InstallOrder::rdfsvisit (const PoolItem item)
{
    typedef list<Capability> CapList;
    CapList requires;

    XXX << "InstallOrder::rdfsvisit, visiting " << ITEMNAME(item) << endl;

    NodeInfo& nodeinfo = _nodes[item];

    nodeinfo.visited = true;
    nodeinfo.begintime = _rdfstime;
    _rdfstime++;

    // items prereq
    CapabilitySet prq( item->dep(Dep::PREREQUIRES).begin(), item->dep(Dep::PREREQUIRES).end() );
    // an installed items prereq (in case they are reqired for uninstall scripts)
    ui::Selectable::Ptr sel( ui::Selectable::get( item ) );
    for_( it, sel->installedBegin(), sel->installedEnd() )
    {
      Capabilities p( it->satSolvable().prerequires() );
      prq.insert( p.begin(), p.end() );
    }
    // any obsoleted items prereq (in case they are reqired for uninstall scripts)
    sat::WhatObsoletes obs( item );
    for_( it, obs.begin(), obs.end() )
    {
      Capabilities p( it->prerequires() );
      prq.insert( p.begin(), p.end() );
    }
    // put prerequires first and requires last on list to ensure
    // that prerequires are processed first
    for (CapabilitySet::const_iterator it = prq.begin(); it != prq.end(); ++it)
    {
	requires.push_back(*it);
    }

    // Product requirements are ignored to assert Product gets installed
    // as early as possible. Some stuff depends on it (e.g. registration).
    if ( ! isKind<Product>( item.resolvable() ) )
      {
        for (Capabilities::const_iterator it = item->dep (Dep::REQUIRES).begin(); it != item->dep (Dep::REQUIRES).end(); ++it)
          {
            requires.push_back(*it);
          }
      }

    for (CapList::const_iterator iter = requires.begin(); iter != requires.end(); ++iter)
    {
        bool goBack = false;
	const Capability requirement = *iter;
        PoolItemList providers;

	XXX << "check requirement " << requirement << " of " << ITEMNAME(item) << endl;
        SATResolver satResolver(_pool, sat::Pool::instance().get());
	PoolItemList tovisit;
        sat::WhatProvides possibleProviders(requirement);

	// first, look in _installed
        for_( iter, possibleProviders.begin(), possibleProviders.end() ) {
            PoolItem provider = ResPool::instance().find( *iter );
            if ( provider == item )
            {
              goBack = true;
              break;
            }
            if (_installed.find( provider ) != _installed.end())	// and is not installed
            {
                XXX << "tovisit " << ITEMNAME(provider) << endl;
                providers.push_back (provider);
            }
        }

        if ( goBack )
          continue;

	// if not found in _installed, look in _toinstall

	if (providers.empty()) {
            for_( iter, possibleProviders.begin(), possibleProviders.end() ) {
                PoolItem provider = ResPool::instance().find( *iter );
                if ((provider.resolvable() != item.resolvable())	        // resolvable could provide its own requirement
                    && (_toinstall.find( provider ) != _toinstall.end()))	// and is not to be installed
                {
                    XXX << "tovisit " << ITEMNAME(provider) << endl;
                    tovisit.push_back (provider);
                }
            }
	}

	for (PoolItemList::iterator it = tovisit.begin(); it != tovisit.end(); ++it)
	{
	    const PoolItem must_visit = *it;
	    if (_nodes[must_visit].visited == false)
	    {
		nodeinfo.order++;
		_rgraph[must_visit].push_back( item );
		_graph[item].push_back( must_visit );
		rdfsvisit( must_visit );
	    }
	    else if (_nodes[must_visit].endtime == 0)
	    {
		if (must_visit != item)
		{
		  // log only the 1st occurrence.
		  std::string lstr( ITEMNAME(item) );
		  lstr += " -> ";
		  lstr += ITEMNAME(must_visit);
		  if ( _logset.insert( lstr ).second )
		  {
		    WAR << "** dependency loop: " << lstr << endl;
		  }
		}
	    }
	    else
	    {
		// filter multiple depends on same item (cosmetic)
		PoolItemList & lrg = _rgraph[must_visit];
		if( find( lrg.begin(), lrg.end(), item) == lrg.end() )
		{
		    nodeinfo.order++;
		    lrg.push_back( item );

		    PoolItemList & lg = _graph[item];
		    if( find( lg.begin(), lg.end(), must_visit ) == lg.end() )
			lg.push_back( must_visit );
		}
	    }
	}
    }
    _topsorted.push_back(item);
    _nodes[item].endtime = _rdfstime;
    _rdfstime++;

    XXX << ITEMNAME(item) << " done" << endl;
}


void
InstallOrder::startrdfs()
{
    _nodes.clear();
    _rgraph.clear();
    _graph.clear();

    _rdfstime = 1;

    _topsorted.clear();

    _numrun++;
    XXX << "run #" << _numrun << endl;

    // initialize all nodes
    for (PoolItemSet::iterator it = _toinstall.begin(); it != _toinstall.end(); ++it)
    {
	PoolItem item = *it;
	_nodes[item] = NodeInfo (item);
	_rgraph[item] = PoolItemList();
	_graph[item] = PoolItemList();
    }

    // visit all nodes
    for (PoolItemSet::iterator it = _toinstall.begin(); it != _toinstall.end(); ++it)
    {
	const PoolItem item = *it;
	if (_nodes[item].visited == false)
	{
	    XXX << "start recursion on " << ITEMNAME(item) << endl;
	    rdfsvisit (item);
	}
    }

    _dirty = false;
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
