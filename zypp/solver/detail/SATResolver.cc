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

#include <zypp/base/LogTools.h>
#include <zypp/base/Gettext.h>
#include <zypp/base/Algorithm.h>

#include <zypp/ZConfig.h>
#include <zypp/Product.h>
#include <zypp/AutoDispose.h>
#include <zypp/sat/WhatProvides.h>
#include <zypp/sat/WhatObsoletes.h>
#include <zypp/sat/detail/PoolImpl.h>

#include <zypp/solver/detail/Resolver.h>
#include <zypp/solver/detail/SATResolver.h>

#include <zypp/solver/detail/ProblemSolutionCombi.h>
#include <zypp/solver/detail/ProblemSolutionIgnore.h>
#include <zypp/solver/detail/SolverQueueItemInstall.h>
#include <zypp/solver/detail/SolverQueueItemDelete.h>
#include <zypp/solver/detail/SystemCheck.h>
#include <zypp/solver/detail/SolutionAction.h>
#include <zypp/solver/detail/SolverQueueItem.h>
using std::endl;

#define XDEBUG(x) do { if (base::logger::isExcessive()) XXX << x << std::endl;} while (0)

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::solver"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////////
      namespace
      {
        inline void solverSetFocus( sat::detail::CSolver & satSolver_r, const ResolverFocus & focus_r )
        {
          switch ( focus_r )
          {
            case ResolverFocus::Default:	// fallthrough to Job
            case ResolverFocus::Job:
              solver_set_flag( &satSolver_r, SOLVER_FLAG_FOCUS_INSTALLED, 0 );
              solver_set_flag( &satSolver_r, SOLVER_FLAG_FOCUS_BEST,      0 );
              break;
            case ResolverFocus::Installed:
              solver_set_flag( &satSolver_r, SOLVER_FLAG_FOCUS_INSTALLED, 1 );
              solver_set_flag( &satSolver_r, SOLVER_FLAG_FOCUS_BEST,      0 );
              break;
            case ResolverFocus::Update:
              solver_set_flag( &satSolver_r, SOLVER_FLAG_FOCUS_INSTALLED, 0 );
              solver_set_flag( &satSolver_r, SOLVER_FLAG_FOCUS_BEST,      1 );
              break;
          }
        }

        /** Helper collecting pseudo installed items from the pool.
         * \todo: pseudoItems are cachable as long as pool content does not change
         */
        inline sat::Queue collectPseudoInstalled( const ResPool & pool_r )
        {
          sat::Queue ret;
          for ( const PoolItem & pi : pool_r )
            if ( traits::isPseudoInstalled( pi.kind() ) ) ret.push( pi.id() );
          return ret;
        }

        /** Copy back new \ref WeakValue to \ref PoolItem after solving.
         * On the fly collect orphaned items (cached by the solver for the UI)
         */
        inline void solverCopyBackWeak( sat::detail::CSolver & satSolver_r, PoolItemList & orphanedItems_r )
        {
          // NOTE: assert all items weak stati are reset (resetWeak was called)
          {
            sat::Queue recommendations;
            sat::Queue suggestions;
            ::solver_get_recommendations( &satSolver_r, recommendations, suggestions, 0 );
            for ( sat::Queue::size_type i = 0; i < recommendations.size(); ++i )
              PoolItem(sat::Solvable(recommendations[i])).status().setRecommended( true );
            for ( sat::Queue::size_type i = 0; i < suggestions.size(); ++i )
              PoolItem(sat::Solvable(suggestions[i])).status().setSuggested( true );
          }
          {
            orphanedItems_r.clear();	// cached on the fly
            sat::Queue orphaned;
            ::solver_get_orphaned( &satSolver_r, orphaned );
            for ( sat::Queue::size_type i = 0; i < orphaned.size(); ++i )
            {
              PoolItem pi { sat::Solvable(orphaned[i]) };
              pi.status().setOrphaned( true );
              orphanedItems_r.push_back( pi );
            }
          }
          {
            sat::Queue unneeded;
            ::solver_get_unneeded( &satSolver_r, unneeded, 1 );
            for ( sat::Queue::size_type i = 0; i < unneeded.size(); ++i )
              PoolItem(sat::Solvable(unneeded[i])).status().setUnneeded( true );
          }
        }

        /** Copy back new \ref ValidateValue to \ref PoolItem after solving. */
        inline void solverCopyBackValidate( sat::detail::CSolver & satSolver_r, const ResPool & pool_r )
        {
          sat::Queue pseudoItems { collectPseudoInstalled( pool_r ) };
          if ( ! pseudoItems.empty() )
          {
            sat::Queue pseudoFlags;
            ::solver_trivial_installable( &satSolver_r, pseudoItems, pseudoFlags );

            for ( sat::Queue::size_type i = 0; i < pseudoItems.size(); ++i )
            {
              PoolItem pi { sat::Solvable(pseudoItems[i]) };
              switch ( pseudoFlags[i] )
              {
                case 0:  pi.status().setBroken(); break;
                case 1:  pi.status().setSatisfied(); break;
                case -1: pi.status().setNonRelevant(); break;
                default: pi.status().setUndetermined(); break;
              }
            }
          }
        }

      } //namespace
      ///////////////////////////////////////////////////////////////////////



IMPL_PTR_TYPE(SATResolver);

#define MAYBE_CLEANDEPS (cleandepsOnRemove()?SOLVER_CLEANDEPS:0)

//---------------------------------------------------------------------------
// Callbacks for SAT policies
//---------------------------------------------------------------------------

int vendorCheck( sat::detail::CPool *pool, Solvable *solvable1, Solvable *solvable2 )
{ return VendorAttr::instance().equivalent( IdString(solvable1->vendor), IdString(solvable2->vendor) ) ? 0 : 1; }

int relaxedVendorCheck( sat::detail::CPool *pool, Solvable *solvable1, Solvable *solvable2 )
{ return VendorAttr::instance().relaxedEquivalent( IdString(solvable1->vendor), IdString(solvable2->vendor) ) ? 0 : 1; }

/** ResPool helper to compute the initial status of Patches etc.
 * An empty solver run (no jobs) just to compute the initial status
 * of pseudo installed items (patches).
 */
void establish( sat::Queue & pseudoItems_r, sat::Queue & pseudoFlags_r )
{
  pseudoItems_r = collectPseudoInstalled( ResPool::instance() );
  if ( ! pseudoItems_r.empty() )
  {
    auto satPool = sat::Pool::instance();
    MIL << "Establish..." << endl;
    sat::detail::CPool * cPool { satPool.get() };
    ::pool_set_custom_vendorcheck( cPool, &vendorCheck );

    sat::Queue jobQueue;
    // Add rules for parallel installable resolvables with different versions
    for ( const sat::Solvable & solv : satPool.multiversion() )
    {
      jobQueue.push( SOLVER_NOOBSOLETES | SOLVER_SOLVABLE );
      jobQueue.push( solv.id() );
    }

    AutoDispose<sat::detail::CSolver*> cSolver { ::solver_create( cPool ), ::solver_free };
    satPool.prepare();
    if ( ::solver_solve( cSolver, jobQueue ) != 0 )
      INT << "How can establish fail?" << endl;

    ::solver_trivial_installable( cSolver, pseudoItems_r, pseudoFlags_r );

    for ( sat::Queue::size_type i = 0; i < pseudoItems_r.size(); ++i )
    {
      PoolItem pi { sat::Solvable(pseudoItems_r[i]) };
      switch ( pseudoFlags_r[i] )
      {
        case 0:  pi.status().setBroken(); break;
        case 1:  pi.status().setSatisfied(); break;
        case -1: pi.status().setNonRelevant(); break;
        default: pi.status().setUndetermined(); break;
      }
    }
    MIL << "Establish DONE" << endl;
  }
  else
    MIL << "Establish not needed." << endl;
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

//---------------------------------------------------------------------------

std::ostream &
SATResolver::dumpOn( std::ostream & os ) const
{
    os << "<resolver>" << endl;
    if (_satSolver) {
#define OUTS(X) os << "  " << #X << "\t= " << solver_get_flag(_satSolver, SOLVER_FLAG_##X) << endl
        OUTS( ALLOW_DOWNGRADE );
        OUTS( ALLOW_ARCHCHANGE );
        OUTS( ALLOW_VENDORCHANGE );
        OUTS( ALLOW_NAMECHANGE );
        OUTS( ALLOW_UNINSTALL );
        OUTS( NO_UPDATEPROVIDE );
        OUTS( SPLITPROVIDES );
        OUTS( ONLY_NAMESPACE_RECOMMENDED );
        OUTS( ADD_ALREADY_RECOMMENDED );
        OUTS( NO_INFARCHCHECK );
        OUTS( KEEP_EXPLICIT_OBSOLETES );
        OUTS( BEST_OBEY_POLICY );
        OUTS( NO_AUTOTARGET );
        OUTS( DUP_ALLOW_DOWNGRADE );
        OUTS( DUP_ALLOW_ARCHCHANGE );
        OUTS( DUP_ALLOW_VENDORCHANGE );
        OUTS( DUP_ALLOW_NAMECHANGE );
        OUTS( KEEP_ORPHANS );
        OUTS( BREAK_ORPHANS );
        OUTS( YUM_OBSOLETES );
#undef OUTS
        os << "  focus	= "	<< _focus << endl;
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

// NOTE: flag defaults must be in sync with ZVARDEFAULT in Resolver.cc
SATResolver::SATResolver (const ResPool & pool, sat::detail::CPool *satPool)
    : _pool(pool)
    , _satPool(satPool)
    , _satSolver(NULL)
    , _focus			( ZConfig::instance().solver_focus() )
    , _fixsystem(false)
    , _allowdowngrade		( false )
    , _allownamechange		( true )	// bsc#1071466
    , _allowarchchange		( false )
    , _allowvendorchange	( ZConfig::instance().solver_allowVendorChange() )
    , _allowuninstall		( false )
    , _updatesystem(false)
    , _noupdateprovide		( false )
    , _dosplitprovides		( true )
    , _onlyRequires		(ZConfig::instance().solver_onlyRequires())
    , _ignorealreadyrecommended(true)
    , _distupgrade(false)
    , _distupgrade_removeunsupported(false)
    , _dup_allowdowngrade	( ZConfig::instance().solver_dupAllowDowngrade() )
    , _dup_allownamechange	( ZConfig::instance().solver_dupAllowNameChange() )
    , _dup_allowarchchange	( ZConfig::instance().solver_dupAllowArchChange() )
    , _dup_allowvendorchange	( ZConfig::instance().solver_dupAllowVendorChange() )
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
        XDEBUG("SATSolutionToPool install returns " << item << ", " << r);
    }
    else if (status.isToBeUninstalledDueToUpgrade()) {
        r = item.status().setToBeUninstalledDueToUpgrade (causer);
        XDEBUG("SATSolutionToPool upgrade returns " << item << ", " <<  r);
    }
    else if (status.isToBeUninstalled()) {
        r = item.status().setToBeUninstalled (causer);
        XDEBUG("SATSolutionToPool remove returns " << item << ", " <<  r);
    }

    return;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// solverInit
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////
/// \class SATCollectTransact
/// \brief Commit helper functor distributing PoolItem by status into lists
///
/// On the fly it clears all PoolItem bySolver/ByApplLow status.
/// The lists are cleared in the Ctor, populated by \ref operator().
/////////////////////////////////////////////////////////////////////////
struct SATCollectTransact
{
  SATCollectTransact( PoolItemList & items_to_install_r,
                      PoolItemList & items_to_remove_r,
                      PoolItemList & items_to_lock_r,
                      PoolItemList & items_to_keep_r,
                      bool solveSrcPackages_r )
  : _items_to_install( items_to_install_r )
  , _items_to_remove( items_to_remove_r )
  , _items_to_lock( items_to_lock_r )
  , _items_to_keep( items_to_keep_r )
  , _solveSrcPackages( solveSrcPackages_r )
  {
    _items_to_install.clear();
    _items_to_remove.clear();
    _items_to_lock.clear();
    _items_to_keep.clear();
  }

  bool operator()( const PoolItem & item_r )
  {

    ResStatus & itemStatus( item_r.status() );
    bool by_solver = ( itemStatus.isBySolver() || itemStatus.isByApplLow() );

    if ( by_solver )
    {
      // Clear former solver/establish resultd
      itemStatus.resetTransact( ResStatus::APPL_LOW );
      return true;	// -> back out here, don't re-queue former results
    }

    if ( !_solveSrcPackages && item_r.isKind<SrcPackage>() )
    {
      // Later we may continue on a per source package base.
      return true; // dont process this source package.
    }

    switch ( itemStatus.getTransactValue() )
    {
      case ResStatus::TRANSACT:
        itemStatus.isUninstalled() ?	_items_to_install.push_back( item_r )
                                   :	_items_to_remove.push_back( item_r );	break;
      case ResStatus::LOCKED:		_items_to_lock.push_back( item_r );	break;
      case ResStatus::KEEP_STATE:	_items_to_keep.push_back( item_r );	break;
    }
    return true;
  }

private:
  PoolItemList & _items_to_install;
  PoolItemList & _items_to_remove;
  PoolItemList & _items_to_lock;
  PoolItemList & _items_to_keep;
  bool _solveSrcPackages;

};
/////////////////////////////////////////////////////////////////////////

void
SATResolver::solverEnd()
{
  // cleanup
  if ( _satSolver )
  {
    solver_free(_satSolver);
    _satSolver = NULL;
    queue_free( &(_jobQueue) );
  }
}

void
SATResolver::solverInit(const PoolItemList & weakItems)
{
    MIL << "SATResolver::solverInit()" << endl;

    // Remove old stuff and create a new jobqueue
    solverEnd();
    _satSolver = solver_create( _satPool );
    queue_init( &_jobQueue );

    {
      // bsc#1182629: in dup allow an available -release package providing 'dup-vendor-relax(suse)'
      // to let (suse/opensuse) vendor being treated as being equivalent.
      bool toRelax = false;
      if ( _distupgrade ) {
        for ( sat::Solvable solv : sat::WhatProvides( Capability("dup-vendor-relax(suse)") ) ) {
          if ( ! solv.isSystem() ) {
            MIL << "Relaxed vendor check requested by " << solv << endl;
            toRelax = true;
            break;
          }
        }
      }
      ::pool_set_custom_vendorcheck( _satPool, toRelax ? &relaxedVendorCheck : &vendorCheck );
    }

    // Add rules for user/auto installed packages
    ::pool_add_userinstalled_jobs(_satPool, sat::Pool::instance().autoInstalled(), &(_jobQueue), GET_USERINSTALLED_NAMES|GET_USERINSTALLED_INVERTED);

    // Collect PoolItem's tasks and cleanup Pool for solving.
    // Todos are kept in _items_to_install, _items_to_remove, _items_to_lock, _items_to_keep
    {
      SATCollectTransact collector( _items_to_install, _items_to_remove, _items_to_lock, _items_to_keep, solveSrcPackages() );
      invokeOnEach ( _pool.begin(), _pool.end(), std::ref( collector ) );
    }

    // Add rules for previous ProblemSolutions "break %s by ignoring some of its dependencies"
    for (PoolItemList::const_iterator iter = weakItems.begin(); iter != weakItems.end(); iter++) {
        Id id = iter->id();
        if (id == ID_NULL) {
            ERR << "Weaken: " << *iter << " not found" << endl;
        }
        MIL << "Weaken dependencies of " << *iter << endl;
        queue_push( &(_jobQueue), SOLVER_WEAKENDEPS | SOLVER_SOLVABLE );
        queue_push( &(_jobQueue), id );
    }

    // Add rules for retracted patches and packages
    {
      queue_push( &(_jobQueue), SOLVER_BLACKLIST|SOLVER_SOLVABLE_PROVIDES );
      queue_push( &(_jobQueue), sat::Solvable::retractedToken.id() );
      queue_push( &(_jobQueue), SOLVER_BLACKLIST|SOLVER_SOLVABLE_PROVIDES );
      queue_push( &(_jobQueue), sat::Solvable::ptfMasterToken.id() );
      // bsc#1186503: ptfPackageToken should not be blacklisted
    }

    // Add rules for changed requestedLocales
    {
      const auto & trackedLocaleIds( myPool().trackedLocaleIds() );

      // just track changed locakes
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
    }

    // Add rules for parallel installable resolvables with different versions
    for ( const sat::Solvable & solv : myPool().multiversionList() )
    {
      queue_push( &(_jobQueue), SOLVER_NOOBSOLETES | SOLVER_SOLVABLE );
      queue_push( &(_jobQueue), solv.id() );
    }

    // Add rules to protect PTF removal without repos (bsc#1203248)
    // Removing a PTF its packages should be replaced by the official
    // versions again. If just the system repo is present, they'd get
    // removed instead.
    {
      _protectPTFs = sat::Pool::instance().reposSize() == 1;
      if ( _protectPTFs ) {
        for ( const auto & solv : sat::AllPTFs() ) {
          if ( solv.isSystem() ) {
            queue_push( &(_jobQueue), SOLVER_INSTALL | SOLVER_SOLVABLE );
            queue_push( &(_jobQueue), solv.id() );
          }
        }
      }
    }

    // set requirements for a running system
    solverInitSetSystemRequirements();

    // set locks for the solver
    solverInitSetLocks();

    // set mode (verify,up,dup) specific jobs and solver flags
    solverInitSetModeJobsAndFlags();
}

void SATResolver::solverInitSetSystemRequirements()
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

void SATResolver::solverInitSetLocks()
{
    unsigned icnt = 0;
    unsigned acnt = 0;

    for (PoolItemList::const_iterator iter = _items_to_lock.begin(); iter != _items_to_lock.end(); ++iter) {
        sat::detail::SolvableIdType id( iter->id() );
        if (iter->status().isInstalled()) {
            ++icnt;
            queue_push( &(_jobQueue), SOLVER_INSTALL | SOLVER_SOLVABLE );
            queue_push( &(_jobQueue), id );
        } else {
            ++acnt;
            queue_push( &(_jobQueue), SOLVER_ERASE | SOLVER_SOLVABLE | MAYBE_CLEANDEPS );
            queue_push( &(_jobQueue), id );
        }
    }
    MIL << "Locked " << icnt << " installed items and " << acnt << " NOT installed items." << endl;

    ///////////////////////////////////////////////////////////////////
    // Weak locks: Ignore if an item with this name is already installed.
    // If it's not installed try to keep it this way using a weak delete
    ///////////////////////////////////////////////////////////////////
    std::set<IdString> unifiedByName;
    for (PoolItemList::const_iterator iter = _items_to_keep.begin(); iter != _items_to_keep.end(); ++iter) {
      IdString ident( iter->ident() );
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

void SATResolver::solverInitSetModeJobsAndFlags()
{
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

    solverSetFocus( *_satSolver, _focus );
    solver_set_flag(_satSolver, SOLVER_FLAG_ADD_ALREADY_RECOMMENDED, !_ignorealreadyrecommended);
    solver_set_flag(_satSolver, SOLVER_FLAG_ALLOW_DOWNGRADE,		_allowdowngrade);
    solver_set_flag(_satSolver, SOLVER_FLAG_ALLOW_NAMECHANGE,		_allownamechange);
    solver_set_flag(_satSolver, SOLVER_FLAG_ALLOW_ARCHCHANGE,		_allowarchchange);
    solver_set_flag(_satSolver, SOLVER_FLAG_ALLOW_VENDORCHANGE,		_allowvendorchange);
    solver_set_flag(_satSolver, SOLVER_FLAG_ALLOW_UNINSTALL,		_allowuninstall);
    solver_set_flag(_satSolver, SOLVER_FLAG_NO_UPDATEPROVIDE,		_noupdateprovide);
    solver_set_flag(_satSolver, SOLVER_FLAG_SPLITPROVIDES,		_dosplitprovides);
    solver_set_flag(_satSolver, SOLVER_FLAG_IGNORE_RECOMMENDED, 	false);		// resolve recommended namespaces
    solver_set_flag(_satSolver, SOLVER_FLAG_ONLY_NAMESPACE_RECOMMENDED,	_onlyRequires);	//
    solver_set_flag(_satSolver, SOLVER_FLAG_DUP_ALLOW_DOWNGRADE,	_dup_allowdowngrade );
    solver_set_flag(_satSolver, SOLVER_FLAG_DUP_ALLOW_NAMECHANGE,	_dup_allownamechange );
    solver_set_flag(_satSolver, SOLVER_FLAG_DUP_ALLOW_ARCHCHANGE,	_dup_allowarchchange );
    solver_set_flag(_satSolver, SOLVER_FLAG_DUP_ALLOW_VENDORCHANGE,	_dup_allowvendorchange );
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// solving.....
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

class CheckIfUpdate
{
  public:
    bool is_updated;
    sat::Solvable _installed;

    CheckIfUpdate( const sat::Solvable & installed_r )
    : is_updated( false )
    , _installed( installed_r )
    {}

    // check this item will be updated

    bool operator()( const PoolItem & item )
    {
        if ( item.status().isToBeInstalled() )
        {
          if ( ! item.multiversionInstall() || sameNVRA( _installed, item ) )
          {
            is_updated = true;
            return false;
          }
        }
        return true;
    }
};


bool
SATResolver::solving(const CapabilitySet & requires_caps,
                     const CapabilitySet & conflict_caps)
{
    sat::Pool::instance().prepare();

    // Solve !
    MIL << "Starting solving...." << endl;
    MIL << *this;
    if ( solver_solve( _satSolver, &(_jobQueue) ) == 0 )
    {
      // bsc#1155819: Weakremovers of future product not evaluated.
      // Do a 2nd run to cleanup weakremovers() of to be installed
      // Produtcs unless removeunsupported is active (cleans up all).
      if ( _distupgrade )
      {
        if ( _distupgrade_removeunsupported )
          MIL << "Droplist processing not needed. RemoveUnsupported is On." << endl;
        else if ( ! ZConfig::instance().solverUpgradeRemoveDroppedPackages() )
          MIL << "Droplist processing is disabled in ZConfig." << endl;
        else
        {
          bool resolve = false;
          MIL << "Checking droplists ..." << endl;
          // get Solvables to be installed...
          sat::SolvableQueue decisionq;
          solver_get_decisionqueue( _satSolver, decisionq );
          for ( sat::detail::IdType id : decisionq )
          {
            if ( id < 0 )
              continue;
            sat::Solvable slv { (sat::detail::SolvableIdType)id };
            // get product buddies (they carry the weakremover)...
            static const Capability productCap { "product()" };
            if ( slv && slv.provides().matches( productCap ) )
            {
              CapabilitySet droplist { slv.valuesOfNamespace( "weakremover" ) };
              MIL << "Droplist for " << slv << ": size " << droplist.size() << endl;
              if ( !droplist.empty() )
              {
                for ( const auto & cap : droplist )
                {
                  queue_push( &_jobQueue, SOLVER_DROP_ORPHANED | SOLVER_SOLVABLE_NAME );
                  queue_push( &_jobQueue, cap.id() );
                }
                // PIN product - a safety net to prevent cleanup from changing the decision for this product
                queue_push( &(_jobQueue), SOLVER_INSTALL | SOLVER_SOLVABLE );
                queue_push( &(_jobQueue), id );
                resolve = true;
              }
            }
          }
          if ( resolve )
            solver_solve( _satSolver, &(_jobQueue) );
        }
      }
    }
    MIL << "....Solver end" << endl;

    // copying solution back to zypp pool
    //-----------------------------------------
    _result_items_to_install.clear();
    _result_items_to_remove.clear();

    /*  solvables to be installed */
    Queue decisionq;
    queue_init(&decisionq);
    solver_get_decisionqueue(_satSolver, &decisionq);
    for ( int i = 0; i < decisionq.count; ++i )
    {
      Id p = decisionq.elements[i];
      if ( p < 0 )
        continue;

      sat::Solvable slv { (sat::detail::SolvableIdType)p };
      if ( ! slv || slv.isSystem() )
        continue;

      PoolItem poolItem( slv );
      SATSolutionToPool (poolItem, ResStatus::toBeInstalled, ResStatus::SOLVER);
      _result_items_to_install.push_back( poolItem );
    }
    queue_free(&decisionq);

    /* solvables to be erased */
    Repository systemRepo( sat::Pool::instance().findSystemRepo() ); // don't create if it does not exist
    if ( systemRepo && ! systemRepo.solvablesEmpty() )
    {
      bool mustCheckObsoletes = false;
      for_( it, systemRepo.solvablesBegin(), systemRepo.solvablesEnd() )
      {
        if (solver_get_decisionlevel(_satSolver, it->id()) > 0)
          continue;

        // Check if this is an update
        CheckIfUpdate info( *it );
        PoolItem poolItem( *it );
        invokeOnEach( _pool.byIdentBegin( poolItem ),
                      _pool.byIdentEnd( poolItem ),
                      resfilter::ByUninstalled(),			// ByUninstalled
                      std::ref(info) );

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

    // copy back computed status values to pool
    // (on the fly cache orphaned items for the UI)
    solverCopyBackWeak( *_satSolver, _problem_items );
    solverCopyBackValidate( *_satSolver, _pool );

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

    if (solver_problem_count(_satSolver) > 0 )
    {
        ERR << "Solverrun finished with an ERROR" << endl;
        return false;
    }

    return true;
}

void SATResolver::solverAddJobsFromPool()
{
  for (PoolItemList::const_iterator iter = _items_to_install.begin(); iter != _items_to_install.end(); iter++) {
    Id id = iter->id();
    if (id == ID_NULL) {
      ERR << "Install: " << *iter << " not found" << endl;
    } else {
      MIL << "Install " << *iter << endl;
      queue_push( &(_jobQueue), SOLVER_INSTALL | SOLVER_SOLVABLE );
      queue_push( &(_jobQueue), id );
    }
  }

  for (PoolItemList::const_iterator iter = _items_to_remove.begin(); iter != _items_to_remove.end(); iter++) {
    Id id = iter->id();
    if (id == ID_NULL) {
      ERR << "Delete: " << *iter << " not found" << endl;
    } else {
      MIL << "Delete " << *iter << endl;
      queue_push( &(_jobQueue), SOLVER_ERASE | SOLVER_SOLVABLE | MAYBE_CLEANDEPS );
      queue_push( &(_jobQueue), id);
    }
  }
}

void SATResolver::solverAddJobsFromExtraQueues( const CapabilitySet & requires_caps, const CapabilitySet & conflict_caps )
{
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
}

bool
SATResolver::resolvePool(const CapabilitySet & requires_caps,
                         const CapabilitySet & conflict_caps,
                         const PoolItemList & weakItems,
                         const std::set<Repository> & upgradeRepos)
{
    MIL << "SATResolver::resolvePool()" << endl;

    // Initialize
    solverInit(weakItems);

    // Add pool and extra jobs.
    solverAddJobsFromPool();
    solverAddJobsFromExtraQueues( requires_caps, conflict_caps );
    // 'dup --from' jobs
    for_( iter, upgradeRepos.begin(), upgradeRepos.end() )
    {
        queue_push( &(_jobQueue), SOLVER_DISTUPGRADE | SOLVER_SOLVABLE_REPO );
        queue_push( &(_jobQueue), iter->get()->repoid );
        MIL << "Upgrade repo " << *iter << endl;
    }

    // Solve!
    bool ret = solving(requires_caps, conflict_caps);

    (ret?MIL:WAR) << "SATResolver::resolvePool() done. Ret:" << ret <<  endl;
    return ret;
}


bool
SATResolver::resolveQueue(const SolverQueueItemList &requestQueue,
                          const PoolItemList & weakItems)
{
    MIL << "SATResolver::resolvQueue()" << endl;

    // Initialize
    solverInit(weakItems);

    // Add request queue's jobs.
    for (SolverQueueItemList::const_iterator iter = requestQueue.begin(); iter != requestQueue.end(); iter++) {
        (*iter)->addRule(_jobQueue);
    }

    // Add pool jobs; they do contain any problem resolutions.
    solverAddJobsFromPool();

    // Solve!
    bool ret = solving();

    (ret?MIL:WAR) << "SATResolver::resolveQueue() done. Ret:" << ret <<  endl;
    return ret;
}


void SATResolver::doUpdate()
{
    MIL << "SATResolver::doUpdate()" << endl;

    // Initialize
    solverInit(PoolItemList());

    // By now, doUpdate has no additional jobs.
    // It does not include any pool jobs, and so it does not create an conflicts.
    // Combinations like patch_with_update are driven by resolvePool + _updatesystem.

    // TODO: Try to join the following with solving()
    sat::Pool::instance().prepare();

    // Solve!
    MIL << "Starting solving for update...." << endl;
    MIL << *this;
    solver_solve( _satSolver, &(_jobQueue) );
    MIL << "....Solver end" << endl;

    // copying solution back to zypp pool
    //-----------------------------------------

    /*  solvables to be installed */
    Queue decisionq;
    queue_init(&decisionq);
    solver_get_decisionqueue(_satSolver, &decisionq);
    for (int i = 0; i < decisionq.count; i++)
    {
      Id p = decisionq.elements[i];
      if ( p < 0 )
        continue;

      sat::Solvable solv { (sat::detail::SolvableIdType)p };
      if ( ! solv || solv.isSystem() )
        continue;

      SATSolutionToPool( PoolItem(solv), ResStatus::toBeInstalled, ResStatus::SOLVER );
    }
    queue_free(&decisionq);

    /* solvables to be erased */
    if ( _satSolver->pool->installed ) {
      for (int i = _satSolver->pool->installed->start; i < _satSolver->pool->installed->start + _satSolver->pool->installed->nsolvables; i++)
      {
        if (solver_get_decisionlevel(_satSolver, i) > 0)
            continue;

        PoolItem poolItem( _pool.find( sat::Solvable(i) ) );
        if (poolItem) {
            // Check if this is an update
            CheckIfUpdate info( (sat::Solvable(i)) );
            invokeOnEach( _pool.byIdentBegin( poolItem ),
                          _pool.byIdentEnd( poolItem ),
                          resfilter::ByUninstalled(),			// ByUninstalled
                          std::ref(info) );

            if (info.is_updated) {
                SATSolutionToPool (poolItem, ResStatus::toBeUninstalledDueToUpgrade , ResStatus::SOLVER);
            } else {
                SATSolutionToPool (poolItem, ResStatus::toBeUninstalled, ResStatus::SOLVER);
            }
        } else {
            ERR << "id " << i << " not found in ZYPP pool." << endl;
        }
      }
    }

    // copy back computed status values to pool
    // (on the fly cache orphaned items for the UI)
    solverCopyBackWeak( *_satSolver, _problem_items );
    solverCopyBackValidate( *_satSolver, _pool );

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

struct FindPackage
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
  if ( item_r.isKind<Package>() )
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

std::vector<std::string> SATResolver::SATgetCompleteProblemInfoStrings ( Id problem )
{
  std::vector<std::string> ret;
  sat::Queue problems;
  solver_findallproblemrules( _satSolver, problem, problems );

  bool nobad = false;

  //filter out generic rule information if more explicit ones are available
  for ( sat::Queue::size_type i = 0; i < problems.size(); i++ ) {
    SolverRuleinfo ruleClass = solver_ruleclass( _satSolver, problems[i]);
    if ( ruleClass != SolverRuleinfo::SOLVER_RULE_UPDATE && ruleClass != SolverRuleinfo::SOLVER_RULE_JOB ) {
      nobad = true;
      break;
    }
  }
  for ( sat::Queue::size_type i = 0; i < problems.size(); i++ ) {
    SolverRuleinfo ruleClass = solver_ruleclass( _satSolver, problems[i]);
    if ( nobad && ( ruleClass == SolverRuleinfo::SOLVER_RULE_UPDATE || ruleClass == SolverRuleinfo::SOLVER_RULE_JOB ) ) {
      continue;
    }

    std::string detail;
    Id ignore = 0;
    std::string pInfo = SATproblemRuleInfoString( problems[i], detail, ignore );

    //we get the same string multiple times, reduce the noise
    if ( std::find( ret.begin(), ret.end(), pInfo ) == ret.end() )
      ret.push_back( pInfo );
  }
  return ret;
}

std::string SATResolver::SATprobleminfoString(Id problem, std::string &detail, Id &ignoreId)
{
  // FIXME: solver_findallproblemrules to get all rules for this problem
  // (the 'most relevabt' one returned by solver_findproblemrule is embedded
  Id probr = solver_findproblemrule(_satSolver, problem);
  return SATproblemRuleInfoString( probr, detail, ignoreId );
}

std::string SATResolver::SATproblemRuleInfoString (Id probr, std::string &detail, Id &ignoreId)
{
  std::string ret;
  sat::detail::CPool *pool = _satSolver->pool;
  Id dep, source, target;
  SolverRuleinfo type = solver_ruleinfo(_satSolver, probr, &source, &target, &dep);

  ignoreId = 0;

  sat::Solvable s = mapSolvable( source );
  sat::Solvable s2 = mapSolvable( target );

  // @FIXME, these strings are a duplicate copied from the libsolv library
  // to provide translations. Instead of having duplicate code we should
  // translate those strings directly in libsolv
  switch ( type )
  {
      case SOLVER_RULE_DISTUPGRADE:
          if ( s.isSystem() )
            ret = str::Format(_("the installed %1% does not belong to a distupgrade repository and must be replaced") ) % s.asString();
          else /*just in case*/
            ret = str::Format(_("the to be installed %1% does not belong to a distupgrade repository") ) % s.asString();
          break;
      case SOLVER_RULE_INFARCH:
          if ( s.isSystem() )
            ret = str::Format(_("the installed %1% has inferior architecture") ) % s.asString();
          else
            ret = str::Format(_("the to be installed %1% has inferior architecture") ) % s.asString();
          break;
      case SOLVER_RULE_UPDATE:
          ret = str::Format(_("problem with the installed %1%") ) % s.asString();
          break;
      case SOLVER_RULE_JOB:
          ret = _("conflicting requests");
          break;
      case SOLVER_RULE_PKG:
          ret = _("some dependency problem");
          break;
      case SOLVER_RULE_JOB_NOTHING_PROVIDES_DEP:
          ret = str::Format(_("nothing provides the requested '%1%'") ) % pool_dep2str(pool, dep);
          detail += _("Have you enabled all the required repositories?");
          break;
      case SOLVER_RULE_JOB_UNKNOWN_PACKAGE:
          ret = str::Format(_("the requested package %1% does not exist") ) % pool_dep2str(pool, dep);
          detail += _("Have you enabled all the required repositories?");
          break;
      case SOLVER_RULE_JOB_UNSUPPORTED:
          ret = _("unsupported request");
          break;
      case SOLVER_RULE_JOB_PROVIDED_BY_SYSTEM:
          ret = str::Format(_("'%1%' is provided by the system and cannot be erased") ) % pool_dep2str(pool, dep);
          break;
      case SOLVER_RULE_PKG_NOT_INSTALLABLE:
          ret = str::Format(_("%1% is not installable") ) % s.asString();
          break;
      case SOLVER_RULE_PKG_NOTHING_PROVIDES_DEP:
          ignoreId = source; // for setting weak dependencies
          if ( s.isSystem() )
            ret = str::Format(_("nothing provides '%1%' needed by the installed %2%") ) % pool_dep2str(pool, dep) % s.asString();
          else
            ret = str::Format(_("nothing provides '%1%' needed by the to be installed %2%") ) % pool_dep2str(pool, dep) % s.asString();
          break;
      case SOLVER_RULE_PKG_SAME_NAME:
          ret = str::Format(_("cannot install both %1% and %2%") ) % s.asString() % s2.asString();
          break;
      case SOLVER_RULE_PKG_CONFLICTS:
          if ( s.isSystem() ) {
            if ( s2.isSystem() )
              ret = str::Format(_("the installed %1% conflicts with '%2%' provided by the installed %3%") ) % s.asString() % pool_dep2str(pool, dep) % s2.asString();
            else
              ret = str::Format(_("the installed %1% conflicts with '%2%' provided by the to be installed %3%") ) % s.asString() % pool_dep2str(pool, dep) % s2.asString();
          }
          else {
            if ( s2.isSystem() )
              ret = str::Format(_("the to be installed %1% conflicts with '%2%' provided by the installed %3%") ) % s.asString() % pool_dep2str(pool, dep) % s2.asString();
            else
              ret = str::Format(_("the to be installed %1% conflicts with '%2%' provided by the to be installed %3%") ) % s.asString() % pool_dep2str(pool, dep) % s2.asString();
          }
          break;
      case SOLVER_RULE_PKG_OBSOLETES:
      case SOLVER_RULE_PKG_INSTALLED_OBSOLETES:
          if ( s.isSystem() ) {
            if ( s2.isSystem() )
              ret = str::Format(_("the installed %1% obsoletes '%2%' provided by the installed %3%") ) % s.asString() % pool_dep2str(pool, dep) % s2.asString();
            else
              ret = str::Format(_("the installed %1% obsoletes '%2%' provided by the to be installed %3%") ) % s.asString() % pool_dep2str(pool, dep) % s2.asString();
          }
          else {
            if ( s2.isSystem() )
              ret = str::Format(_("the to be installed %1% obsoletes '%2%' provided by the installed %3%") ) % s.asString() % pool_dep2str(pool, dep) % s2.asString();
            else
              ret = str::Format(_("the to be installed %1% obsoletes '%2%' provided by the to be installed %3%") ) % s.asString() % pool_dep2str(pool, dep) % s2.asString();
          }
          break;
      case SOLVER_RULE_PKG_SELF_CONFLICT:
          if ( s.isSystem() )
            ret = str::Format(_("the installed %1% conflicts with '%2%' provided by itself") ) % s.asString() % pool_dep2str(pool, dep);
          else
            ret = str::Format(_("the to be installed %1% conflicts with '%2%' provided by itself") ) % s.asString() % pool_dep2str(pool, dep);
          break;
      case SOLVER_RULE_PKG_REQUIRES: {
          ignoreId = source; // for setting weak dependencies
          Capability cap(dep);
          sat::WhatProvides possibleProviders(cap);

          // check, if a provider will be deleted
          typedef std::list<PoolItem> ProviderList;
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

          if ( s.isSystem() )
            ret = str::Format(_("the installed %1% requires '%2%', but this requirement cannot be provided") ) % s.asString() % pool_dep2str(pool, dep);
          else
            ret = str::Format(_("the to be installed %1% requires '%2%', but this requirement cannot be provided") ) % s.asString() % pool_dep2str(pool, dep);
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
                  detail += _("\nnot installable providers: ");
              else
                  detail = _("not installable providers: ");
              for (ProviderList::const_iterator iter = providerlistUninstalled.begin(); iter != providerlistUninstalled.end(); iter++) {
                  if (iter == providerlistUninstalled.begin())
                      detail += itemToString( *iter );
                  else
                      detail += "\n                   " + itemToString( mapItem(*iter) );
              }
          }
          break;
      }
      default: {
          DBG << "Unknown rule type(" << type << ") going to query libsolv for rule information." << endl;
          ret = str::asString( ::solver_problemruleinfo2str( _satSolver, type, static_cast<Id>(s.id()), static_cast<Id>(s2.id()), dep ) );
          break;
      }
  }
  return ret;
}

/////////////////////////////////////////////////////////////////////////
namespace {
  /// bsc#1194848 hint on ptf<>patch conflicts or common ptf conflicts
  struct PtfPatchHint
  {
    void notInstallPatch( sat::Solvable slv_r )
    { _patch.push_back( slv_r.ident() ); }

    void removePtf(  sat::Solvable slv_r, bool showremoveProtectHint_r = false )
    { _ptf.push_back( slv_r.ident() ); if ( showremoveProtectHint_r ) _showremoveProtectHint = true; }

    bool applies() const
    { return not _ptf.empty(); }

    std::string description() const {
      if ( not _patch.empty() ) {
        return str::Str()
        // translator: %1% is the name of a PTF, %2% the name of a patch.
        << (str::Format( _("%1% is not yet fully integrated into %2%.") ) % printlist(_ptf) % printlist(_patch)) << endl
        << _("Typically you want to keep the PTF and choose to not install the maintenance patches.");
      }
      //else: a common problem due to an installed ptf

      if ( _showremoveProtectHint ) { // bsc#1203248
        const std::string & removeptfCommand { str::Format("zypper removeptf %1%") % printlist(_ptf) };
        return str::Str()
        // translator: %1% is the name of a PTF.
        << (str::Format( _("Removing the installed %1% in this context will remove (not replace!) the included PTF-packages too." ) ) % printlist(_ptf)) << endl
        << (str::Format( _("The PTF should be removed by calling '%1%'. This will update the included PTF-packages rather than removing them." ) ) % removeptfCommand) << endl
        << _("Typically you want to keep the PTF or choose to cancel the action."); // ma: When translated, it should replace the '..and choose..' below too
      }

      return str::Str()
      // translator: %1% is the name of a PTF.
      << (str::Format( _("The installed %1% blocks the desired action.") ) % printlist(_ptf)) << endl
      << _("Typically you want to keep the PTF and choose to cancel the action.");
    }
  private:
    using StoreType = IdString;
    static std::string printlist( const std::vector<StoreType> & list_r )
    { str::Str ret; dumpRange( ret.stream(), list_r.begin(), list_r.end(), "", "", ", ", "", "" ); return ret; }

    std::vector<StoreType> _ptf;
    std::vector<StoreType> _patch;
    bool _showremoveProtectHint = false;
  };
}
/////////////////////////////////////////////////////////////////////////

ResolverProblemList
SATResolver::problems ()
{
    ResolverProblemList resolverProblems;
    if (_satSolver && solver_problem_count(_satSolver)) {
        sat::detail::CPool *pool = _satSolver->pool;
        int pcnt;
        Id p, rp, what;
        Id problem, solution, element;
        sat::Solvable s, sd;

        CapabilitySet system_requires = SystemCheck::instance().requiredSystemCap();
        CapabilitySet system_conflicts = SystemCheck::instance().conflictSystemCap();

        MIL << "Encountered problems! Here are the solutions:\n" << endl;
        pcnt = 1;
        problem = 0;
        while ((problem = solver_next_problem(_satSolver, problem)) != 0) {
            MIL << "Problem " <<  pcnt++ << ":" << endl;
            MIL << "====================================" << endl;
            std::string detail;
            Id ignoreId;
            std::string whatString = SATprobleminfoString (problem,detail,ignoreId);
            MIL << whatString << endl;
            MIL << "------------------------------------" << endl;
            ResolverProblem_Ptr resolverProblem = new ResolverProblem (whatString, detail, SATgetCompleteProblemInfoStrings( problem ));
            PtfPatchHint ptfPatchHint;  // bsc#1194848 hint on ptf<>patch conflicts
            solution = 0;
            while ((solution = solver_next_solution(_satSolver, problem, solution)) != 0) {
                element = 0;
                ProblemSolutionCombi *problemSolution = new ProblemSolutionCombi;
                while ((element = solver_next_solutionelement(_satSolver, problem, solution, element, &p, &rp)) != 0) {
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
                                        std::string description = str::Format(_("remove lock to allow removal of %1%") ) % s.asString();
                                        MIL << description << endl;
                                        problemSolution->addDescription (description);
                                        if ( _protectPTFs && s.isPtfMaster() )
                                          ptfPatchHint.removePtf( s, _protectPTFs ); // bsc#1203248
                                    } else {
                                        problemSolution->addSingleAction (poolItem, KEEP);
                                        std::string description = str::Format(_("do not install %1%") ) % s.asString();
                                        MIL << description << endl;
                                        problemSolution->addDescription (description);
                                        if ( s.isKind<Patch>() )
                                          ptfPatchHint.notInstallPatch( s );
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
                                        std::string description = str::Format(_("keep %1%") ) % s.asString();
                                        MIL << description << endl;
                                        problemSolution->addDescription (description);
                                    } else {
                                        problemSolution->addSingleAction (poolItem, UNLOCK);
                                        std::string description = str::Format(_("remove lock to allow installation of %1%") ) % itemToString( poolItem );
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

                                std::string description = str::Format(_("do not install %1%") ) % ident;
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
                                              std::ref(info) );

                                SolverQueueItemDelete_Ptr del =
                                    new SolverQueueItemDelete(_pool, ident.asString(), false );
                                problemSolution->addSingleAction (del, REMOVE_SOLVE_QUEUE_ITEM);

                                std::string description = str::Format(_("keep %1%") ) % ident;
                                MIL << description << endl;
                                problemSolution->addDescription (description);
                                }
                                break;
                            case SOLVER_INSTALL | SOLVER_SOLVABLE_PROVIDES:
                                {
                                problemSolution->addSingleAction (Capability(what), REMOVE_EXTRA_REQUIRE);
                                std::string description = "";

                                // Checking if this problem solution would break your system
                                if (system_requires.find(Capability(what)) != system_requires.end()) {
                                    // Show a better warning
                                    resolverProblem->setDetails( resolverProblem->description() + "\n" + resolverProblem->details() );
                                    resolverProblem->setDescription(_("This request will break your system!"));
                                    description = _("ignore the warning of a broken system");
                                    description += std::string(" (requires:")+pool_dep2str(pool, what)+")";
                                    MIL << description << endl;
                                    problemSolution->addFrontDescription (description);
                                } else {
                                    description = str::Format(_("do not ask to install a solvable providing %1%") ) % pool_dep2str(pool, what);
                                    MIL << description << endl;
                                    problemSolution->addDescription (description);
                                }
                                }
                                break;
                            case SOLVER_ERASE | SOLVER_SOLVABLE_PROVIDES:
                                {
                                problemSolution->addSingleAction (Capability(what), REMOVE_EXTRA_CONFLICT);
                                std::string description = "";

                                // Checking if this problem solution would break your system
                                if (system_conflicts.find(Capability(what)) != system_conflicts.end()) {
                                    // Show a better warning
                                    resolverProblem->setDetails( resolverProblem->description() + "\n" + resolverProblem->details() );
                                    resolverProblem->setDescription(_("This request will break your system!"));
                                    description = _("ignore the warning of a broken system");
                                    description += std::string(" (conflicts:")+pool_dep2str(pool, what)+")";
                                    MIL << description << endl;
                                    problemSolution->addFrontDescription (description);

                                } else {
                                    description = str::Format(_("do not ask to delete all solvables providing %1%") ) % pool_dep2str(pool, what);
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
                                        std::string description = str::Format(_("do not install most recent version of %1%") ) % s.asString();
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
                            std::string description = str::Format(_("keep %1% despite the inferior architecture") ) % s.asString();
                            MIL << description << endl;
                            problemSolution->addDescription (description);
                        } else {
                            problemSolution->addSingleAction (poolItem, INSTALL);
                            std::string description = str::Format(_("install %1% despite the inferior architecture") ) % s.asString();
                            MIL << description << endl;
                            problemSolution->addDescription (description);
                        }
                    } else if (p == SOLVER_SOLUTION_DISTUPGRADE) {
                        s = mapSolvable (rp);
                        PoolItem poolItem = _pool.find (s);
                        if (pool->installed && s.get()->repo == pool->installed) {
                            problemSolution->addSingleAction (poolItem, LOCK);
                            std::string description = str::Format(_("keep obsolete %1%") ) % s.asString();
                            MIL << description << endl;
                            problemSolution->addDescription (description);
                        } else {
                            problemSolution->addSingleAction (poolItem, INSTALL);
                            std::string description = str::Format(_("install %1% from excluded repository") ) % s.asString();
                            MIL << description << endl;
                            problemSolution->addDescription (description);
                        }
                    } else if ( p == SOLVER_SOLUTION_BLACK ) {
                        // Allow to install a blacklisted package (PTF, retracted,...).
                        // For not-installed items only
                        s = mapSolvable (rp);
                        PoolItem poolItem = _pool.find (s);

                        problemSolution->addSingleAction (poolItem, INSTALL);
                        std::string description;
                        if ( s.isRetracted() ) {
                          // translator: %1% is a package name
                          description = str::Format(_("install %1% although it has been retracted")) % s.asString();
                        } else if ( s.isPtf() ) {
                          // translator: %1% is a package name
                          description = str::Format(_("allow installing the PTF %1%")) % s.asString();
                        } else {
                          // translator: %1% is a package name
                          description = str::Format(_("install %1% although it is blacklisted")) % s.asString();
                        }
                        MIL << description << endl;
                        problemSolution->addDescription( description );
                    } else if ( p > 0 ) {
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
                                int illegal = policy_is_illegal(_satSolver, s.get(), sd.get(), 0);

                                if ((illegal & POLICY_ILLEGAL_DOWNGRADE) != 0)
                                {
                                    std::string description = str::Format(_("downgrade of %1% to %2%") ) % s.asString() % sd.asString();
                                    MIL << description << endl;
                                    problemSolution->addDescription (description);
                                    gotone = 1;
                                }
                                if ((illegal & POLICY_ILLEGAL_ARCHCHANGE) != 0)
                                {
                                    std::string description = str::Format(_("architecture change of %1% to %2%") ) % s.asString() % sd.asString();
                                    MIL << description << endl;
                                    problemSolution->addDescription (description);
                                    gotone = 1;
                                }
                                if ((illegal & POLICY_ILLEGAL_VENDORCHANGE) != 0)
                                {
                                    IdString s_vendor( s.vendor() );
                                    IdString sd_vendor( sd.vendor() );
                                    std::string description;
                                    if ( s == sd ) // FIXME? Actually .ident() must be eq. But the more verbose 'else' isn't bad either.
                                      description = str::Format(_("install %1% (with vendor change)\n  %2%  -->  %3%") )
                                      % sd.asString()
                                      % ( s_vendor ? s_vendor.c_str() : " (no vendor) " )
                                      % ( sd_vendor ? sd_vendor.c_str() : " (no vendor) " );
                                    else
                                      description = str::Format(_("install %1% from vendor %2%\n  replacing %3% from vendor %4%") )
                                      % sd.asString()  % ( sd_vendor ? sd_vendor.c_str() : " (no vendor) " )
                                      % s.asString() % ( s_vendor ? s_vendor.c_str() : " (no vendor) " );

                                    MIL << description << endl;
                                    problemSolution->addDescription (description);
                                    gotone = 1;
                                }
                                if (!gotone) {
                                    std::string description = str::Format(_("replacement of %1% with %2%") ) % s.asString() % sd.asString();
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
                                std::string description = str::Format(_("deinstallation of %1%") ) % s.asString();
                                MIL << description << endl;
                                problemSolution->addDescription (description);
                                problemSolution->addSingleAction (itemFrom, REMOVE);
                                if ( s.isPtfMaster() )
                                  ptfPatchHint.removePtf( s );
                            }
                        }
                    }
                    else
                    {
                      INT << "Unknown solution " << p << endl;
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

            // bsc#1194848 hint on ptf<>patch conflicts
            if ( ptfPatchHint.applies() ) {
              resolverProblem->setDescription( str::Str() << ptfPatchHint.description() << endl << "(" << resolverProblem->description() << ")" );
            }
            // save problem
            resolverProblems.push_back (resolverProblem);
        }
    }
    return resolverProblems;
}

void SATResolver::applySolutions( const ProblemSolutionList & solutions )
{ Resolver( _pool ).applySolutions( solutions ); }

sat::StringQueue SATResolver::autoInstalled() const
{
  sat::StringQueue ret;
  if ( _satSolver )
    ::solver_get_userinstalled( _satSolver, ret, GET_USERINSTALLED_NAMES|GET_USERINSTALLED_INVERTED );
  return ret;
}

sat::StringQueue SATResolver::userInstalled() const
{
  sat::StringQueue ret;
  if ( _satSolver )
    ::solver_get_userinstalled( _satSolver, ret, GET_USERINSTALLED_NAMES );
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
