/*
 *
 * Easy-to use interface to the ZYPP dependency resolver
 *
 * Author: Stefan Hundhammer <sh@suse.de>
 *
 **/

#ifndef ZYPP_RESOLVERPROBLEM_H
#define ZYPP_RESOLVERPROBLEM_H

#include <list>
#include <string>

#include "zypp/ProblemTypes.h"
#include "zypp/ProblemSolution.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////////
  /// \class ResolverProblem
  /// \brief Describe a solver problem and offer solutions.
  ///////////////////////////////////////////////////////////////////////
  class ResolverProblem : public base::ReferenceCounted
  {
  public:
    /** Constructor. */
    ResolverProblem();
    /** Constructor. */
    ResolverProblem( std::string description );
    /** Constructor. */
    ResolverProblem( std::string description, std::string details );

    /** Destructor. */
    ~ResolverProblem();


    /**
     * Return a one-line description of the problem.
     **/
    const std::string & description() const;

    /**
     * Return a (possibly muti-line) detailed description of the problem
     * or an empty string if there are no useful details.
     **/
    const std::string & details() const;

    /**
     * Return the possible solutions to this problem.
     * All problems should have at least 2-3 (mutually exclusive) solutions:
     *
     *	  -  Undo: Do not perform the offending transaction
     *	 (do not install the package that had unsatisfied requirements,
     *	  do not remove	 the package that would break other packages' requirements)
     *
     *	  - Remove referrers: Remove all packages that would break because
     *	they depend on the package that is requested to be removed
     *
     *	  - Ignore: Inject artificial "provides" for a missing requirement
     *	(pretend that requirement is satisfied)
     **/
    const ProblemSolutionList & solutions() const;


    /**
     * Set description of the problem.
     **/
    void setDescription( std::string description );

    /**
     * Set detail description of the problem.
     **/
    void setDetails( std::string details );

    /**
     * Add a solution to this problem. This class takes over ownership of
     * the problem and will delete it when neccessary.
     **/
    void addSolution( ProblemSolution_Ptr solution, bool inFront = false );

  private:
    class Impl;
    RWCOW_pointer<Impl> _pimpl;
  };

  /** \relates ResolverProblem Stream output */
  std::ostream & operator<<( std::ostream &, const ResolverProblem & obj );

  /** \relates ResolverProblem Stream output */
  std::ostream & operator<<( std::ostream &, const ResolverProblemList & obj );


} // namespace zypp
/////////////////////////////////////////////////////////////////////////
#endif // ZYPP_RESOLVERPROBLEM_H

