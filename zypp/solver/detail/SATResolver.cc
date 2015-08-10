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
extern "C"
{
#include <solv/repo_solv.h>
#include <solv/poolarch.h>
#include <solv/evr.h>
#include <solv/poolvendor.h>
#include <solv/policy.h>
#include <solv/bitmap.h>
#include <solv/queue.h>
}

#define ZYPP_USE_RESOLVER_INTERNALS

#include "zypp/base/String.h"
#include "zypp/Product.h"
#include "zypp/Capability.h"
#include "zypp/ResStatus.h"
#include "zypp/VendorAttr.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Algorithm.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/ZConfig.h"
#include "zypp/sat/Pool.h"
#include "zypp/sat/WhatProvides.h"
#include "zypp/sat/WhatObsoletes.h"
#include "zypp/solver/detail/Resolver.h"
#include "zypp/solver/detail/SATResolver.h"
#include "zypp/solver/detail/ProblemSolutionCombi.h"
#include "zypp/solver/detail/ProblemSolutionIgnore.h"
#include "zypp/solver/detail/SolverQueueItemInstall.h"
#include "zypp/solver/detail/SolverQueueItemDelete.h"
#include "zypp/solver/detail/SystemCheck.h"
#include "zypp/solver/detail/SolutionAction.h"
#include "zypp/solver/detail/SolverQueueItem.h"
#include "zypp/sat/Transaction.h"
#include "zypp/sat/Queue.h"

#include "zypp/sat/detail/PoolImpl.h"

#define _XDEBUG(x) do { if (base::logger::isExcessive()) XXX << x << std::endl;} while (0)

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////////
  namespace env
  {
    inline bool HACKENV( const char * var_r, bool default_r )
    {
      bool ret = default_r;
      const char * val = ::getenv( var_r );
      if ( val )
      {
	ret = str::strToBool( val, default_r );
	if ( ret != default_r )
	  INT << "HACKENV " << var_r << " = " << ret << endl;
      }
      return ret;
    }
  } // namespace env
  /////////////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

using namespace std;

IMPL_PTR_TYPE(SATResolver);

#define MAYBE_CLEANDEPS (cleandepsOnRemove()?SOLVER_CLEANDEPS:0)

//---------------------------------------------------------------------------
// Callbacks for SAT policies
//---------------------------------------------------------------------------

int vendorCheck( Pool *pool, Solvable *solvable1, Solvable *solvable2 )
{
  return VendorAttr::instance().equivalent( IdString(solvable1->vendor),
                                            IdString(solvable2->vendor) ) ? 0 : 1;
}


inline std::string itemToString( const PoolItem & item )
{
  if ( !item )
    return std::string();

  sat::Solvable slv( item.satSolvable() );
  std::string ret( slv.asString() ); // n-v-r.a
  if ( ! slv.isSystem() )
  {
    ret += "[";
    ret += slv.repository().alias();
    ret += "]";
  }
  return ret;
}

inline PoolItem getPoolItem( Id id_r )
{
  PoolItem ret( (sat::Solvable( id_r )) );
  if ( !ret && id_r )
    INT << "id " << id_r << " not found in ZYPP pool." << endl;
  return ret;
}

//---------------------------------------------------------------------------

std::ostream &
SATResolver::dumpOn( std::ostream & os ) const
{
    os << "<resolver>" << endl;
    if (_solv) {
#define OUTS(X) os << "  " << #X << "\t= " << solver_get_flag(_solv, SOLVER_FLAG_##X) << endl
	OUTS( ALLOW_DOWNGRADE );
	OUTS( ALLOW_ARCHCHANGE );
	OUTS( ALLOW_VENDORCHANGE );
	OUTS( ALLOW_UNINSTALL );
	OUTS( NO_UPDATEPROVIDE );
	OUTS( SPLITPROVIDES );
	OUTS( IGNORE_RECOMMENDED );
	OUTS( ADD_ALREADY_RECOMMENDED );
	OUTS( NO_INFARCHCHECK );
	OUTS( ALLOW_NAMECHANGE );
	OUTS( KEEP_EXPLICIT_OBSOLETES );
	OUTS( BEST_OBEY_POLICY );
	OUTS( NO_AUTOTARGET );
	OUTS( DUP_ALLOW_DOWNGRADE );
	OUTS( DUP_ALLOW_ARCHCHANGE );
	OUTS( DUP_ALLOW_VENDORCHANGE );
	OUTS( DUP_ALLOW_NAMECHANGE );
	OUTS( KEEP_ORPHANS );
	OUTS( BREAK_ORPHANS );
	OUTS( FOCUS_INSTALLED );
	OUTS( YUM_OBSOLETES );
#undef OUTS
	os << "  distupgrade	= "	<< _distupgrade << endl;
        os << "  distupgrade_removeunsupported	= " << _distupgrade_removeunsupported << endl;
	os << "  solveSrcPackages	= "	<< _solveSrcPackages << endl;
	os << "  cleandepsOnRemove	= "	<< _cleandepsOnRemove << endl;
        os << "  fixsystem		= "	<< _fixsystem << endl;
    } else {
	os << "<NULL>";
    }
    return os << "<resolver/>" << endl;
}

//---------------------------------------------------------------------------

SATResolver::SATResolver (const ResPool & pool, Pool *SATPool)
    : _pool (pool)
    , _SATPool (SATPool)
    , _solv(NULL)
    , _fixsystem(false)
    , _allowdowngrade(false)
    , _allowarchchange(false)
    , _allowvendorchange(ZConfig::instance().solver_allowVendorChange())
    , _allowuninstall(false)
    , _updatesystem(false)
    , _noupdateprovide(false)
    , _dosplitprovides(true)
    , _onlyRequires(ZConfig::instance().solver_onlyRequires())
    , _ignorealreadyrecommended(true)
    , _distupgrade(false)
    , _distupgrade_removeunsupported(false)
    , _dup_allowdowngrade( true )
    , _dup_allownamechange( true )
    , _dup_allowarchchange( true )
    , _dup_allowvendorchange( true )
    , _solveSrcPackages(false)
    , _cleandepsOnRemove(ZConfig::instance().solver_cleandepsOnRemove())
{
}


SATResolver::~SATResolver()
{
  solverEnd();
}

//---------------------------------------------------------------------------

ResPool
SATResolver::pool (void) const
{
    return _pool;
}

void
SATResolver::resetItemTransaction (PoolItem item)
{
    bool found = false;
    for (PoolItemList::const_iterator iter = _items_to_remove.begin();
	 iter != _items_to_remove.end(); ++iter) {
	if (*iter == item) {
	    _items_to_remove.remove(*iter);
	    found = true;
	    break;
	}
    }
    if (!found) {
	for (PoolItemList::const_iterator iter = _items_to_install.begin();
	     iter != _items_to_install.end(); ++iter) {
	    if (*iter == item) {
		_items_to_install.remove(*iter);
		found = true;
		break;
	    }
	}
    }
    if (!found) {
	for (PoolItemList::const_iterator iter = _items_to_keep.begin();
	     iter != _items_to_keep.end(); ++iter) {
	    if (*iter == item) {
		_items_to_keep.remove(*iter);
		found = true;
		break;
	    }
	}
    }
    if (!found) {
	for (PoolItemList::const_iterator iter = _items_to_lock.begin();
	     iter != _items_to_lock.end(); ++iter) {
	    if (*iter == item) {
		_items_to_lock.remove(*iter);
		found = true;
		break;
	    }
	}
    }
}


void
SATResolver::addPoolItemToInstall (PoolItem item)
{
    resetItemTransaction (item);
    _items_to_install.push_back (item);
    _items_to_install.unique ();
}


void
SATResolver::addPoolItemsToInstallFromList (PoolItemList & rl)
{
    for (PoolItemList::const_iterator iter = rl.begin(); iter != rl.end(); iter++) {
	addPoolItemToInstall (*iter);
    }
}


void
SATResolver::addPoolItemToRemove (PoolItem item)
{
    resetItemTransaction (item);
    _items_to_remove.push_back (item);
    _items_to_remove.unique ();
}


void
SATResolver::addPoolItemsToRemoveFromList (PoolItemList & rl)
{
    for (PoolItemList::const_iterator iter = rl.begin(); iter != rl.end(); iter++) {
	addPoolItemToRemove (*iter);
    }
}

void
SATResolver::addPoolItemToLock (PoolItem item)
{
    resetItemTransaction (item);
    _items_to_lock.push_back (item);
    _items_to_lock.unique ();
}

void
SATResolver::addPoolItemToKeep (PoolItem item)
{
    resetItemTransaction (item);
    _items_to_keep.push_back (item);
    _items_to_keep.unique ();
}

//---------------------------------------------------------------------------

// copy marked item from solution back to pool
// if data != NULL, set as APPL_LOW (from establishPool())

static void
SATSolutionToPool (PoolItem item, const ResStatus & status, const ResStatus::TransactByValue causer)
{
    // resetting
    item.status().resetTransact (causer);
    item.status().resetWeak ();

    bool r;

    // installation/deletion
    if (status.isToBeInstalled()) {
	r = item.status().setToBeInstalled (causer);
	_XDEBUG("SATSolutionToPool install returns " << item << ", " << r);
    }
    else if (status.isToBeUninstalledDueToUpgrade()) {
	r = item.status().setToBeUninstalledDueToUpgrade (causer);
	_XDEBUG("SATSolutionToPool upgrade returns " << item << ", " <<  r);
    }
    else if (status.isToBeUninstalled()) {
	r = item.status().setToBeUninstalled (causer);
	_XDEBUG("SATSolutionToPool remove returns " << item << ", " <<  r);
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


//------------------------------------------------------------------------------------------------------------
//  This function loops over the pool and grabs all items
//  It clears all previous bySolver() states also
//
//  Every toBeInstalled is passed to zypp::solver:detail::Resolver.addPoolItemToInstall()
//  Every toBeUninstalled is passed to zypp::solver:detail::Resolver.addPoolItemToRemove()
//
//  Solver results must be written back to the pool.
//------------------------------------------------------------------------------------------------------------


struct SATCollectTransact : public resfilter::PoolItemFilterFunctor
{
    SATResolver & resolver;

    SATCollectTransact (SATResolver & r)
	: resolver (r)
    { }

    bool operator()( PoolItem item )		// only transacts() items go here
    {
	ResStatus status = item.status();
	bool by_solver = (status.isBySolver() || status.isByApplLow());

	if (by_solver) {
	    item.status().resetTransact( ResStatus::APPL_LOW );// clear any solver/establish transactions
	    return true;				// back out here, dont re-queue former solver result
	}

	if ( item.satSolvable().isKind<SrcPackage>() && ! resolver.solveSrcPackages() )
	{
	  // Later we may continue on a per source package base.
	  return true; // dont process this source package.
	}

	if (status.isToBeInstalled()) {
	    resolver.addPoolItemToInstall(item);	// -> install!
	}
	else if (status.isToBeUninstalled()) {
	    resolver.addPoolItemToRemove(item);		// -> remove !
	}
        else if (status.isLocked()
		 && !by_solver) {
	    resolver.addPoolItemToLock (item);
        }
        else if (status.isKept()
		 && !by_solver) {
	    resolver.addPoolItemToKeep (item);
        }

	return true;
    }
};


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// solving.....
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


class CheckIfUpdate : public resfilter::PoolItemFilterFunctor
{
  public:
    bool is_updated;
    bool multiversion;
    sat::Solvable _installed;

    CheckIfUpdate( sat::Solvable installed_r )
	: is_updated( false )
        , multiversion( installed_r.multiversionInstall() )
        , _installed( installed_r )
    {}

    // check this item will be updated

    bool operator()( PoolItem item )
    {
	if ( item.status().isToBeInstalled() )
        {
          if ( ! multiversion || sameNVRA( _installed, item ) )
          {
            is_updated = true;
            return false;
          }
	}
	return true;
    }
};


class CollectPseudoInstalled : public resfilter::PoolItemFilterFunctor
{
  public:
    Queue *solvableQueue;

    CollectPseudoInstalled( Queue *queue )
	:solvableQueue (queue)
    {}

    // collecting PseudoInstalled items
    bool operator()( PoolItem item )
    {
      if ( traits::isPseudoInstalled( item.satSolvable().kind() ) )
        queue_push( solvableQueue, item.satSolvable().id() );
      return true;
    }
};

bool
SATResolver::solving(const CapabilitySet & requires_caps,
		     const CapabilitySet & conflict_caps)
{
    _solv = solver_create( _SATPool );
    ::pool_set_custom_vendorcheck( _SATPool, &vendorCheck );
    if (_fixsystem) {
	queue_push( &(_jobQueue), SOLVER_VERIFY|SOLVER_SOLVABLE_ALL);
	queue_push( &(_jobQueue), 0 );
    }
    if (_updatesystem) {
	queue_push( &(_jobQueue), SOLVER_UPDATE|SOLVER_SOLVABLE_ALL);
	queue_push( &(_jobQueue), 0 );
    }
    if (_distupgrade) {
	queue_push( &(_jobQueue), SOLVER_DISTUPGRADE|SOLVER_SOLVABLE_ALL);
	queue_push( &(_jobQueue), 0 );
    }
    if (_distupgrade_removeunsupported) {
	queue_push( &(_jobQueue), SOLVER_DROP_ORPHANED|SOLVER_SOLVABLE_ALL);
	queue_push( &(_jobQueue), 0 );
    }
    solver_set_flag(_solv, SOLVER_FLAG_ADD_ALREADY_RECOMMENDED, !_ignorealreadyrecommended);
    solver_set_flag(_solv, SOLVER_FLAG_ALLOW_DOWNGRADE, _allowdowngrade);
    solver_set_flag(_solv, SOLVER_FLAG_ALLOW_UNINSTALL, _allowuninstall);
    solver_set_flag(_solv, SOLVER_FLAG_ALLOW_ARCHCHANGE, _allowarchchange);
    solver_set_flag(_solv, SOLVER_FLAG_ALLOW_VENDORCHANGE, _allowvendorchange);
    solver_set_flag(_solv, SOLVER_FLAG_SPLITPROVIDES, _dosplitprovides);
    solver_set_flag(_solv, SOLVER_FLAG_NO_UPDATEPROVIDE, _noupdateprovide);
    solver_set_flag(_solv, SOLVER_FLAG_IGNORE_RECOMMENDED, _onlyRequires);
    solver_set_flag(_solv, SOLVER_FLAG_DUP_ALLOW_DOWNGRADE,	_dup_allowdowngrade );
    solver_set_flag(_solv, SOLVER_FLAG_DUP_ALLOW_ARCHCHANGE,	_dup_allownamechange );
    solver_set_flag(_solv, SOLVER_FLAG_DUP_ALLOW_VENDORCHANGE,	_dup_allowarchchange );
    solver_set_flag(_solv, SOLVER_FLAG_DUP_ALLOW_NAMECHANGE,	_dup_allowvendorchange );
#if 1
#define HACKENV(X,D) solver_set_flag(_solv, X, env::HACKENV( #X, D ) );
    HACKENV( SOLVER_FLAG_DUP_ALLOW_DOWNGRADE,	_dup_allowdowngrade );
    HACKENV( SOLVER_FLAG_DUP_ALLOW_ARCHCHANGE,	_dup_allownamechange );
    HACKENV( SOLVER_FLAG_DUP_ALLOW_VENDORCHANGE,_dup_allowarchchange );
    HACKENV( SOLVER_FLAG_DUP_ALLOW_NAMECHANGE,	_dup_allowvendorchange );
#undef HACKENV
#endif
    sat::Pool::instance().prepareForSolving();

    // Solve !
    MIL << "Starting solving...." << endl;
    MIL << *this;
    solver_solve( _solv, &(_jobQueue) );
    MIL << "....Solver end" << endl;

    // copying solution back to zypp pool
    //-----------------------------------------
    _result_items_to_install.clear();
    _result_items_to_remove.clear();

    /*  solvables to be installed */
    Queue decisionq;
    queue_init(&decisionq);
    solver_get_decisionqueue(_solv, &decisionq);
    for ( int i = 0; i < decisionq.count; ++i )
    {
      sat::Solvable slv( decisionq.elements[i] );
      if ( !slv || slv.isSystem() )
	continue;

      PoolItem poolItem( slv );
      SATSolutionToPool (poolItem, ResStatus::toBeInstalled, ResStatus::SOLVER);
      _result_items_to_install.push_back (poolItem);
    }
    queue_free(&decisionq);

    /* solvables to be erased */
    Repository systemRepo( sat::Pool::instance().findSystemRepo() ); // don't create if it does not exist
    if ( systemRepo && ! systemRepo.solvablesEmpty() )
    {
      bool mustCheckObsoletes = false;
      for_( it, systemRepo.solvablesBegin(), systemRepo.solvablesEnd() )
      {
	if (solver_get_decisionlevel(_solv, it->id()) > 0)
	  continue;

	// Check if this is an update
	CheckIfUpdate info( *it );
	PoolItem poolItem( *it );
	invokeOnEach( _pool.byIdentBegin( poolItem ),
		      _pool.byIdentEnd( poolItem ),
		      resfilter::ByUninstalled(),			// ByUninstalled
		      functor::functorRef<bool,PoolItem> (info) );

	if (info.is_updated) {
	  SATSolutionToPool( poolItem, ResStatus::toBeUninstalledDueToUpgrade, ResStatus::SOLVER );
	} else {
	  SATSolutionToPool( poolItem, ResStatus::toBeUninstalled, ResStatus::SOLVER );
	  if ( ! mustCheckObsoletes )
	    mustCheckObsoletes = true; // lazy check for UninstalledDueToObsolete
	}
	_result_items_to_remove.push_back (poolItem);
      }
      if ( mustCheckObsoletes )
      {
	sat::WhatObsoletes obsoleted( _result_items_to_install.begin(), _result_items_to_install.end() );
	for_( it, obsoleted.poolItemBegin(), obsoleted.poolItemEnd() )
	{
	  ResStatus & status( it->status() );
	  // WhatObsoletes contains installed items only!
	  if ( status.transacts() && ! status.isToBeUninstalledDueToUpgrade() )
	    status.setToBeUninstalledDueToObsolete();
	}
      }
    }

    Queue recommendations;
    Queue suggestions;
    Queue orphaned;
    Queue unneeded;
    queue_init(&recommendations);
    queue_init(&suggestions);
    queue_init(&orphaned);
    queue_init(&unneeded);
    solver_get_recommendations(_solv, &recommendations, &suggestions, 0);
    solver_get_orphaned(_solv, &orphaned);
    solver_get_unneeded(_solv, &unneeded, 1);
    /*  solvables which are recommended */
    for ( int i = 0; i < recommendations.count; ++i )
    {
      PoolItem poolItem( getPoolItem( recommendations.elements[i] ) );
      poolItem.status().setRecommended( true );
    }

    /*  solvables which are suggested */
    for ( int i = 0; i < suggestions.count; ++i )
    {
      PoolItem poolItem( getPoolItem( suggestions.elements[i] ) );
      poolItem.status().setSuggested( true );
    }

    _problem_items.clear();
    /*  solvables which are orphaned */
    for ( int i = 0; i < orphaned.count; ++i )
    {
      PoolItem poolItem( getPoolItem( orphaned.elements[i] ) );
      poolItem.status().setOrphaned( true );
      _problem_items.push_back( poolItem );
    }

    /*  solvables which are unneeded */
    for ( int i = 0; i < unneeded.count; ++i )
    {
      PoolItem poolItem( getPoolItem( unneeded.elements[i] ) );
      poolItem.status().setUnneeded( true );
    }

    queue_free(&recommendations);
    queue_free(&suggestions);
    queue_free(&orphaned);
    queue_free(&unneeded);

    /* Write validation state back to pool */
    Queue flags, solvableQueue;

    queue_init(&flags);
    queue_init(&solvableQueue);

    CollectPseudoInstalled collectPseudoInstalled(&solvableQueue);
    invokeOnEach( _pool.begin(),
		  _pool.end(),
		  functor::functorRef<bool,PoolItem> (collectPseudoInstalled) );
    solver_trivial_installable(_solv, &solvableQueue, &flags );
    for (int i = 0; i < solvableQueue.count; i++) {
	PoolItem item = _pool.find (sat::Solvable(solvableQueue.elements[i]));
	item.status().setUndetermined();

	if (flags.elements[i] == -1) {
	    item.status().setNonRelevant();
	    _XDEBUG("SATSolutionToPool(" << item << " ) nonRelevant !");
	} else if (flags.elements[i] == 1) {
	    item.status().setSatisfied();
	    _XDEBUG("SATSolutionToPool(" << item << " ) satisfied !");
	} else if (flags.elements[i] == 0) {
	    item.status().setBroken();
	    _XDEBUG("SATSolutionToPool(" << item << " ) broken !");
	}
    }
    queue_free(&(solvableQueue));
    queue_free(&flags);


    // Solvables which were selected due requirements which have been made by the user will
    // be selected by APPL_LOW. We can't use any higher level, because this setting must
    // not serve as a request for the next solver run. APPL_LOW is reset before solving.
    for (CapabilitySet::const_iterator iter = requires_caps.begin(); iter != requires_caps.end(); iter++) {
	sat::WhatProvides rpmProviders(*iter);
	for_( iter2, rpmProviders.begin(), rpmProviders.end() ) {
	    PoolItem poolItem(*iter2);
	    if (poolItem.status().isToBeInstalled()) {
		MIL << "User requirement " << *iter << " sets " << poolItem << endl;
		poolItem.status().setTransactByValue (ResStatus::APPL_LOW);
	    }
	}
    }
    for (CapabilitySet::const_iterator iter = conflict_caps.begin(); iter != conflict_caps.end(); iter++) {
	sat::WhatProvides rpmProviders(*iter);
	for_( iter2, rpmProviders.begin(), rpmProviders.end() ) {
	    PoolItem poolItem(*iter2);
	    if (poolItem.status().isToBeUninstalled()) {
		MIL << "User conflict " << *iter << " sets " << poolItem << endl;
		poolItem.status().setTransactByValue (ResStatus::APPL_LOW);
	    }
	}
    }

    if (solver_problem_count(_solv) > 0 )
    {
	ERR << "Solverrun finished with an ERROR" << endl;
	return false;
    }

    return true;
}


void
SATResolver::solverInit(const PoolItemList & weakItems)
{
    SATCollectTransact info (*this);

    MIL << "SATResolver::solverInit()" << endl;

    // remove old stuff
    solverEnd();

    queue_init( &_jobQueue );
    _items_to_install.clear();
    _items_to_remove.clear();
    _items_to_lock.clear();
    _items_to_keep.clear();

    invokeOnEach ( _pool.begin(), _pool.end(),
		   functor::functorRef<bool,PoolItem>(info) );

    for (PoolItemList::const_iterator iter = weakItems.begin(); iter != weakItems.end(); iter++) {
	Id id = (*iter)->satSolvable().id();
	if (id == ID_NULL) {
	    ERR << "Weaken: " << *iter << " not found" << endl;
	}
	MIL << "Weaken dependencies of " << *iter << endl;
	queue_push( &(_jobQueue), SOLVER_WEAKENDEPS | SOLVER_SOLVABLE );
        queue_push( &(_jobQueue), id );
    }

    // Ad rules for changed requestedLocales
    const auto & trackedLocaleIds( myPool().trackedLocaleIds() );
    for ( const auto & locale : trackedLocaleIds.added() )
    {
      queue_push( &(_jobQueue), SOLVER_INSTALL | SOLVER_SOLVABLE_PROVIDES );
      queue_push( &(_jobQueue), Capability( ResolverNamespace::language, IdString(locale) ).id() );
    }

    for ( const auto & locale : trackedLocaleIds.removed() )
    {
      queue_push( &(_jobQueue), SOLVER_ERASE | SOLVER_SOLVABLE_PROVIDES | SOLVER_CLEANDEPS );	// needs uncond. SOLVER_CLEANDEPS!
      queue_push( &(_jobQueue), Capability( ResolverNamespace::language, IdString(locale) ).id() );
    }

    // Add rules for parallel installable resolvables with different versions
    for ( const sat::Solvable & solv : myPool().multiversionList() )
    {
      queue_push( &(_jobQueue), SOLVER_NOOBSOLETES | SOLVER_SOLVABLE );
      queue_push( &(_jobQueue), solv.id() );
    }

    ::pool_add_userinstalled_jobs(_SATPool, sat::Pool::instance().autoInstalled(), &(_jobQueue), GET_USERINSTALLED_NAMES|GET_USERINSTALLED_INVERTED);

    if ( _distupgrade )
    {
      if ( ZConfig::instance().solverUpgradeRemoveDroppedPackages() )
      {
        MIL << "Checking droplists ..." << endl;
        // Dropped packages: look for 'weakremover()' provides
        // in dup candidates of installed products.
        ResPoolProxy proxy( ResPool::instance().proxy() );
        for_( it, proxy.byKindBegin<Product>(), proxy.byKindEnd<Product>() )
        {
          if ( (*it)->onSystem() ) // (to install) or (not to delete)
          {
            Product::constPtr prodCand( (*it)->candidateAsKind<Product>() );
            if ( ! prodCand )
              continue; // product no longer available

            CapabilitySet droplist( prodCand->droplist() );
            dumpRangeLine( MIL << "Droplist for " << (*it)->candidateObj() << ": " << droplist.size() << " ", droplist.begin(), droplist.end() ) << endl;
            for_( cap, droplist.begin(), droplist.end() )
            {
              queue_push( &_jobQueue, SOLVER_DROP_ORPHANED | SOLVER_SOLVABLE_NAME );
              queue_push( &_jobQueue, cap->id() );
            }
          }
        }
      }
      else
      {
        MIL << "Droplist processing is disabled." << endl;
      }
    }
}

void
SATResolver::solverEnd()
{
  // cleanup
  if ( _solv )
  {
    solver_free(_solv);
    _solv = NULL;
    queue_free( &(_jobQueue) );
  }
}


bool
SATResolver::resolvePool(const CapabilitySet & requires_caps,
			 const CapabilitySet & conflict_caps,
			 const PoolItemList & weakItems,
                         const std::set<Repository> & upgradeRepos)
{
    MIL << "SATResolver::resolvePool()" << endl;

    // initialize
    solverInit(weakItems);

    for (PoolItemList::const_iterator iter = _items_to_install.begin(); iter != _items_to_install.end(); iter++) {
	Id id = (*iter)->satSolvable().id();
	if (id == ID_NULL) {
	    ERR << "Install: " << *iter << " not found" << endl;
	} else {
	    MIL << "Install " << *iter << endl;
	    queue_push( &(_jobQueue), SOLVER_INSTALL | SOLVER_SOLVABLE );
	    queue_push( &(_jobQueue), id );
	}
    }

    for (PoolItemList::const_iterator iter = _items_to_remove.begin(); iter != _items_to_remove.end(); iter++) {
	Id id = (*iter)->satSolvable().id();
	if (id == ID_NULL) {
	    ERR << "Delete: " << *iter << " not found" << endl;
	} else {
	    MIL << "Delete " << *iter << endl;
	    queue_push( &(_jobQueue), SOLVER_ERASE | SOLVER_SOLVABLE | MAYBE_CLEANDEPS );
	    queue_push( &(_jobQueue), id);
	}
    }

    for_( iter, upgradeRepos.begin(), upgradeRepos.end() )
    {
	queue_push( &(_jobQueue), SOLVER_DISTUPGRADE | SOLVER_SOLVABLE_REPO );
	queue_push( &(_jobQueue), iter->get()->repoid );
        MIL << "Upgrade repo " << *iter << endl;
    }

    for (CapabilitySet::const_iterator iter = requires_caps.begin(); iter != requires_caps.end(); iter++) {
	queue_push( &(_jobQueue), SOLVER_INSTALL | SOLVER_SOLVABLE_PROVIDES );
	queue_push( &(_jobQueue), iter->id() );
	MIL << "Requires " << *iter << endl;
    }

    for (CapabilitySet::const_iterator iter = conflict_caps.begin(); iter != conflict_caps.end(); iter++) {
	queue_push( &(_jobQueue), SOLVER_ERASE | SOLVER_SOLVABLE_PROVIDES | MAYBE_CLEANDEPS );
	queue_push( &(_jobQueue), iter->id() );
	MIL << "Conflicts " << *iter << endl;
    }

    // set requirements for a running system
    setSystemRequirements();

    // set locks for the solver
    setLocks();

    // solving
    bool ret = solving(requires_caps, conflict_caps);

    (ret?MIL:WAR) << "SATResolver::resolvePool() done. Ret:" << ret <<  endl;
    return ret;
}


bool
SATResolver::resolveQueue(const SolverQueueItemList &requestQueue,
			  const PoolItemList & weakItems)
{
    MIL << "SATResolver::resolvQueue()" << endl;

    // initialize
    solverInit(weakItems);

    // generate solver queue
    for (SolverQueueItemList::const_iterator iter = requestQueue.begin(); iter != requestQueue.end(); iter++) {
	(*iter)->addRule(_jobQueue);
    }

    // Add addition item status to the resolve-queue cause these can be set by problem resolutions
    for (PoolItemList::const_iterator iter = _items_to_install.begin(); iter != _items_to_install.end(); iter++) {
	Id id = (*iter)->satSolvable().id();
	if (id == ID_NULL) {
	    ERR << "Install: " << *iter << " not found" << endl;
	} else {
	    MIL << "Install " << *iter << endl;
	    queue_push( &(_jobQueue), SOLVER_INSTALL | SOLVER_SOLVABLE );
	    queue_push( &(_jobQueue), id );
	}
    }
    for (PoolItemList::const_iterator iter = _items_to_remove.begin(); iter != _items_to_remove.end(); iter++) {
        sat::detail::IdType ident( (*iter)->satSolvable().ident().id() );
	MIL << "Delete " << *iter << ident << endl;
	queue_push( &(_jobQueue), SOLVER_ERASE | SOLVER_SOLVABLE_NAME | MAYBE_CLEANDEPS );
	queue_push( &(_jobQueue), ident);
    }

    // set requirements for a running system
    setSystemRequirements();

    // set locks for the solver
    setLocks();

    // solving
    bool ret = solving();

    MIL << "SATResolver::resolveQueue() done. Ret:" << ret <<  endl;
    return ret;
}

/** \todo duplicate code to be joined with \ref solving. */
void SATResolver::doUpdate()
{
    MIL << "SATResolver::doUpdate()" << endl;

    // initialize
    solverInit(PoolItemList());

    // set requirements for a running system
    setSystemRequirements();

    // set locks for the solver
    setLocks();

    _solv = solver_create( _SATPool );
    ::pool_set_custom_vendorcheck( _SATPool, &vendorCheck );
    if (_fixsystem) {
	queue_push( &(_jobQueue), SOLVER_VERIFY|SOLVER_SOLVABLE_ALL);
	queue_push( &(_jobQueue), 0 );
    }
    if (1) {
	queue_push( &(_jobQueue), SOLVER_UPDATE|SOLVER_SOLVABLE_ALL);
	queue_push( &(_jobQueue), 0 );
    }
    if (_distupgrade) {
	queue_push( &(_jobQueue), SOLVER_DISTUPGRADE|SOLVER_SOLVABLE_ALL);
	queue_push( &(_jobQueue), 0 );
    }
    if (_distupgrade_removeunsupported) {
	queue_push( &(_jobQueue), SOLVER_DROP_ORPHANED|SOLVER_SOLVABLE_ALL);
	queue_push( &(_jobQueue), 0 );
    }
    solver_set_flag(_solv, SOLVER_FLAG_ADD_ALREADY_RECOMMENDED, !_ignorealreadyrecommended);
    solver_set_flag(_solv, SOLVER_FLAG_ALLOW_DOWNGRADE, _allowdowngrade);
    solver_set_flag(_solv, SOLVER_FLAG_ALLOW_UNINSTALL, _allowuninstall);
    solver_set_flag(_solv, SOLVER_FLAG_ALLOW_ARCHCHANGE, _allowarchchange);
    solver_set_flag(_solv, SOLVER_FLAG_ALLOW_VENDORCHANGE, _allowvendorchange);
    solver_set_flag(_solv, SOLVER_FLAG_SPLITPROVIDES, _dosplitprovides);
    solver_set_flag(_solv, SOLVER_FLAG_NO_UPDATEPROVIDE, _noupdateprovide);
    solver_set_flag(_solv, SOLVER_FLAG_IGNORE_RECOMMENDED, _onlyRequires);

    sat::Pool::instance().prepareForSolving();

    // Solve !
    MIL << "Starting solving for update...." << endl;
    MIL << *this;
    solver_solve( _solv, &(_jobQueue) );
    MIL << "....Solver end" << endl;

    // copying solution back to zypp pool
    //-----------------------------------------

    /*  solvables to be installed */
    Queue decisionq;
    queue_init(&decisionq);
    solver_get_decisionqueue(_solv, &decisionq);
    for (int i = 0; i < decisionq.count; i++)
    {
      Id p;
      p = decisionq.elements[i];
      if (p < 0 || !sat::Solvable(p))
	continue;
      if (sat::Solvable(p).repository().get() == _solv->pool->installed)
	continue;

      PoolItem poolItem = _pool.find (sat::Solvable(p));
      if (poolItem) {
	  SATSolutionToPool (poolItem, ResStatus::toBeInstalled, ResStatus::SOLVER);
      } else {
	  ERR << "id " << p << " not found in ZYPP pool." << endl;
      }
    }
    queue_free(&decisionq);

    /* solvables to be erased */
    for (int i = _solv->pool->installed->start; i < _solv->pool->installed->start + _solv->pool->installed->nsolvables; i++)
    {
      if (solver_get_decisionlevel(_solv, i) > 0)
	  continue;

      PoolItem poolItem( _pool.find( sat::Solvable(i) ) );
      if (poolItem) {
	  // Check if this is an update
	  CheckIfUpdate info( (sat::Solvable(i)) );
	  invokeOnEach( _pool.byIdentBegin( poolItem ),
			_pool.byIdentEnd( poolItem ),
			resfilter::ByUninstalled(),			// ByUninstalled
			functor::functorRef<bool,PoolItem> (info) );

	  if (info.is_updated) {
	      SATSolutionToPool (poolItem, ResStatus::toBeUninstalledDueToUpgrade , ResStatus::SOLVER);
	  } else {
	      SATSolutionToPool (poolItem, ResStatus::toBeUninstalled, ResStatus::SOLVER);
	  }
      } else {
	  ERR << "id " << i << " not found in ZYPP pool." << endl;
      }
    }
    MIL << "SATResolver::doUpdate() done" << endl;
}



//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// error handling
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// helper function
//----------------------------------------------------------------------------

struct FindPackage : public resfilter::ResObjectFilterFunctor
{
    ProblemSolutionCombi *problemSolution;
    TransactionKind action;
    FindPackage (ProblemSolutionCombi *p, const TransactionKind act)
       : problemSolution (p)
       , action (act)
	{
	}

    bool operator()( PoolItem p)
   {
       problemSolution->addSingleAction (p, action);
       return true;
   }
};


//----------------------------------------------------------------------------
// Checking if this solvable/item has a buddy which reflect the real
// user visible description of an item
// e.g. The release package has a buddy to the concerning product item.
// This user want's the message "Product foo conflicts with product bar" and
// NOT "package release-foo conflicts with package release-bar"
// (ma: that's why we should map just packages to buddies, not vice versa)
//----------------------------------------------------------------------------
inline sat::Solvable mapBuddy( const PoolItem & item_r )
{
  if ( item_r.satSolvable().isKind<Package>() )
  {
    sat::Solvable buddy = item_r.buddy();
    if ( buddy )
      return buddy;
  }
  return item_r.satSolvable();
}
inline sat::Solvable mapBuddy( sat::Solvable item_r )
{ return mapBuddy( PoolItem( item_r ) ); }

PoolItem SATResolver::mapItem ( const PoolItem & item )
{ return PoolItem( mapBuddy( item ) ); }

sat::Solvable SATResolver::mapSolvable ( const Id & id )
{ return mapBuddy( sat::Solvable(id) ); }

string SATResolver::SATprobleminfoString(Id problem, string &detail, Id &ignoreId)
{
  string ret;
  Pool *pool = _solv->pool;
  Id probr;
  Id dep, source, target;
  sat::Solvable s, s2;

  ignoreId = 0;

  // FIXME: solver_findallproblemrules to get all rules for this problem
  // (the 'most relevabt' one returned by solver_findproblemrule is embedded
  probr = solver_findproblemrule(_solv, problem);
  switch (solver_ruleinfo(_solv, probr, &source, &target, &dep))
  {
      case SOLVER_RULE_DISTUPGRADE:
	  s = mapSolvable (source);
	  ret = str::form (_("%s does not belong to a distupgrade repository"), s.asString().c_str());
  	  break;
      case SOLVER_RULE_INFARCH:
	  s = mapSolvable (source);
	  ret = str::form (_("%s has inferior architecture"), s.asString().c_str());
	  break;
      case SOLVER_RULE_UPDATE:
	  s = mapSolvable (source);
	  ret = str::form (_("problem with installed package %s"), s.asString().c_str());
	  break;
      case SOLVER_RULE_JOB:
	  ret = _("conflicting requests");
	  break;
      case SOLVER_RULE_RPM:
	  ret = _("some dependency problem");
	  break;
      case SOLVER_RULE_JOB_NOTHING_PROVIDES_DEP:
	  ret = str::form (_("nothing provides requested %s"), pool_dep2str(pool, dep));
	  detail += _("Have you enabled all requested repositories?");
	  break;
      case SOLVER_RULE_JOB_UNKNOWN_PACKAGE:
	  ret = str::form (_("package %s does not exist"), pool_dep2str(pool, dep));
	  detail += _("Have you enabled all requested repositories?");
	  break;
      case SOLVER_RULE_JOB_UNSUPPORTED:
	  ret = _("unsupported request");
	  break;
      case SOLVER_RULE_JOB_PROVIDED_BY_SYSTEM:
	  ret = str::form (_("%s is provided by the system and cannot be erased"), pool_dep2str(pool, dep));
	  break;
      case SOLVER_RULE_RPM_NOT_INSTALLABLE:
	  s = mapSolvable (source);
	  ret = str::form (_("%s is not installable"), s.asString().c_str());
	  break;
      case SOLVER_RULE_RPM_NOTHING_PROVIDES_DEP:
	  ignoreId = source; // for setting weak dependencies
	  s = mapSolvable (source);
	  ret = str::form (_("nothing provides %s needed by %s"), pool_dep2str(pool, dep), s.asString().c_str());
	  break;
      case SOLVER_RULE_RPM_SAME_NAME:
	  s = mapSolvable (source);
	  s2 = mapSolvable (target);
	  ret = str::form (_("cannot install both %s and %s"), s.asString().c_str(), s2.asString().c_str());
	  break;
      case SOLVER_RULE_RPM_PACKAGE_CONFLICT:
	  s = mapSolvable (source);
	  s2 = mapSolvable (target);
	  ret = str::form (_("%s conflicts with %s provided by %s"), s.asString().c_str(), pool_dep2str(pool, dep), s2.asString().c_str());
	  break;
      case SOLVER_RULE_RPM_PACKAGE_OBSOLETES:
	  s = mapSolvable (source);
	  s2 = mapSolvable (target);
	  ret = str::form (_("%s obsoletes %s provided by %s"), s.asString().c_str(), pool_dep2str(pool, dep), s2.asString().c_str());
	  break;
      case SOLVER_RULE_RPM_INSTALLEDPKG_OBSOLETES:
	  s = mapSolvable (source);
	  s2 = mapSolvable (target);
	  ret = str::form (_("installed %s obsoletes %s provided by %s"), s.asString().c_str(), pool_dep2str(pool, dep), s2.asString().c_str());
	  break;
      case SOLVER_RULE_RPM_SELF_CONFLICT:
	  s = mapSolvable (source);
	  ret = str::form (_("solvable %s conflicts with %s provided by itself"), s.asString().c_str(), pool_dep2str(pool, dep));
          break;
      case SOLVER_RULE_RPM_PACKAGE_REQUIRES:
	  ignoreId = source; // for setting weak dependencies
	  s = mapSolvable (source);
	  Capability cap(dep);
	  sat::WhatProvides possibleProviders(cap);

	  // check, if a provider will be deleted
	  typedef list<PoolItem> ProviderList;
	  ProviderList providerlistInstalled, providerlistUninstalled;
	  for_( iter1, possibleProviders.begin(), possibleProviders.end() ) {
	      PoolItem provider1 = ResPool::instance().find( *iter1 );
	      // find pair of an installed/uninstalled item with the same NVR
	      bool found = false;
	      for_( iter2, possibleProviders.begin(), possibleProviders.end() ) {
		  PoolItem provider2 = ResPool::instance().find( *iter2 );
		  if (compareByNVR (provider1,provider2) == 0
		      && ( (provider1.status().isInstalled() && provider2.status().isUninstalled())
			  || (provider2.status().isInstalled() && provider1.status().isUninstalled()) ))  {
		      found = true;
		      break;
		  }
	      }
	      if (!found) {
		  if (provider1.status().isInstalled())
		      providerlistInstalled.push_back(provider1);
		  else
		      providerlistUninstalled.push_back(provider1);
	      }
	  }

	  ret = str::form (_("%s requires %s, but this requirement cannot be provided"), s.asString().c_str(), pool_dep2str(pool, dep));
	  if (providerlistInstalled.size() > 0) {
	      detail += _("deleted providers: ");
	      for (ProviderList::const_iterator iter = providerlistInstalled.begin(); iter != providerlistInstalled.end(); iter++) {
		  if (iter == providerlistInstalled.begin())
		      detail += itemToString( *iter );
		  else
		      detail += "\n                   " + itemToString( mapItem(*iter) );
	      }
	  }
	  if (providerlistUninstalled.size() > 0) {
	      if (detail.size() > 0)
		  detail += _("\nuninstallable providers: ");
	      else
		  detail = _("uninstallable providers: ");
	      for (ProviderList::const_iterator iter = providerlistUninstalled.begin(); iter != providerlistUninstalled.end(); iter++) {
		  if (iter == providerlistUninstalled.begin())
		      detail += itemToString( *iter );
		  else
		      detail += "\n                   " + itemToString( mapItem(*iter) );
	      }
	  }
	  break;
  }

  return ret;
}

ResolverProblemList
SATResolver::problems ()
{
    ResolverProblemList resolverProblems;
    if (_solv && solver_problem_count(_solv)) {
	Pool *pool = _solv->pool;
	int pcnt;
	Id p, rp, what;
	Id problem, solution, element;
	sat::Solvable s, sd;

	CapabilitySet system_requires = SystemCheck::instance().requiredSystemCap();
	CapabilitySet system_conflicts = SystemCheck::instance().conflictSystemCap();

	MIL << "Encountered problems! Here are the solutions:\n" << endl;
	pcnt = 1;
	problem = 0;
	while ((problem = solver_next_problem(_solv, problem)) != 0) {
	    MIL << "Problem " <<  pcnt++ << ":" << endl;
	    MIL << "====================================" << endl;
	    string detail;
	    Id ignoreId;
	    string whatString = SATprobleminfoString (problem,detail,ignoreId);
	    MIL << whatString << endl;
	    MIL << "------------------------------------" << endl;
	    ResolverProblem_Ptr resolverProblem = new ResolverProblem (whatString, detail);

	    solution = 0;
	    while ((solution = solver_next_solution(_solv, problem, solution)) != 0) {
		element = 0;
		ProblemSolutionCombi *problemSolution = new ProblemSolutionCombi;
		while ((element = solver_next_solutionelement(_solv, problem, solution, element, &p, &rp)) != 0) {
		    if (p == SOLVER_SOLUTION_JOB) {
			/* job, rp is index into job queue */
			what = _jobQueue.elements[rp];
			switch (_jobQueue.elements[rp-1]&(SOLVER_SELECTMASK|SOLVER_JOBMASK))
			{
			    case SOLVER_INSTALL | SOLVER_SOLVABLE: {
				s = mapSolvable (what);
				PoolItem poolItem = _pool.find (s);
				if (poolItem) {
				    if (pool->installed && s.get()->repo == pool->installed) {
					problemSolution->addSingleAction (poolItem, REMOVE);
					string description = str::form (_("remove lock to allow removal of %s"),  s.asString().c_str() );
					MIL << description << endl;
					problemSolution->addDescription (description);
				    } else {
					problemSolution->addSingleAction (poolItem, KEEP);
					string description = str::form (_("do not install %s"), s.asString().c_str());
					MIL << description << endl;
					problemSolution->addDescription (description);
				    }
				} else {
				    ERR << "SOLVER_INSTALL_SOLVABLE: No item found for " << s.asString() << endl;
				}
			    }
				break;
			    case SOLVER_ERASE | SOLVER_SOLVABLE: {
				s = mapSolvable (what);
				PoolItem poolItem = _pool.find (s);
				if (poolItem) {
				    if (pool->installed && s.get()->repo == pool->installed) {
					problemSolution->addSingleAction (poolItem, KEEP);
					string description = str::form (_("keep %s"), s.asString().c_str());
					MIL << description << endl;
					problemSolution->addDescription (description);
				    } else {
					problemSolution->addSingleAction (poolItem, UNLOCK);
					string description = str::form (_("remove lock to allow installation of %s"), itemToString( poolItem ).c_str());
					MIL << description << endl;
					problemSolution->addDescription (description);
				    }
				} else {
				    ERR << "SOLVER_ERASE_SOLVABLE: No item found for " << s.asString() << endl;
				}
			    }
				break;
			    case SOLVER_INSTALL | SOLVER_SOLVABLE_NAME:
				{
				IdString ident( what );
				SolverQueueItemInstall_Ptr install =
				    new SolverQueueItemInstall(_pool, ident.asString(), false );
				problemSolution->addSingleAction (install, REMOVE_SOLVE_QUEUE_ITEM);

				string description = str::form (_("do not install %s"), ident.c_str() );
				MIL << description << endl;
				problemSolution->addDescription (description);
				}
				break;
			    case SOLVER_ERASE | SOLVER_SOLVABLE_NAME:
				{
				// As we do not know, if this request has come from resolvePool or
				// resolveQueue we will have to take care for both cases.
                                IdString ident( what );
				FindPackage info (problemSolution, KEEP);
				invokeOnEach( _pool.byIdentBegin( ident ),
					      _pool.byIdentEnd( ident ),
					      functor::chain (resfilter::ByInstalled (),			// ByInstalled
							      resfilter::ByTransact ()),			// will be deinstalled
					      functor::functorRef<bool,PoolItem> (info) );

				SolverQueueItemDelete_Ptr del =
				    new SolverQueueItemDelete(_pool, ident.asString(), false );
				problemSolution->addSingleAction (del, REMOVE_SOLVE_QUEUE_ITEM);

				string description = str::form (_("keep %s"), ident.c_str());
				MIL << description << endl;
				problemSolution->addDescription (description);
				}
				break;
			    case SOLVER_INSTALL | SOLVER_SOLVABLE_PROVIDES:
				{
				problemSolution->addSingleAction (Capability(what), REMOVE_EXTRA_REQUIRE);
				string description = "";

				// Checking if this problem solution would break your system
				if (system_requires.find(Capability(what)) != system_requires.end()) {
				    // Show a better warning
				    resolverProblem->setDetails( resolverProblem->description() + "\n" + resolverProblem->details() );
				    resolverProblem->setDescription(_("This request will break your system!"));
				    description = _("ignore the warning of a broken system");
                                    description += string(" (requires:")+pool_dep2str(pool, what)+")";
                                    MIL << description << endl;
                                    problemSolution->addFrontDescription (description);
				} else {
				    description = str::form (_("do not ask to install a solvable providing %s"), pool_dep2str(pool, what));
                                    MIL << description << endl;
                                    problemSolution->addDescription (description);
				}
				}
				break;
			    case SOLVER_ERASE | SOLVER_SOLVABLE_PROVIDES:
				{
				problemSolution->addSingleAction (Capability(what), REMOVE_EXTRA_CONFLICT);
				string description = "";

				// Checking if this problem solution would break your system
				if (system_conflicts.find(Capability(what)) != system_conflicts.end()) {
				    // Show a better warning
				    resolverProblem->setDetails( resolverProblem->description() + "\n" + resolverProblem->details() );
				    resolverProblem->setDescription(_("This request will break your system!"));
				    description = _("ignore the warning of a broken system");
                                    description += string(" (conflicts:")+pool_dep2str(pool, what)+")";
                                    MIL << description << endl;
                                    problemSolution->addFrontDescription (description);

				} else {
				    description = str::form (_("do not ask to delete all solvables providing %s"), pool_dep2str(pool, what));
                                    MIL << description << endl;
                                    problemSolution->addDescription (description);
				}
				}
				break;
			    case SOLVER_UPDATE | SOLVER_SOLVABLE:
				{
				s = mapSolvable (what);
				PoolItem poolItem = _pool.find (s);
				if (poolItem) {
				    if (pool->installed && s.get()->repo == pool->installed) {
					problemSolution->addSingleAction (poolItem, KEEP);
					string description = str::form (_("do not install most recent version of %s"), s.asString().c_str());
					MIL << description << endl;
					problemSolution->addDescription (description);
				    } else {
					ERR << "SOLVER_INSTALL_SOLVABLE_UPDATE " << poolItem << " is not selected for installation" << endl;
				    }
				} else {
				    ERR << "SOLVER_INSTALL_SOLVABLE_UPDATE: No item found for " << s.asString() << endl;
				}
				}
				break;
			    default:
				MIL << "- do something different" << endl;
				ERR << "No valid solution available" << endl;
				break;
			}
		    } else if (p == SOLVER_SOLUTION_INFARCH) {
			s = mapSolvable (rp);
			PoolItem poolItem = _pool.find (s);
			if (pool->installed && s.get()->repo == pool->installed) {
			    problemSolution->addSingleAction (poolItem, LOCK);
			    string description = str::form (_("keep %s despite the inferior architecture"), s.asString().c_str());
			    MIL << description << endl;
			    problemSolution->addDescription (description);
			} else {
			    problemSolution->addSingleAction (poolItem, INSTALL);
			    string description = str::form (_("install %s despite the inferior architecture"), s.asString().c_str());
			    MIL << description << endl;
			    problemSolution->addDescription (description);
			}
		    } else if (p == SOLVER_SOLUTION_DISTUPGRADE) {
			s = mapSolvable (rp);
			PoolItem poolItem = _pool.find (s);
			if (pool->installed && s.get()->repo == pool->installed) {
			    problemSolution->addSingleAction (poolItem, LOCK);
			    string description = str::form (_("keep obsolete %s"), s.asString().c_str());
			    MIL << description << endl;
			    problemSolution->addDescription (description);
			} else {
			    problemSolution->addSingleAction (poolItem, INSTALL);
			    string description = str::form (_("install %s from excluded repository"), s.asString().c_str());
			    MIL << description << endl;
			    problemSolution->addDescription (description);
			}
		    } else {
			/* policy, replace p with rp */
			s = mapSolvable (p);
			PoolItem itemFrom = _pool.find (s);
			if (rp)
			{
			    int gotone = 0;

			    sd = mapSolvable (rp);
			    PoolItem itemTo = _pool.find (sd);
			    if (itemFrom && itemTo) {
				problemSolution->addSingleAction (itemTo, INSTALL);
				int illegal = policy_is_illegal(_solv, s.get(), sd.get(), 0);

				if ((illegal & POLICY_ILLEGAL_DOWNGRADE) != 0)
				{
				    string description = str::form (_("downgrade of %s to %s"), s.asString().c_str(), sd.asString().c_str());
				    MIL << description << endl;
				    problemSolution->addDescription (description);
				    gotone = 1;
				}
				if ((illegal & POLICY_ILLEGAL_ARCHCHANGE) != 0)
				{
				    string description = str::form (_("architecture change of %s to %s"), s.asString().c_str(), sd.asString().c_str());
				    MIL << description << endl;
				    problemSolution->addDescription (description);
				    gotone = 1;
				}
				if ((illegal & POLICY_ILLEGAL_VENDORCHANGE) != 0)
				{
                                    IdString s_vendor( s.vendor() );
                                    IdString sd_vendor( sd.vendor() );
				    string description = str::form (_("install %s (with vendor change)\n  %s  -->  %s") ,
								    sd.asString().c_str(),
                                                                    ( s_vendor ? s_vendor.c_str() : " (no vendor) " ),
                                                                    ( sd_vendor ? sd_vendor.c_str() : " (no vendor) " ) );
				    MIL << description << endl;
				    problemSolution->addDescription (description);
				    gotone = 1;
				}
				if (!gotone) {
				    string description = str::form (_("replacement of %s with %s"), s.asString().c_str(), sd.asString().c_str());
				    MIL << description << endl;
				    problemSolution->addDescription (description);
				}
			    } else {
				ERR << s.asString() << " or "  << sd.asString() << " not found" << endl;
			    }
			}
			else
			{
			    if (itemFrom) {
				string description = str::form (_("deinstallation of %s"), s.asString().c_str());
				MIL << description << endl;
				problemSolution->addDescription (description);
				problemSolution->addSingleAction (itemFrom, REMOVE);
			    }
			}
		    }
		}
		resolverProblem->addSolution (problemSolution,
					      problemSolution->actionCount() > 1 ? true : false); // Solutions with more than 1 action will be shown first.
		MIL << "------------------------------------" << endl;
	    }

	    if (ignoreId > 0) {
		// There is a possibility to ignore this error by setting weak dependencies
		PoolItem item = _pool.find (sat::Solvable(ignoreId));
		ProblemSolutionIgnore *problemSolution = new ProblemSolutionIgnore(item);
		resolverProblem->addSolution (problemSolution,
					      false); // Solutions will be shown at the end
		MIL << "ignore some dependencies of " << item << endl;
		MIL << "------------------------------------" << endl;
	    }

	    // save problem
	    resolverProblems.push_back (resolverProblem);
	}
    }
    return resolverProblems;
}

void SATResolver::applySolutions( const ProblemSolutionList & solutions )
{ Resolver( _pool ).applySolutions( solutions ); }

void SATResolver::setLocks()
{
    for (PoolItemList::const_iterator iter = _items_to_lock.begin(); iter != _items_to_lock.end(); ++iter) {
        sat::detail::SolvableIdType ident( (*iter)->satSolvable().id() );
	if (iter->status().isInstalled()) {
	    MIL << "Lock installed item " << *iter << endl;
	    queue_push( &(_jobQueue), SOLVER_INSTALL | SOLVER_SOLVABLE );
	    queue_push( &(_jobQueue), ident );
	} else {
	    MIL << "Lock NOT installed item " << *iter << endl;
	    queue_push( &(_jobQueue), SOLVER_ERASE | SOLVER_SOLVABLE | MAYBE_CLEANDEPS );
	    queue_push( &(_jobQueue), ident );
	}
    }

    ///////////////////////////////////////////////////////////////////
    // Weak locks: Ignore if an item with this name is already installed.
    // If it's not installed try to keep it this way using a weak delete
    ///////////////////////////////////////////////////////////////////
    std::set<IdString> unifiedByName;
    for (PoolItemList::const_iterator iter = _items_to_keep.begin(); iter != _items_to_keep.end(); ++iter) {
      IdString ident( (*iter)->satSolvable().ident() );
      if ( unifiedByName.insert( ident ).second )
      {
	if ( ! ui::Selectable::get( *iter )->hasInstalledObj() )
	{
	  MIL << "Keep NOT installed name " << ident << " (" << *iter << ")" << endl;
	  queue_push( &(_jobQueue), SOLVER_ERASE | SOLVER_SOLVABLE_NAME | SOLVER_WEAK | MAYBE_CLEANDEPS );
	  queue_push( &(_jobQueue), ident.id() );
	}
      }
    }
}

void SATResolver::setSystemRequirements()
{
    CapabilitySet system_requires = SystemCheck::instance().requiredSystemCap();
    CapabilitySet system_conflicts = SystemCheck::instance().conflictSystemCap();

    for (CapabilitySet::const_iterator iter = system_requires.begin(); iter != system_requires.end(); ++iter) {
	queue_push( &(_jobQueue), SOLVER_INSTALL | SOLVER_SOLVABLE_PROVIDES );
	queue_push( &(_jobQueue), iter->id() );
	MIL << "SYSTEM Requires " << *iter << endl;
    }

    for (CapabilitySet::const_iterator iter = system_conflicts.begin(); iter != system_conflicts.end(); ++iter) {
	queue_push( &(_jobQueue), SOLVER_ERASE | SOLVER_SOLVABLE_PROVIDES | MAYBE_CLEANDEPS );
	queue_push( &(_jobQueue), iter->id() );
	MIL << "SYSTEM Conflicts " << *iter << endl;
    }

    // Lock the architecture of the running systems rpm
    // package on distupgrade.
    if ( _distupgrade && ZConfig::instance().systemRoot() == "/" )
    {
      ResPool pool( ResPool::instance() );
      IdString rpm( "rpm" );
      for_( it, pool.byIdentBegin(rpm), pool.byIdentEnd(rpm) )
      {
        if ( (*it)->isSystem() )
        {
          Capability archrule( (*it)->arch(), rpm.c_str(), Capability::PARSED );
          queue_push( &(_jobQueue), SOLVER_INSTALL | SOLVER_SOLVABLE_NAME | SOLVER_ESSENTIAL );
          queue_push( &(_jobQueue), archrule.id() );

        }
      }
    }
}

sat::StringQueue SATResolver::autoInstalled() const
{
  sat::StringQueue ret;
  if ( _solv )
    ::solver_get_userinstalled( _solv, ret, GET_USERINSTALLED_NAMES|GET_USERINSTALLED_INVERTED );
  return ret;
}

sat::StringQueue SATResolver::userInstalled() const
{
  sat::StringQueue ret;
  if ( _solv )
    ::solver_get_userinstalled( _solv, ret, GET_USERINSTALLED_NAMES );
  return ret;
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

