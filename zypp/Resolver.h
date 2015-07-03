/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Resolver.h
 *
*/
#ifndef ZYPP_RESOLVER_H
#define ZYPP_RESOLVER_H

#include <iosfwd>
#include <functional>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/ResPool.h"
#include "zypp/solver/detail/Resolver.h"
#include "zypp/solver/detail/SolverQueueItem.h"
#include "zypp/ProblemTypes.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  namespace sat
  {
    class Transaction;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Resolver
  //
  /**
   * Dependency resolver interface.
   *
   * To resolve dependencies after making changes to the \ref ResPool (using
   * \ref addRequire(), \ref addConflict(), \ref applySolutions(), or by making
   * the changes directly on the \ref PoolItem status objects,
   * call the \ref resolvePool() method.
   */
  class Resolver : public base::ReferenceCounted, private base::NonCopyable
  {
  public:

    /** Ctor */
    Resolver( const ResPool & pool );
    /** Dtor */
    virtual ~Resolver();

    /**
     * Resolve package dependencies:
     *
     * Enter \ref systemVerification mode to monitor and repair dependencies
     * of already installed packages, and solve immediately.
     *
     * Call \ref setSystemVerification to turn of this mode.
     **/
    bool verifySystem();


    /**
     * Resolve package dependencies:
     *
     * Try to execute all pending transactions (there may be more than
     * one!).
     * The solver collects all transactions (install/delete resolvables)
     * from the pool, generates task, solving it and writes the
     * results back to pool
     *
     * Returns "true" on success (i.e., if there were no problems that
     * need user interaction) and "false" if there were problems.  In
     * the latter case, use problems() and later applySolutions()
     * below.
     **/
    bool resolvePool();


    /**
     * Resolve package dependencies:
     *
     * The solver works off the given queue and writes back the solution
     * to pool.
     *
     * Returns "true" on success (i.e., if there were no problems that
     * need user interaction) and "false" if there were problems.  In
     * the latter case, use problems() and later applySolutions()
     * below.
     * The solution could be that the solver remove/add some entries
     * in the task queue. So make a new call of resolveQueue after you
     * have applied any solution AND check the parameter "queue" if
     * there has been any changes by the solver and adapt these changes
     * to e.g. the selectables.
     *
     **/
    bool resolveQueue( solver::detail::SolverQueueItemList & queue );

    /*
     * Undo solver changes done in resolvePool()
     * Throwing away all ignored dependencies.
     */
    void undo();

    /*
     * Resets solver information and verify option.
     */
    void reset();


    /**
     * Do an distribution upgrade (DUP)
     *
     * Perform a distribution upgrade. This performs an update of
     * all packages with a special resolver algorithm which takes
     * care of package splits, pattern  and  product  updates,
     * etc.
     * This call also turns the solver into \ref upgradeMode, so
     * consecutive calls to \ref resolvePool performed in this
     * mode too. Call \ref setUpgradeMode to turn this mode off.
     *
     * \see \ref addUpgradeRepo
     **/
    bool doUpgrade();

    /**
     * Update to newest package
     *
     * Install the newest version of your installed packages as
     * far as possible. This means a newer package will NOT be
     * installed if it generates dependency problems.
     * So the user will not get an error message.
     *
     **/
    void doUpdate( );

    /**
     * Unmaintained packages which does not fit to
     * the updated system (broken dependencies) will be
     * deleted.
     * Return the list of deleted items.
     * Note : This list is valid after the call doUpgrade() only.
     **/
    std::list<PoolItem> problematicUpdateItems() const;

    /**
     * Return the dependency problems found by the last call to
     * resolveDependencies(). If there were no problems, the returned
     * list will be empty.
     **/
    ResolverProblemList problems();


    /**
     * Apply problem solutions. No more than one solution per problem
     * can be applied.
     **/
    void applySolutions( const ProblemSolutionList & solutions );

    /**
     * Return the \ref Transaction computed by the last solver run.
     */
    sat::Transaction getTransaction();

    /**
     * Remove resolvables which are conflicts with others or
     * have unfulfilled requirements.
     * This behaviour is favourited by ZMD.
     **/
    void setForceResolve( bool force );
    bool forceResolve() const;

    /**
     * Ignore recommended packages that were already recommended by
     * the installed packages
     **/
    void setIgnoreAlreadyRecommended( bool yesno_r );
    bool ignoreAlreadyRecommended() const;

    /**
     * Setting whether required packages are installed ONLY
     * So recommended packages, language packages and packages which depend
     * on hardware (modalias) will not be regarded.
     **/
    void setOnlyRequires( bool yesno_r );
    void resetOnlyRequires(); // set back to default (described in zypp.conf)
    bool onlyRequires() const;

    /**
     * Setting whether the solver should perform in 'upgrade' mode or
     * not.
     * \see \ref doUpgrade.
     */
    void setUpgradeMode( bool yesno_r );
    bool upgradeMode() const;

    /**
     * Setting whether the solver should allow or disallow vendor changes.
     *
     * If OFF (the default) the solver will replace packages with packages
     * of the same (or equivalent) vendor ony.
     *
     * \see \ref VendorAttr for definition of vendor equivalence.
     **/
    void setAllowVendorChange( bool yesno_r );
    void setDefaultAllowVendorChange(); // set back to default (in zypp.conf)
    bool allowVendorChange() const;

    /**
     * System verification mode also monitors and repairs dependencies
     * of already installed packages.
     * \see \ref verifySystem
     */
    void setSystemVerification( bool yesno_r );
    void setDefaultSystemVerification();
    bool systemVerification() const;

    /**
     * Set whether to solve source packages build dependencies per default.
     * Usually turned off and if, enabled per source package.
     * \NOTE This affects only source packges selected in the \ref ResPool. No solver rule
     * will be generated for them. Source packages requested via e.g. \ref addRequire will
     * always be solved.
     * \NOTE Be carefull. The older the source package is, the stranger may be the
     * result of solving it's build dependencies.
     */
    void setSolveSrcPackages( bool yesno_r );
    void setDefaultSolveSrcPackages();
    bool solveSrcPackages() const;

    /**
     * Cleanup when deleting packages. Whether the solver should per default
     * try to remove packages exclusively required by the ones he's asked to delete.
     */
    void setCleandepsOnRemove( bool yesno_r );
    void setDefaultCleandepsOnRemove(); // set back to default (in zypp.conf)
    bool cleandepsOnRemove() const;

    /** \name  Solver flags for DUP mode.
     * DUP mode default settings differ from 'ordinary' ones. Default for
     * all DUP flags is \c true.
     */
    //@{
    /** dup mode: allow to downgrade installed solvable */
    void dupSetAllowDowngrade( bool yesno_r );
    void dupSetDefaultAllowDowngrade();	// Set back to default
    bool dupAllowDowngrade() const;

    /** dup mode: allow to change name of installed solvable */
    void dupSetAllowNameChange( bool yesno_r );
    void dupSetDefaultAllowNameChange();	// Set back to default
    bool dupAllowNameChange() const;

    /** dup mode: allow to change architecture of installed solvables */
    void dupSetAllowArchChange( bool yesno_r );
    void dupSetDefaultAllowArchChange();	// Set back to default
    bool dupAllowArchChange() const;

    /**  dup mode: allow to change vendor of installed solvables*/
    void dupSetAllowVendorChange( bool yesno_r );
    void dupSetDefaultAllowVendorChange();	// Set back to default
    bool dupAllowVendorChange() const;
    //@}

    /** \name Upgrade to content of a specific repository.
     * \note This is an ordinary solver request. You should simply
     * \ref resolvePool to execute, and not \ref doUpgrade.
     */
    //@{
    /**
     * Adding request to perform a dist upgrade restricted to this repository.
     *
     * This is what e.g. <tt>zypper dup --repo myrepo</tt> should perform.
     * \see \ref doUpgrade
     */
    void addUpgradeRepo( Repository repo_r );

    /**
     * Whether there is an \c UpgradeRepo request pending for this repo.
     */
    bool upgradingRepo( Repository repo_r ) const;

    /**
     * Remove an upgrade request for this repo.
     */
    void removeUpgradeRepo( Repository repo_r );

    /**
     * Remove all upgrade repo requests.
     */
    void removeUpgradeRepos();
    //@}

    /**
     * Adding additional requirement
     *
     */
    void addRequire( const Capability & capability );

    /**
     * Adding additional conflict
     *
     */
    void addConflict( const Capability & capability );

    /**
     * Remove the additional requirement set by \ref addRequire(Capability).
     *
     */
    void removeRequire( const Capability & capability );

    /**
     * Remove the additional conflict set by \ref addConflict(Capability).
     *
     */
    void removeConflict( const Capability & capability );

    /**
     * Get all the additional requirements set by \ref addRequire(Capability).
     *
     */
    CapabilitySet getRequire() const;

    /**
     * Get all the additional conflicts set by \ref addConflict(Capability).
     *
     */
    CapabilitySet getConflict() const;

    /**
     * Generates a solver Testcase of the current state
     *
     * \parame dumpPath destination directory of the created directory
     * \return true if it was successful
     */
    bool createSolverTestcase( const std::string & dumpPath = "/var/log/YaST2/solverTestcase", bool runSolver = true );

    /**
     * Gives information about WHO has pused an installation of an given item.
     *
     * \param item    Evaluate additional information for this resolvable.
     * \return A list of structures which contains:
     *		item                Item which has triggered the installation of the given param item.
     *          initialInstallation This item has triggered the installation
     *	                            Not already fullfilled requierement only.
     *		cap                 Capability which has triggerd this installation
     *		capKind             Kind of that capability (e.g.  Dep::REQUIRES,Dep::RECOMMENDS,... )
     *
     * Note: In order to have a result start a solver run before. Not matter if it is valid or invalid.
     *
     */
    solver::detail::ItemCapKindList isInstalledBy( const PoolItem & item );

    /**
     * Gives information about WHICH additional items will be installed due the installation of an item.
     *
     * \param item     Evaluate additional information for this resolvable.
     * \return A list of structures which contains:
     *		item                Item which has triggered the installation of the given param item.
     *          initialInstallation This item has triggered the installation
     *	                            Not already fullfilled requierement only.
     *		cap                 Capability which has triggerd this installation
     *		capKind             Kind of that capability (e.g.  Dep::REQUIRES,Dep::RECOMMENDS,... )
     *
     * Note: In order to have a result start a solver run before. Not matter if it is valid or invalid.
     *
     */
    solver::detail::ItemCapKindList installs( const PoolItem & item );

    /**
     * Gives information about WHICH installed items are requested by the installation of an item.
     *
     * \param item     Evaluate additional information for this resolvable.
     * \return A list of structures which contains:
     *		item                Item which has triggered the installation of the given param item.
     *          initialInstallation This item has triggered the installation
     *	                            Not already fullfilled requierement only.
     *		cap                 Capability which has triggerd this installation
     *		capKind             Kind of that capability (e.g.  Dep::REQUIRES,Dep::RECOMMENDS,... )
     *
     * Note: In order to have a result start a solver run before. Not matter if it is valid or invalid.
     *
     */
    solver::detail::ItemCapKindList satifiedByInstalled( const PoolItem & item );


    /**
     * Gives information about WHICH items require an already installed item.
     *
     * \param item     Evaluate additional information for this resolvable.
     * \return A list of structures which contains:
     *		item                Item which has triggered the installation of the given param item.
     *          initialInstallation This item has triggered the installation
     *	                            Not already fullfilled requierement only.
     *		cap                 Capability which has triggerd this installation
     *		capKind             Kind of that capability (e.g.  Dep::REQUIRES,Dep::RECOMMENDS,... )
     *
     * Note: In order to have a result start a solver run before. Not matter if it is valid or invalid.
     *
     */
    solver::detail::ItemCapKindList installedSatisfied( const PoolItem & item );


  private:
    friend std::ostream & operator<<( std::ostream & str, const Resolver & obj );

    typedef solver::detail::Resolver Impl;
    zypp::RW_pointer<Impl,rw_pointer::Intrusive<Impl> > _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Resolver Stream output */
  std::ostream & operator<<( std::ostream & str, const Resolver & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESOLVER_H
