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
    bool verifySystem (void);

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
     * Resolve package dependencies:
     *
     * Try to execute all pending transactions (there may be more than
     * one!).
     *
     * Returns "true" on success (i.e., if there were no problems that
     * need user interaction) and "false" if there were problems.  In
     * the latter case, use problems() and later applySolutions()
     * below.
     **/
    bool resolvePool (void);

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
     * transact a single ResObject
     *
     * Installs (install == true) or removes (install == false) all required
     * and recommended packages(!) of \c robj
     * (More or less a 'single step' resolver call
     *
     * returns false if requirements are not all fulfillable
     *
     */
    bool transactResObject( ResObject::constPtr robj, bool install = true);

  protected:

  private:
    solver::detail::Resolver_Ptr _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESOLVER_H
