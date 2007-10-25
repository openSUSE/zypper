/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* SATResolver.cc
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

#include "zypp/solver/detail/Helper.h"
#include "zypp/base/String.h"
#include "zypp/CapSet.h"
#include "zypp/ResStatus.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Algorithm.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"
#include "zypp/sat/SATResolver.h"

extern "C" {
#include "satsolver/source_solv.h"
#include "satsolver/poolarch.h"
}

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace zypp_detail
  { /////////////////////////////////////////////////////////////////
    Arch defaultArchitecture();
    /////////////////////////////////////////////////////////////////
  } // namespace zypp_detail
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

using namespace std;

IMPL_PTR_TYPE(SATResolver);

static const unsigned MAX_SECOND_RUNS( 3 );
static const unsigned MAX_VALID_SOLUTIONS( 10 );
static const unsigned TIMOUT_SECOND_RUN( 30 );

static PoolItemSet triggeredSolution;   // only the latest state of an item is interesting
                                        // for the pool. Documents already inserted items.

//---------------------------------------------------------------------------


std::ostream &
SATResolver::dumpOn( std::ostream & os ) const
{
    return os << "<resolver/>";
}


//---------------------------------------------------------------------------

SATResolver::SATResolver (const ResPool & pool, Pool *SATPool)
    : _pool (pool)
    , _SATPool (SATPool) 
    , _timeout_seconds (0)
    , _maxSolverPasses (0)
    , _testing (false)
    , _valid_solution_count (0)
    , _timed_out (false)
    , _architecture( zypp_detail::defaultArchitecture() )

{

}


SATResolver::~SATResolver()
{
}

//---------------------------------------------------------------------------

ResPool
SATResolver::pool (void) const
{
    return _pool;
}


void
SATResolver::addPoolItemToInstall (PoolItem_Ref item)
{
    bool found = false;
    for (PoolItemList::const_iterator iter = _items_to_remove.begin();
	 iter != _items_to_remove.end(); iter++) {
	if (*iter == item) {
	    _items_to_remove.remove(*iter);
	    found = true;
	    break;
	}
    }
    if (!found) {
	_items_to_install.push_back (item);
	_items_to_install.unique ();
    }
}


void
SATResolver::addPoolItemsToInstallFromList (PoolItemList & rl)
{
    for (PoolItemList::const_iterator iter = rl.begin(); iter != rl.end(); iter++) {
	addPoolItemToInstall (*iter);
    }
}


void
SATResolver::addPoolItemToRemove (PoolItem_Ref item)
{
    bool found = false;
    for (PoolItemList::const_iterator iter = _items_to_install.begin();
	 iter != _items_to_install.end(); iter++) {
	if (*iter == item) {
	    _items_to_install.remove(*iter);
	    found = true;
	    break;
	}
    }
    if (!found) {
	_items_to_remove.push_back (item);
	_items_to_remove.unique ();
    }
}


void
SATResolver::addPoolItemsToRemoveFromList (PoolItemList & rl)
{
    for (PoolItemList::const_iterator iter = rl.begin(); iter != rl.end(); iter++) {
	addPoolItemToRemove (*iter);
    }
}

void
SATResolver::addPoolItemToLockUninstalled (PoolItem_Ref item)
{
    _items_to_lockUninstalled.push_back (item);
    _items_to_lockUninstalled.unique ();
}

void
SATResolver::addPoolItemToKepp (PoolItem_Ref item)
{
    _items_to_keep.push_back (item);
    _items_to_keep.unique ();	        
}


//---------------------------------------------------------------------------

// copy marked item from solution back to pool
// if data != NULL, set as APPL_LOW (from establishPool())

static void
solution_to_pool (PoolItem_Ref item, const ResStatus & status, const ResStatus::TransactByValue causer)
{
    if (triggeredSolution.find(item) != triggeredSolution.end()) {
        _XDEBUG("solution_to_pool(" << item << ") is already in the pool --> skip");
        return;
    }

    triggeredSolution.insert(item);

    // resetting transaction only
    item.status().resetTransact (causer);

    bool r;

    if (status.isToBeInstalled()) {
	r = item.status().setToBeInstalled (causer);
	_XDEBUG("solution_to_pool(" << item << ", " << status << ") install !" << r);
    }
    else if (status.isToBeUninstalledDueToUpgrade()) {
	r = item.status().setToBeUninstalledDueToUpgrade (causer);
	_XDEBUG("solution_to_pool(" << item << ", " << status << ") upgrade !" << r);
    }
    else if (status.isToBeUninstalled()) {
	r = item.status().setToBeUninstalled (causer);
	_XDEBUG("solution_to_pool(" << item << ", " << status << ") remove !" << r);
    }
    else if (status.isIncomplete()
	     || status.isNeeded()) {
	r = item.status().setIncomplete();
	_XDEBUG("solution_to_pool(" << item << ", " << status << ") incomplete !" << r);
    }
    else if (status.isUnneeded()) {
	r = item.status().setUnneeded();
	_XDEBUG("solution_to_pool(" << item << ", " << status << ") unneeded !" << r);
    }
    else if (status.isSatisfied()) {
	r = item.status().setSatisfied();
	_XDEBUG("solution_to_pool(" << item << ", " << status << ") satisfied !" << r);
    } else {
	_XDEBUG("solution_to_pool(" << item << ", " << status << ") unchanged !");
    }
    return;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// resolvePool
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Helper functions for the ZYPP-Pool
//----------------------------------------------------------------------------


Resolvable::Kind
string2kind (const std::string & str)
{
    Resolvable::Kind kind = ResTraits<zypp::Package>::kind;
    if (!str.empty()) {
	if (str == "package") {
	    // empty
	}
	else if (str == "patch") {
	    kind = ResTraits<zypp::Patch>::kind;
	}
	else if (str == "atom") {
	    kind = ResTraits<zypp::Atom>::kind;
	}
	else if (str == "pattern") {
	    kind = ResTraits<zypp::Pattern>::kind;
	}
	else if (str == "selection") {
	    kind = ResTraits<zypp::Selection>::kind;
	}
	else if (str == "script") {
	    kind = ResTraits<zypp::Script>::kind;
	}
	else if (str == "message") {
	    kind = ResTraits<zypp::Message>::kind;
	}
	else if (str == "product") {
	    kind = ResTraits<zypp::Product>::kind;
	}
	else if (str == "language") {
	    kind = ResTraits<zypp::Language>::kind;
	}
	else {
	    ERR << "get_poolItem unknown kind '" << str << "'" << endl;
	}
    }
    return kind;
}


struct FindPackage : public resfilter::ResObjectFilterFunctor
{
    PoolItem_Ref poolItem;
    Resolvable::Kind kind;
    bool edition_set;
    Edition edition;
    bool arch_set;
    Arch arch;

    FindPackage (Resolvable::Kind k, const string & e, const string & a)
	: kind (k)
	, edition_set( !e.empty() )
	, edition( e )
	, arch_set( !a.empty() )
	, arch( a )
    {
    }

    bool operator()( PoolItem_Ref p)
    {
	if (arch_set && arch != p->arch()) {				// if arch requested, force this arch
	    return true;
	}

	if (edition_set) {
	    if (p->edition().compare( edition ) == 0) {			// if edition requested, force this edition
		poolItem = p;
		return false;
	    }
	    return true;
	}

	if (!poolItem							// none yet
	    || (poolItem->arch().compare( p->arch() ) < 0)		// new has better arch
	    || (poolItem->edition().compare( p->edition() ) < 0))	// new has better edition
	{
	    poolItem = p;
	}
	return true;
    }
};


static PoolItem_Ref
get_poolItem (const ResPool & pool, const string & source_alias, const string & package_name, const string & kind_name = "", const string & edition = "",  const string & arch = "")
{
    PoolItem_Ref poolItem;
    Resolvable::Kind kind = string2kind (kind_name);

    try {
	FindPackage info (kind, edition, arch);

	invokeOnEach( pool.byNameBegin( package_name ),
		      pool.byNameEnd( package_name ),
		      functor::chain( resfilter::ByRepository(source_alias), resfilter::ByKind (kind) ),
		      functor::functorRef<bool,PoolItem> (info) );

	poolItem = info.poolItem;
        if (!poolItem) {
            // try to find the resolvable over all channel. This is useful for e.g. languages
            invokeOnEach( pool.byNameBegin( package_name ),
                          pool.byNameEnd( package_name ),
                          resfilter::ByKind (kind),
                          functor::functorRef<bool,PoolItem> (info) );
            poolItem = info.poolItem;
        }
    }
    catch (Exception & excpt_r) {
	ZYPP_CAUGHT (excpt_r);
	ERR << "Can't find kind[" << kind_name << "]:'" << package_name << "': source '" << source_alias << "' not defined" << endl;
	if (kind_name.empty())
	    ERR << "Please specify kind=\"...\" in the <install.../> request." << endl;
	return poolItem;
    }

    if (!poolItem) {
	ERR << "Can't find kind: " << kind << ":'" << package_name << "' in source '" << source_alias << "': no such name/kind" << endl;
    }

    return poolItem;
}

struct CollectTransact : public resfilter::PoolItemFilterFunctor
{
    SATResolver & resolver;

    CollectTransact (SATResolver & r)
	: resolver (r)
    { }

    bool operator()( PoolItem_Ref item )		// only transacts() items go here
    {
	ResStatus status = item.status();
	_XDEBUG( "CollectTransact(" << item << ")" );
	bool by_solver = (status.isBySolver() || status.isByApplLow());

	if (by_solver) {
	    _XDEBUG("Resetting " << item );
	    item.status().resetTransact( ResStatus::APPL_LOW );// clear any solver/establish transactions
	    return true;				// back out here, dont re-queue former solver result
	}

	if (status.isToBeInstalled()) {
	    resolver.addPoolItemToInstall(item);	// -> install!
	}
	if (status.isToBeUninstalled()) {
	    resolver.addPoolItemToRemove(item);		// -> remove !
	}
	if (status.isIncomplete()) {			// incomplete (re-install needed)
	    PoolItem_Ref reinstall = Helper::findReinstallItem (resolver.pool(), item);
	    if (reinstall) {
		MIL << "Reinstall " << reinstall << " for incomplete " << item << endl;
		resolver.addPoolItemToInstall(reinstall);	// -> install!
	    }
	    else {
		WAR << "Can't find " << item << " for re-installation" << endl;
	    }
	}

        if (status.isLocked()
            && status.isUninstalled()) {
            // This item could be selected by solver in a former run. Now it
            // is locked. So we will have to evaluate a new solver run.
            resolver.addPoolItemToLockUninstalled (item);
        }

        if (status.isKept()
            && !by_solver) {
	    // collecting all keep states
	    resolver.addPoolItemToKepp (item);
	}

	return true;
    }
};

//------------------------------------------------------------------------------------------------------------
// Helper functions for the SAT-Pool
//------------------------------------------------------------------------------------------------------------

// find solvable id by name and source
//   If source != NULL, find there
//   else find in pool (available packages)
//

static Id
select_solvable( Pool *pool,
		 const PoolItem_Ref item)
{
    Id id, archid;
    int end;

    string packageName = str::form (_("%s:%s"),
				    item->kind().asString().c_str(),
				    item->name().c_str()
				    );	

    string repoName = item->repository().info().alias();

    // Searching concerning source
    Source *source = NULL;
    for (int i = 0; i < pool->nsources; i++)
    {
	string compName(source_name(pool->sources[i]));
	if (repoName == compName) {
	    source = pool->sources[i];
	    break;
	}
    }
  
    id = str2id( pool, packageName.c_str(), 0 );
    if (id == ID_NULL) {
	return id;
    }
    archid = ID_NULL;
    archid = str2id( pool, item->arch().asString().c_str(), 0 );
    if (archid == ID_NULL) {
	return ID_NULL;
    }
    
    end = source ? source->start + source->nsolvables : pool->nsolvables;
    for (int i = source ? source->start : 1 ; i < end; i++)
    {
	if (archid && pool->solvables[i].arch != archid)
	    continue;
	if (pool->solvables[i].name == id)
	    return i;
    }

    return ID_NULL;
}



//  This function loops over the pool and grabs
//  all item.status().transacts() and item.status().byUser()
//  It clears all previous bySolver() states also
//
//  Every toBeInstalled is passed to zypp::solver:detail::Resolver.addPoolItemToInstall()
//  Every toBeUninstalled is passed to zypp::solver:detail::Resolver.addPoolItemToRemove()
//
//  Solver results must be written back to the pool.


bool
SATResolver::resolvePool()
{
    CollectTransact info (*this);

    MIL << "SATResolver::resolvePool()" << endl;

    invokeOnEach ( _pool.begin(), _pool.end(),
		   resfilter::ByTransact( ),			// collect transacts from Pool to resolver queue
		   functor::functorRef<bool,PoolItem>(info) );

    invokeOnEach ( _pool.begin(), _pool.end(),
                   resfilter::ByLock( ),                        // collect locks from Pool to resolver queue
                   functor::functorRef<bool,PoolItem>(info) );

    invokeOnEach ( _pool.begin(), _pool.end(),
                   resfilter::ByKeep( ),                        // collect keeps from Pool to resolver queue
                   functor::functorRef<bool,PoolItem>(info) );    

    Queue trials;    

    for (PoolItemList::const_iterator iter = _items_to_install.begin(); iter != _items_to_install.end(); iter++) {
	PoolItem_Ref r = *iter;

	Id id = select_solvable( _SATPool, *iter );
	if (id == ID_NULL) {
	    ERR << "Install: " << *iter << " not found" << endl;
	}
	queuepush( &(trials), SOLVER_INSTALL_SOLVABLE );
        queuepush( &(trials), id );
    }

    for (PoolItemList::const_iterator iter = _items_to_remove.begin(); iter != _items_to_remove.end(); iter++) {
	string packageName = str::form (_("%s:%s"),
					iter->resolvable()->kind().asString().c_str(),
					iter->resolvable()->name().c_str()
					);	
	Id id = str2id( _SATPool, packageName.c_str(), 1 );
	queuepush( &(trials), SOLVER_ERASE_SOLVABLE_NAME );
	queuepush( &(trials), id);
    }

    // Searching concerning system source
    Source *systemSource = NULL;
    for (int i = 0; i < _SATPool->nsources; i++)
    {
	string compName(source_name(_SATPool->sources[i]));
	if (compName == "system") {
	    systemSource = _SATPool->sources[i];
	    break;
	}
    }
    
    Solver *solv = solver_create( _SATPool, systemSource );
    solv->fixsystem = false;
    solv->updatesystem = false;
    solv->allowdowngrade = false;
    solv->allowuninstall = false;
    solv->noupdateprovide = false;
    _SATPool->verbose = true;

    // Solve !
    solve( solv, &(trials) );

    // copying solution back to zypp pool
    //-----------------------------------------
    Id p;
    Solvable *s;
  
    /* solvables to be erased */
    for (int i = solv->system->start; i < solv->system->start + solv->system->nsolvables; i++)
    {
      if (solv->decisionmap[i] > 0)
	continue;

      // getting source
      s = _SATPool->solvables + i;
      Source *source = pool_source(_SATPool, s);

      PoolItem_Ref poolItem;
      string kindName(id2str(_SATPool, s->name));
      std::vector<std::string> nameVector;
      // expect "<kind>::<name>"
      unsigned count = str::split( kindName, std::back_inserter(nameVector), ":" );
      
      if ( count == 3 ) {
	  PoolItem_Ref poolItem = get_poolItem (_pool,
						source ? string(source_name(source)) : "",
						nameVector[2], // name
						nameVector[0], // kind,
						string(id2str(_SATPool, s->evr)),
						string(id2str(_SATPool, s->arch)));
	  if (poolItem) {
	      ResStatus status;
	      status.isToBeUninstalled();
	      solution_to_pool (poolItem, status, ResStatus::SOLVER);
	  } else {
	      ERR << kindName << " not found in ZYPP pool." << endl;
	  }
      } else {
	  ERR << "Cannot split " << kindName << " correctly." << endl;
      }
    }

    /*  solvables to be installed */
    for (int i = 0; i < solv->decisionq.count; i++)
    {
      p = solv->decisionq.elements[i];
      if (p < 0)
	continue;
      if (p >= solv->system->start && p < solv->system->start + solv->system->nsolvables)
	continue;

      // getting source
      s = _SATPool->solvables + p;
      Source *source = pool_source(_SATPool, s);

      PoolItem_Ref poolItem;
      string kindName(id2str(_SATPool, s->name));
      std::vector<std::string> nameVector;
      // expect "<kind>::<name>"
      unsigned count = str::split( kindName, std::back_inserter(nameVector), ":" );
      
      if ( count == 3 ) {
	  PoolItem_Ref poolItem = get_poolItem (_pool,
						source ? string(source_name(source)) : "",
						nameVector[2], // name
						nameVector[0], // kind,
						string(id2str(_SATPool, s->evr)),
						string(id2str(_SATPool, s->arch)));
	  if (poolItem) {
	      ResStatus status;
	      status.isToBeInstalled();
	      solution_to_pool (poolItem, status, ResStatus::SOLVER);
	  } else {
	      ERR << kindName << " not found in ZYPP pool." << endl;
	  }
      } else {
	  ERR << "Cannot split " << kindName << " correctly." << endl;
      }
    }

    // clean up
    solver_free(solv);
    queuefree( &(trials) );
    
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

