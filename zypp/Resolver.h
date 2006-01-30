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
#include "zypp/solver/detail/ResolverContext.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(Resolver);

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Resolver
  //
  /** Resolver interface.
  */
  class Resolver : public base::ReferenceCounted, private base::NonCopyable
  {
  public:

  protected:
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
    void verifySystem (void);

    /**
     * Establish state of 'higher level' Resolvables
     *
     * Must be called when dealing with non-package resolvables,
     * like Patches, Patterns, and Products
     *
     **/
    void establishState (void);

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

    /**
     * Return the dependency problems found by the last call to
     * resolveDependencies(). If there were no problems, the returned
     * list will be empty.
     **/
//    list<ResolverProblem_Ptr> problems();

    /**
     * Apply problem solutions. No more than one solution per problem
     * can be applied.
     **/
//    void applySolutions( list<ProblemSolution_Ptr> solutions );

    // ResolverContext_constPtr bestContext (void) const;

  protected:

  private:
    solver::detail::Resolver_Ptr _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESOLVER_H
