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
 * along with this program; if not, write the Free Software
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
#include "zypp/sat/Solvable.h"
#include "zypp/solver/detail/ProblemSolutionCombi.h"

extern "C" {
#include "satsolver/repo_solv.h"
#include "satsolver/poolarch.h"
#include "satsolver/evr.h"
#include "satsolver/poolvendor.h"
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
    , solv(NULL)
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
	    ERR << "string2kind unknown kind '" << str << "'" << endl;
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
get_poolItem (const ResPool & pool, const string & repo_alias, const string & package_name, const string & kind_name = "", const string & edition = "",  const string & arch = "")
{
    PoolItem_Ref poolItem;
    Resolvable::Kind kind = string2kind (kind_name);

    try {
	FindPackage info (kind, edition, arch);

	invokeOnEach( pool.byNameBegin( package_name ),
		      pool.byNameEnd( package_name ),
		      functor::chain( resfilter::ByRepository(repo_alias), resfilter::ByKind (kind) ),
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
	ERR << "Can't find kind[" << kind_name << "]:'" << package_name << "': repo '" << repo_alias << "' not defined" << endl;
	if (kind_name.empty())
	    ERR << "Please specify kind=\"...\" in the <install.../> request." << endl;
	return poolItem;
    }

    if (!poolItem) {
	ERR << "Can't find kind: " << kind << ":'" << package_name << "' in repo '" << repo_alias << "': no such name/kind" << endl;
    }

    return poolItem;
}

//------------------------------------------------------------------------------------------------------------
//  This function loops over the pool and grabs
//  all item.status().transacts() and item.status().byUser()
//  It clears all previous bySolver() states also
//
//  Every toBeInstalled is passed to zypp::solver:detail::Resolver.addPoolItemToInstall()
//  Every toBeUninstalled is passed to zypp::solver:detail::Resolver.addPoolItemToRemove()
//
//  Solver results must be written back to the pool.
//------------------------------------------------------------------------------------------------------------


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


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// solving.....
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


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

    for (PoolItemList::const_iterator iter = _items_to_install.begin(); iter != _items_to_install.end(); iter++) {
	PoolItem_Ref r = *iter;

	Id id = iter->satSolvable().id(); 
	if (id == ID_NULL) {
	    ERR << "Install: " << *iter << " not found" << endl;
	}
	queue_push( &(jobQueue), SOLVER_INSTALL_SOLVABLE );
        queue_push( &(jobQueue), id );
    }

    for (PoolItemList::const_iterator iter = _items_to_remove.begin(); iter != _items_to_remove.end(); iter++) {
	string packageName = str::form (_("%s:%s"),
					iter->resolvable()->kind().asString().c_str(),
					iter->resolvable()->name().c_str()
					);
	Id id = str2id( _SATPool, packageName.c_str(), 1 );
	queue_push( &(jobQueue), SOLVER_ERASE_SOLVABLE_NAME );
	queue_push( &(jobQueue), id);
    }

    // Searching concerning system repo
    Repo *systemRepo = NULL;
    for (int i = 0; i < _SATPool->nrepos; i++)
    {
	string compName(repo_name(_SATPool->repos[i]));
	if (compName == "system") {
	    systemRepo = _SATPool->repos[i];
	    break;
	}
    }

    solv = solver_create( _SATPool, systemRepo );
    solv->fixsystem = false;
    solv->updatesystem = false;
    solv->allowdowngrade = false;
    solv->allowuninstall = false;
    solv->noupdateprovide = false;
    _SATPool->verbose = true;

    // Solve !
    solver_solve( solv, &(jobQueue) );

    // copying solution back to zypp pool
    //-----------------------------------------
    Id p;
    Solvable *s;

    /* solvables to be erased */
    for (int i = solv->installed->start; i < solv->installed->start + solv->installed->nsolvables; i++)
    {
      if (solv->decisionmap[i] > 0)
	continue;

      // getting repo
      s = _SATPool->solvables + i;
      Repo *repo = s->repo;
      PoolItem_Ref poolItem;
      string kindName(id2str(_SATPool, s->name));
      std::vector<std::string> nameVector;

      // expect "<kind>::<name>"
      unsigned count = str::split( kindName, std::back_inserter(nameVector), ":" );

      if ( count == 3 ) {
	  PoolItem_Ref poolItem = get_poolItem (_pool,
						repo ? string(repo_name(repo)) : "",
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
      if (p >= solv->installed->start && p < solv->installed->start + solv->installed->nsolvables)
	continue;

      // getting repo
      s = _SATPool->solvables + p;
      Repo *repo = s->repo;

      PoolItem_Ref poolItem;
      string kindName(id2str(_SATPool, s->name));
      std::vector<std::string> nameVector;
      // expect "<kind>::<name>"
      unsigned count = str::split( kindName, std::back_inserter(nameVector), ":" );

      if ( count == 3 ) {
	  PoolItem_Ref poolItem = get_poolItem (_pool,
						repo ? string(repo_name(repo)) : "",
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
    solv = NULL;
    queue_free( &(jobQueue) );

    return true;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// error handling
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

string
conflictsString(Solver *solv, Solvable *s, Id pc)
{
    Pool *pool = solv->pool;
    Solvable *sc = pool->solvables + pc;
    Id p, *pp, con, *conp, obs, *obsp;
    string ret;

    if (s->conflicts) {
	conp = s->repo->idarraydata + s->conflicts;
	while ((con = *conp++) != 0) {
	    FOR_PROVIDES(p, pp, con) {
		if (p != pc)
		    continue;
		ret = str::form("packags %s conflicts with %s, which is provided by %s\n", solvable2str(pool, s), dep2str(pool, con), solvable2str(pool, sc));
	    }
	}
    }
    if (s->obsoletes && (!solv->installed || s->repo != solv->installed)) {
	obsp = s->repo->idarraydata + s->obsoletes;
	while ((obs = *obsp++) != 0) {
	    FOR_PROVIDES(p, pp, obs) {
		if (p != pc)
		    continue;
		ret += str::form("packags %s obsolets %s, which is provided by %s\n", solvable2str(pool, s), dep2str(pool, obs), solvable2str(pool, sc));
	    }
	}
    }
    return ret;
}


string
probleminfoString(Solver *solv, const Queue *job, Id problem)
{
  Pool *pool = solv->pool;
  Rule *r;
  Solvable *s;
  Id p, d, rn;
  Id idx = solv->problems.elements[problem - 1];
  string ret;

  rn = solv->learnt_pool.elements[idx];
  if (rn < 0) {
      p = rn;		/* fake a negative assertion rule */
      r = 0;
  } else {
      r = solv->rules + rn;
      p = r->p;
  }

  if (!r || r->w2 == 0) {
      Id req, *reqp, *dp;
      int count = 0;

      /* assertions */
      if (p == -SYSTEMSOLVABLE)	{
          Id ji, what;

	  /* we tried to deinstall the system solvable. must be a job. */
	  if (rn < solv->jobrules || rn >= solv->systemrules)
	      abort();
	  ji = solv->ruletojob.elements[rn - solv->jobrules];
	  what = job->elements[ji + 1];
	  switch (job->elements[ji]) {
	      case SOLVER_INSTALL_SOLVABLE_NAME:
		  ret = str::form ("no solvable exists with name %s\n", dep2str(pool, what));
		  break;
	      case SOLVER_INSTALL_SOLVABLE_PROVIDES:
		  ret = str::form ("no solvable provides %s\n", dep2str(pool, what));
		  break;
	      default:
		  ret = str::form ("unknown  job\n");
	  }
	  return ret;
      }
      if (p > 0 && solv->learnt_pool.elements[idx + 1] == -p) {
	  /* we conflicted with a direct rpm assertion */
	  /* print other rule */
	  p = -p;
	  rn = 0;
      }
      if (rn >= solv->jobrules) {
	  ret += "some job/system/learnt rule\n";
	  return ret;
      }
      if (p >= 0)
	  abort();
      /* negative assertion, i.e. package is not installable */
      s = pool->solvables + (-p);
      if (s->requires) {
	  reqp = s->repo->idarraydata + s->requires;
	  while ((req = *reqp++) != 0) {
	      if (req == SOLVABLE_PREREQMARKER)
		  continue;
	      dp = pool_whatprovides(pool, req);
	      if (*dp)
		  continue;
	      ret += str::form ("package %s requires %s, but no package provides it\n", solvable2str(pool, s), dep2str(pool, req));
	      count++;
	  }
      }
      if (!count)
	  ret += str::form ("package %s is not installable\n", solvable2str(pool, s));
      return ret;
  }

  if (rn >= solv->learntrules) {
      /* learnt rule, ignore for now */
      ret += str::form ("some learnt rule...\n");
//      printrule(solv, r);
      return ret;
  }
  if (rn >= solv->systemrules) {
      /* system rule, ignore for now */
      ret += str::form ("some system rule...\n");
//      printrule(solv, r);
      return ret;
  }
  if (rn >= solv->jobrules) {
      /* job rule, ignore for now */
      ret += str::form ("some job rule...\n");
//      printrule(solv, r);
      return ret;
  }
  /* only rpm rules left... */
  p = r->p;
  d = r->d;
  if (p >= 0)
    abort();
  if (d == 0 && r->w2 < 0) {
      Solvable *sp, *sd;
      d = r->w2;
      sp = pool->solvables + (-p);
      sd = pool->solvables + (-d);
      if (sp->name == sd->name) {
	  ret += str::form ("cannot install both %s and %s\n", solvable2str(pool, sp), solvable2str(pool, sd));
      } else {
	  ret += conflictsString (solv, pool->solvables + (-p), -d);
	  ret += conflictsString (solv, pool->solvables + (-d), -p);
      }
  } else {
      /* find requires of p that corresponds with our rule */
      Id req, *reqp, *dp;
      s = pool->solvables + (-p);
      reqp = s->repo->idarraydata + s->requires;
      while ((req = *reqp++) != 0) {
	  if (req == SOLVABLE_PREREQMARKER)
	      continue;
	  dp = pool_whatprovides(pool, req);
          if (d == 0) {
	      if (*dp == r->w2 && dp[1] == 0)
		  break;
	  }
	  else if (dp - pool->whatprovidesdata == d)
	      break;
      }
      if (!req) {
	  ret += str::form ("req not found\n");
      }
      ret += str::form ("package %s requires %s, but none of its providers can be installed\n", solvable2str(pool, s), dep2str(pool, req));
  }
  return ret;
}

ResolverProblemList
SATResolver::problems () const
{
    ResolverProblemList resolverProblems;
    if (solv && solv->problems.count) {
	Pool *pool = solv->pool;
	int pcnt;
	Id p, rp, what;
	Id problem, solution, element;
	Solvable *s, *sd;

	MIL << "Encountered problems! Here are the solutions:\n" << endl;
	pcnt = 1;
	problem = 0;
	while ((problem = solver_next_problem(solv, problem)) != 0) {
	    MIL << "Problem " <<  pcnt << ":" << endl;
	    MIL << "====================================" << endl;
	    string whatString = probleminfoString(solv, &jobQueue, problem);
	    ResolverProblem_Ptr resolverProblem = new ResolverProblem (whatString, "");
	    solution = 0;
	    while ((solution = solver_next_solution(solv, problem, solution)) != 0) {
		element = 0;
		ProblemSolutionCombi *problemSolution = new ProblemSolutionCombi(resolverProblem);
		while ((element = solver_next_solutionelement(solv, problem, solution, element, &p, &rp)) != 0) {
		    if (p == 0) {
			/* job, rp is index into job queue */
			what = jobQueue.elements[rp];
			switch (jobQueue.elements[rp-1])
			{
			    case SOLVER_INSTALL_SOLVABLE: {
				s = pool->solvables + what;
				std::vector<std::string> nameVector;
				string kindName(id2str(_SATPool, s->name));

				// expect "<kind>::<name>"
				unsigned count = str::split( kindName, std::back_inserter(nameVector), ":" );
				PoolItem_Ref poolItem;
				if (count >= 2) {
				    poolItem = get_poolItem (_pool,
							     s->repo ? string(repo_name(s->repo)) : "", //repo
							     nameVector[1], // name
							     nameVector[0], // kind,
							     string(id2str(_SATPool, s->evr)),
							     string(id2str(_SATPool, s->arch)));
				}
				if (poolItem) {
				    if (what >= solv->installed->start && what < solv->installed->start + solv->installed->nsolvables) {
					problemSolution->addSingleAction (poolItem, REMOVE);
					MIL << "- do not keep " << id2str(pool, s->name) << "-" <<  id2str(pool, s->evr) << "." <<  id2str(pool, s->arch) <<
					    "  installed" << endl;
				    } else {
					problemSolution->addSingleAction (poolItem, KEEP);
					MIL << "- do not install " << id2str(pool, s->name) << "-" <<  id2str(pool, s->evr) << "." <<  id2str(pool, s->arch) <<
					    endl;
				    }
				} else {
				    ERR << "SOLVER_INSTALL_SOLVABLE: No item found for " << id2str(pool, s->name) << "-"
					<<  id2str(pool, s->evr) << "." <<  id2str(pool, s->arch) << endl;
				}
			    }
				break;
			    case SOLVER_ERASE_SOLVABLE: {
				s = pool->solvables + what;
				std::vector<std::string> nameVector;
				string kindName(id2str(_SATPool, s->name));

				// expect "<kind>::<name>"
				unsigned count = str::split( kindName, std::back_inserter(nameVector), ":" );
				PoolItem_Ref poolItem;

				if (count >= 2) {
				    poolItem = get_poolItem (_pool,
							     s->repo ? string(repo_name(s->repo)) : "", //repo
							     nameVector[1], // name
							     nameVector[0], // kind,
							     string(id2str(_SATPool, s->evr)),
							     string(id2str(_SATPool, s->arch)));
				}
				if (poolItem) {
				    if (what >= solv->installed->start && what < solv->installed->start + solv->installed->nsolvables) {
					problemSolution->addSingleAction (poolItem, KEEP);
					MIL << "- do not deinstall " << id2str(pool, s->name) << "-" <<  id2str(pool, s->evr) << "." <<  id2str(pool, s->arch) <<
					    endl;
				    } else {
					problemSolution->addSingleAction (poolItem, INSTALL);
					MIL << "- do not forbid installation of " << id2str(pool, s->name) << "-" <<  id2str(pool, s->evr) << "."
					    << id2str(pool, s->arch) <<  endl;
				    }
				} else {
				    ERR << "SOLVER_ERASE_SOLVABLE: No item found for " << id2str(pool, s->name) << "-" <<  id2str(pool, s->evr) << "." <<
					id2str(pool, s->arch) << endl;
				}
			    }
				break;
			    case SOLVER_INSTALL_SOLVABLE_NAME:
				MIL << "- do not install "<<  id2str(pool, what) << endl;;
				ERR << "No valid solution available" << endl;
				break;
			    case SOLVER_ERASE_SOLVABLE_NAME:
				MIL << "- do not deinstall " << id2str(pool, what) << endl;
				ERR << "No valid solution available" << endl;
				break;
			    case SOLVER_INSTALL_SOLVABLE_PROVIDES:
				MIL << "- do not install a solvable providing " <<  dep2str(pool, what) << endl;
				ERR << "No valid solution available" << endl;
				break;
			    case SOLVER_ERASE_SOLVABLE_PROVIDES:
				MIL << "- do not deinstall all solvables providing " << dep2str(pool, what) << endl;
				ERR << "No valid solution available" << endl;
				break;
			    case SOLVER_INSTALL_SOLVABLE_UPDATE:
				s = pool->solvables + what;
				MIL << "- do not install most recent version of " << id2str(pool, s->name) << "-" <<  id2str(pool, s->evr)
				    << "." <<  id2str(pool, s->arch) << endl;
				ERR << "No valid solution available" << endl;
				break;
			    default:
				MIL << "- do something different" << endl;
				ERR << "No valid solution available" << endl;
				break;
			}
		    } else {
			/* policy, replace p with rp */
			s = pool->solvables + p;
			sd = rp ? pool->solvables + rp : 0;

			std::vector<std::string> nameVector;
			string kindNameFrom(id2str(_SATPool, s->name));
			// expect "<kind>::<name>"
			unsigned count = str::split( kindNameFrom, std::back_inserter(nameVector), ":" );
			PoolItem_Ref itemFrom;

			if (count >= 2) {
			    itemFrom = get_poolItem (_pool,
						     s->repo ? string(repo_name(s->repo)) : "", //repo
						     nameVector[1], // name
						     nameVector[0], // kind,
						     string(id2str(_SATPool, s->evr)),
						     string(id2str(_SATPool, s->arch)));
			}

			if (sd)
			{
			    int gotone = 0;

			    string kindNameTo(id2str(_SATPool, sd->name));
			    // expect "<kind>::<name>"
			    count = str::split( kindNameTo, std::back_inserter(nameVector), ":" );
			    PoolItem_Ref itemTo;
			    if (count >= 2) {
				itemTo = get_poolItem (_pool,
						       sd->repo ? string(repo_name(s->repo)) : "", //repo
						       nameVector[1], // name
						       nameVector[0], // kind,
						       string(id2str(_SATPool, sd->evr)),
						       string(id2str(_SATPool, sd->arch)));
			    }
			    if (itemFrom && itemTo) {
				problemSolution->addSingleAction (itemTo, INSTALL);
				problemSolution->addSingleAction (itemFrom, REMOVE);

				if (evrcmp(pool, sd->evr, s->evr) < 0)
				{
				    MIL << "- allow downgrade of " << id2str(pool, s->name) << "-" <<  id2str(pool, s->evr) << "." <<  id2str(pool, s->arch)
					<< " to "  << id2str(pool, sd->name) << "-" <<  id2str(pool, sd->evr) << "." <<  id2str(pool, sd->arch) << endl;
				    gotone = 1;
				}
				if (!solv->allowarchchange && s->name == sd->name )//&& archchanges(pool, sd, s))
				{
				    MIL << "- allow architecture change of " << id2str(pool, s->name) << "-" <<  id2str(pool, s->evr) << "." <<  id2str(pool, s->arch)
					<< " to "  << id2str(pool, sd->name) << "-" <<  id2str(pool, sd->evr) << "." <<  id2str(pool, sd->arch) << endl;
				    gotone = 1;
				}
				if (!solv->allowvendorchange && s->name == sd->name && s->vendor != sd->vendor && pool_vendor2mask(pool, s->vendor) && (pool_vendor2mask(pool, s->vendor) & pool_vendor2mask(pool, sd->vendor)) == 0)
				{
				    MIL << "- allow vendor change of " << id2str(pool, s->vendor) << id2str(pool, s->name) << "-" <<  id2str(pool, s->evr) << "." <<  id2str(pool, s->arch)
					<< " to " << string(sd->vendor ?  id2str(pool, sd->vendor) : " (no vendor) ") << id2str(pool, sd->name) << "-" <<  id2str(pool, sd->evr) << "." <<  id2str(pool, sd->arch) << endl;
				    gotone = 1;
				}
				if (!gotone) {
				    MIL << "- allow replacement of " << id2str(pool, s->name) << "-" <<  id2str(pool, s->evr) << "." <<  id2str(pool, s->arch)
					<< " to "  << id2str(pool, sd->name) << "-" <<  id2str(pool, sd->evr) << "." <<  id2str(pool, sd->arch) << endl;
				}
			    } else {
				ERR << id2str(pool, s->name) << "-" <<  id2str(pool, s->evr) << "." <<  id2str(pool, s->arch)
				    << " or "  << id2str(pool, sd->name) << "-" <<  id2str(pool, sd->evr) << "." <<  id2str(pool, sd->arch) << " not found" << endl;
			    }
			}
			else
			{
			    if (itemFrom) {
				problemSolution->addSingleAction (itemFrom, REMOVE);
				MIL << "- allow replacement of " << id2str(pool, s->name) << "-" <<  id2str(pool, s->evr) << "." <<  id2str(pool, s->arch) << endl;
			    }
			}
		    }
		}
		resolverProblem->addSolution (problemSolution);
		MIL << "------------------------------------" << endl;
	    }
	    // save problem
	    resolverProblems.push_back (resolverProblem);
	}
    }
    return resolverProblems;
}

void
SATResolver::applySolutions (const ProblemSolutionList & solutions)
{
    for (ProblemSolutionList::const_iterator iter = solutions.begin();
	 iter != solutions.end(); ++iter) {
	ProblemSolution_Ptr solution = *iter;
	Resolver dummyResolver(_pool);
	if (!solution->apply (dummyResolver))
	    break;
    }
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

