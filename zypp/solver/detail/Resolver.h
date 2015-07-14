/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* Resolver.h
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

#ifndef ZYPP_SOLVER_DETAIL_RESOLVER_H
#define ZYPP_SOLVER_DETAIL_RESOLVER_H
#ifndef ZYPP_USE_RESOLVER_INTERNALS
#error Do not directly include this file!
#else

#include <iosfwd>
#include <string>
#include <list>
#include <map>

#include "zypp/ResPool.h"
#include "zypp/TriBool.h"
#include "zypp/base/SerialNumber.h"
#include "zypp/base/NonCopyable.h"

#include "zypp/ProblemTypes.h"
#include "zypp/ResolverProblem.h"
#include "zypp/ProblemSolution.h"
#include "zypp/Capabilities.h"
#include "zypp/Capability.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{
  namespace sat
  {
    class Transaction;
  }
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  {
    /////////////////////////////////////////////////////////////////////
    namespace detail
    {
      class SATResolver;
      typedef std::list<PoolItem> PoolItemList;
      typedef std::set<PoolItem> PoolItemSet;

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : Resolver
/** A mid layer class we should remove
 * \todo Merge this and class SATResolver. Logic and date are horribly
 * distributed between this and SATResolver. Either SATResolver becomes
 * a pure wrapper adapting the libsolv C interface to fit our needs, and
 * all the solver logic and problem handling goes here; or completely merge
 * both classes.
 */
class Resolver : private base::NonCopyable
{
  typedef std::multimap<PoolItem,ItemCapKind> ItemCapKindMap;
  private:
    ResPool _pool;
    SATResolver *_satResolver;
    SerialNumberWatcher _poolchanged;

    CapabilitySet _extra_requires;
    CapabilitySet _extra_conflicts;
    std::set<Repository> _upgradeRepos;

    // Regard dependencies of the item weak onl
    PoolItemList _addWeak;

    /** \name Solver flags */
    //@{
    bool _forceResolve;           // remove items which are conflicts with others or
                                  // have unfulfilled requirements.
                                  // This behaviour is favourited by ZMD
    bool _upgradeMode;            // Resolver has been called with doUpgrade
    bool _updateMode;            // Resolver has been called with doUpdate
    bool _verifying;              // The system will be checked
    bool _onlyRequires; 	  // do install required resolvables only
                                  // no recommended resolvables, language
                                  // packages, hardware packages (modalias)
    bool _allowVendorChange;	// whether the solver should allow or disallow vendor changes.
    bool _solveSrcPackages;	// whether to generate solver jobs for selected source packges.
    bool _cleandepsOnRemove;	// whether removing a package should also remove no longer needed requirements

    bool _ignoreAlreadyRecommended;   //ignore recommended packages that have already been recommended by the installed packages
    //@}

    // Additional QueueItems which has to be regarded by the solver
    // This will be used e.g. by solution actions
    solver::detail::SolverQueueItemList _removed_queue_items;
    solver::detail::SolverQueueItemList _added_queue_items;

    // Additional information about the solverrun
    ItemCapKindMap _isInstalledBy;
    ItemCapKindMap _installs;
    ItemCapKindMap _satifiedByInstalled;
    ItemCapKindMap _installedSatisfied;

    // helpers
    void collectResolverInfo();

    // Unmaintained packages which does not fit to the updated system
    // (broken dependencies) will be deleted.
    // returns true if solving was successful
    bool checkUnmaintainedItems ();

    void solverInit();

  public:

    Resolver( const ResPool & pool );
    virtual ~Resolver();

    // ---------------------------------- I/O

    std::ostream & dumpOn( std::ostream & str ) const;

    friend std::ostream& operator<<( std::ostream& str, const Resolver & obj )
    { return obj.dumpOn (str); }

    // ---------------------------------- methods

    ResPool pool() const;
    void setPool( const ResPool & pool ) { _pool = pool; }

    void addUpgradeRepo( Repository repo_r ) 		{ if ( repo_r && ! repo_r.isSystemRepo() ) _upgradeRepos.insert( repo_r ); }
    bool upgradingRepo( Repository repo_r ) const	{ return( _upgradeRepos.find( repo_r ) != _upgradeRepos.end() ); }
    void removeUpgradeRepo( Repository repo_r )		{ _upgradeRepos.erase( repo_r ); }
    void removeUpgradeRepos()				{ _upgradeRepos.clear(); }
    const std::set<Repository> & upgradeRepos() const	{ return _upgradeRepos; }

    void addExtraRequire( const Capability & capability );
    void removeExtraRequire( const Capability & capability );
    void addExtraConflict( const Capability & capability );
    void removeExtraConflict( const Capability & capability );

    void removeQueueItem( SolverQueueItem_Ptr item );
    void addQueueItem( SolverQueueItem_Ptr item );

    CapabilitySet extraRequires() const		{ return _extra_requires; }
    CapabilitySet extraConflicts() const	{ return _extra_conflicts; }

    void addWeak( const PoolItem & item );

    bool verifySystem();
    bool resolvePool();
    bool resolveQueue( SolverQueueItemList & queue );
    void doUpdate();

    bool doUpgrade();
    PoolItemList problematicUpdateItems() const;

    /** \name Solver flags */
    //@{
    bool ignoreAlreadyRecommended() const	{ return _ignoreAlreadyRecommended; }
    void setIgnoreAlreadyRecommended( bool yesno_r ) { _ignoreAlreadyRecommended = yesno_r; }

    bool onlyRequires () const			{ return _onlyRequires; }
    void setOnlyRequires( TriBool state_r );

    bool forceResolve()	const 			{ return _forceResolve; }
    void setForceResolve( TriBool state_r )	{ _forceResolve = indeterminate(state_r) ? false : bool(state_r); }

    bool isUpgradeMode() const 			{ return _upgradeMode; }// Resolver has been called with doUpgrade
    void setUpgradeMode( bool yesno_r )		{ _upgradeMode = yesno_r; }

    bool isUpdateMode() const 			{ return _updateMode; }	// Resolver has been called with doUpdate

    bool isVerifyingMode() const 		{ return _verifying; }	// The system will be checked
    void setVerifyingMode( TriBool state_r )	{ _verifying = indeterminate(state_r) ? false : bool(state_r); }

    bool allowVendorChange() const		{ return _allowVendorChange; }
    void setAllowVendorChange( TriBool state_r );

    bool solveSrcPackages() const 		{ return _solveSrcPackages; }
    void setSolveSrcPackages( TriBool state_r )	{ _solveSrcPackages = indeterminate(state_r) ? false : bool(state_r); }

    bool cleandepsOnRemove() const 		{ return _cleandepsOnRemove; }
    void setCleandepsOnRemove( TriBool state_r );
    //@}

#define ZOLV_FLAG_TRIBOOL( ZSETTER, ZGETTER )	\
    void ZSETTER( TriBool state_r );		\
    bool ZGETTER() const;			\

    ZOLV_FLAG_TRIBOOL( dupSetAllowDowngrade,	dupAllowDowngrade )
    ZOLV_FLAG_TRIBOOL( dupSetAllowNameChange,	dupAllowNameChange )
    ZOLV_FLAG_TRIBOOL( dupSetAllowArchChange,	dupAllowArchChange )
    ZOLV_FLAG_TRIBOOL( dupSetAllowVendorChange,	dupAllowVendorChange )

#undef ZOLV_FLAG_TRIBOOL

    ResolverProblemList problems() const;

    void applySolutions( const ProblemSolutionList & solutions );
    bool applySolution( const ProblemSolution & solution );

    // Return the Transaction computed by the last solver run.
    sat::Transaction getTransaction();

    // reset all SOLVER transaction in pool
    void undo();

    void reset( bool keepExtras = false );

    // Get more information about the solverrun
    // Which item will be installed by another item or triggers an item for
    // installation
    ItemCapKindList isInstalledBy( const PoolItem & item );
    ItemCapKindList installs( const PoolItem & item );
    ItemCapKindList satifiedByInstalled (const PoolItem & item );
    ItemCapKindList installedSatisfied( const PoolItem & item );

};

///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////
#endif // ZYPP_USE_RESOLVER_INTERNALS
#endif // ZYPP_SOLVER_DETAIL_RESOLVER_H
