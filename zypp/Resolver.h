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
#include "zypp/UpgradeStatistics.h"
#include "zypp/solver/detail/Resolver.h"
#include "zypp/solver/detail/ResolverContext.h"
#include "zypp/ProblemTypes.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////



  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Resolver
  //
  /** Resolver interface.
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
     * Verify consistency of system
     *
     **/
    bool verifySystem ();

    /**
     * Resolve package dependencies:
     *
     * Verify consistency of system
     * considerNewHardware = install packages which depends on
     * new hardware
     *
     **/
    bool verifySystem (bool considerNewHardware);

    /**
     * Establish state of 'higher level' Resolvables in Pool
     *
     * Must be called when dealing with non-package resolvables,
     * like Patches, Patterns, and Products
     *
     * Must be called with a 'plain' pool, e.g. no additonal
     * transacts set.
     *
     * return true if it was successful
     * return false if not (this will only happen if other
     *   transactions are in the pool which will lead to
     *   no solution)
     **/
    bool establishPool (void);

    /**
     * go through all package 'freshen' dependencies and
     * schedule matches for installation.
     *
     * To be called at begin of installation and upgrade.
     * Probably also useful after adding a new package
     * repository.
     *
     * return true if it was successful
     * return false if not (this will only happen if other
     *   transactions are in the pool which will lead to
     *   no solution)
     **/
    bool freshenPool (void);

    /**
     * Resolve package dependencies:
     *
     * Try to execute all pending transactions (there may be more than
     * one!).
     * The solver pays attention to the BEST packages only in order to
     * come to a solution. 
     * If there has not been found a valid results all other branches
     * (e.G. packages with older version numbers, worse architecture)
     *  will be regarded.
     *
     * Returns "true" on success (i.e., if there were no problems that
     * need user interaction) and "false" if there were problems.  In
     * the latter case, use problems() and later applySolutions()
     * below.
     **/
    bool resolvePool (void);      

    /**
     * Resolve package dependencies:
     *
     * Try to execute all pending transactions (there may be more than
     * one!).
     * If tryAllPossibilities is false, restrict searches for matching
     *  requirements to best architecture, highest version.
     * If tryAllPossibilities is true, evaluate all possible matches
     *  for a requirement.
     *
     * Returns "true" on success (i.e., if there were no problems that
     * need user interaction) and "false" if there were problems.  In
     * the latter case, use problems() and later applySolutions()
     * below.
     **/
    bool resolvePool (bool tryAllPossibilities);
      
    bool resolveDependencies( void );

    /*
     * Undo solver changes done in resolvePool()
     * Throwing away all ignored dependencies.
     */
    void undo( void );

    /*
     * Get the most recent resolver context
     *
     * It will be NULL if resolvePool() or establishPool() was never called.
     * Depending on the return code of the last resolvePool() call,
     * it _either_ points to a valid or an invalid solution.
     */
    solver::detail::ResolverContext_Ptr context (void) const;

    /**
     * Do an distribution upgrade
     *
     * This will run a full upgrade on the pool, taking all upgrade
     * dependencies (provide/obsolete for package renames, split-
     * provides, etc.) into account and actually removing installed
     * packages if no upgrade exists.
     *
     * To be run with great caution. It basically brings your
     * system 'back to start'.
     * Quite helpful to get back to a 'sane state'. Quite disastrous
     * since you'll loose all non-distribution packages
     **/
    void doUpgrade( UpgradeStatistics & opt_stats_r );

    /**
     * Return the list of problematic update items
     * i.e. locked ones (due to foreign vendor)
     **/
    std::list<PoolItem_Ref> problematicUpdateItems( void ) const;

    /**
     * Return the dependency problems found by the last call to
     * resolveDependencies(). If there were no problems, the returned
     * list will be empty.
     **/
    ResolverProblemList problems();

    /**
     * Return more solver information if an error has happened.
     **/
      
    std::list<std::string> problemDescription( void ) const;      

    /**
     * Apply problem solutions. No more than one solution per problem
     * can be applied.
     **/
    void applySolutions( const ProblemSolutionList & solutions );

    Arch architecture() const;
    void setArchitecture( const Arch & arch);

    /**      
     * Remove resolvables which are conflicts with others or
     * have unfulfilled requirements.
     * This behaviour is favourited by ZMD.
     **/
    void setForceResolve (const bool force);
    const bool forceResolve();

    /**      
     * Prefer the result with the newest version if there are more solver
     * results. 
     **/
    void setPreferHighestVersion (const bool highestVersion);
    const bool preferHighestVersion();      

    /**
     * transact a single ResObject
     *
     * Installs (install == true) or removes (install == false) all required
     * and recommended packages(!) of \c robj
     * (More or less a 'single step' resolver call)
     *
     * returns false if requirements are not all fulfillable
     *
     */
    bool transactResObject( ResObject::constPtr robj, bool install = true);

    /**
     * transact all objects of this kind
     *
     * Look through the pool and runs transactResObject, first for removes
     * then for installs
     * (More or less a 'single step' resolver call)
     *
     * returns false if any transactResObject() call returned false.
     *
     */
    bool transactResKind( Resolvable::Kind kind );

    /**
     * reset any transact states
     *
     * Look through the pool and clear transact state.
     * It will only reset states which have an equal or
     * lower causer
     *
     */
    void transactReset( ResStatus::TransactByValue causer );

    /**
     * Adding additional requirement
     *
     */
    void addRequire (const Capability & capability);

    /**
     * Adding additional conflict
     *
     */
    void addConflict (const Capability & capability);      
      
    /**
     * Setting solver timeout
     *
     * Stop solving after a given timeframe (seconds) 
     * seconds = 0 : No timeout
     *
     */
    void setTimeout( int seconds );

    /**
     * Getting solver timeout in seconds
     *
     */
    int timeout();      

    /**
     * Restricting solver passes
     *
     * Stop solving after a given amount of passes
     * count = 0 : No restriction
     *
     */
    void setMaxSolverPasses (int count);

    /**
     * Count of max solver passes
     *
     */
    int maxSolverPasses ();

    /**
     * Generates a solver Testcase of the current state
     *
     * \parame dumpPath destination directory of the created directory
     * \return true if it was successful     
     */
    bool createSolverTestcase (const std::string & dumpPath = "/var/log/YaST2/solverTestcase");


    /**
     * Gives information about WHO has pused an installation of an given item.
     *
     * \param item    Evaluate additional information for this resolvable.
     * \return A list of structures which contains:
     *		item     Item which has triggered the installation of the given param item.
     *		cap      Capability which has triggerd this installation
     *		capKind  Kind of that capability (e.g.  Dep::REQUIRES,Dep::RECOMMENDS,... )
     *
     * Note: In order to have a result start a solver run before. Not matter if it is valid or invalid.
     *
     */
    const solver::detail::ItemCapKindList isInstalledBy (const PoolItem_Ref item);

    /**
     * Gives information about WHICH additional items will be installed due the installation of an item.
     *
     * \param item     Evaluate additional information for this resolvable.
     * \return A list of structures which contains:
     *		item     Item which will be installed due to the installation of the given param item too.
     *		cap      Capability which causes the installation
     *		capKind  Kind of that capability (e.g.  Dep::REQUIRES,Dep::RECOMMENDS,... )
     *
     * Note: In order to have a result start a solver run before. Not matter if it is valid or invalid.
     *
     */      
    const solver::detail::ItemCapKindList installs (const PoolItem_Ref item);
      

  protected:

  private:
    solver::detail::Resolver_Ptr _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESOLVER_H
