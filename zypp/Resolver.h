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

    /*
     * Undo solver changes done in resolvePool()
     * Throwing away all ignored dependencies.
     */
    void undo( void );

    /**
     * Do an distribution upgrade
     *
     * This will run a full upgrade on the pool, taking all upgrade
     * dependencies (provide/obsolete for package renames, split-
     * provides, etc.) into account and actually removing installed
     * packages if no upgrade exists AND the package dependency is
     * broken
     *
     * To be run with great caution. It basically brings your
     * system 'back to start'.
     * Quite helpful to get back to a 'sane state'. Quite disastrous
     * since you'll loose all non-distribution packages
     **/
    void doUpgrade( UpgradeStatistics & opt_stats_r );

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
     * Return the list of problematic update items
     * i.e. locked ones (due to foreign vendor)
     **/
    std::list<PoolItem> problematicUpdateItems( void ) const;

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
     * Remove resolvables which are conflicts with others or
     * have unfulfilled requirements.
     * This behaviour is favourited by ZMD.
     **/
    void setForceResolve (const bool force);
    bool forceResolve();

    /**
     * Setting whether required packages are installed ONLY
     * So recommended packages, language packages and packages which depend 
     * on hardware (modalias) will not be regarded.
     **/
    void setOnlyRequires (const bool onlyRequires);
    void resetOnlyRequires(); // set back to default (described in zypp.conf)  
    bool onlyRequires();

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
     * Remove the additional requirement set by \ref addRequire(Capability).
     *
     */
    void removeRequire (const Capability & capability);

    /**
     * Remove the additional conflict set by \ref addConflict(Capability).
     *
     */
    void removeConflict (const Capability & capability);

    /**
     * Get all the additional requirements set by \ref addRequire(Capability).
     *
     */      
    const CapabilitySet getRequire ();
      
    /**
     * Get all the additional conflicts set by \ref addConflict(Capability).
     *
     */            
    const CapabilitySet getConflict();

    /**
     * Generates a solver Testcase of the current state
     *
     * \parame dumpPath destination directory of the created directory
     * \return true if it was successful     
     */
    bool createSolverTestcase (const std::string & dumpPath = "/var/log/YaST2/solverTestcase");


  protected:

  private:
    solver::detail::Resolver_Ptr _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESOLVER_H
