/**
 *
 * Easy-to use interface to the ZYPP dependency resolver
 *
 * Author: Stefan Hundhammer <sh@suse.de>
 *
 **/

#ifndef ZYPP_SOLVER_DETAIL_RESOLVERPROBLEM_H
#define ZYPP_SOLVER_DETAIL_RESOLVERPROBLEM_H

#include <list>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/solver/detail/Types.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

	class ResolverProblem : public base::ReferenceCounted
	{
	private:

	    /**
	     * Clear all data.
	     * In particular, delete all members of _solutions.
	     **/
	    void clear();


	    //
	    // Data members
	    //

	    Resolver_constPtr	_resolver;
	    std::string		_description;
	    std::string		_details;
	    ProblemSolutionList	_solutions;

	public:

	    /**
	     * Constructor.
	     **/
	    ResolverProblem( const std::string & description, const std::string & details );

	    /**
	     * Destructor.
	     **/
	    ~ResolverProblem();

	  // ---------------------------------- I/O

	  static std::string toString (const ResolverProblem & problem);
	  static std::string toString (const ResolverProblemList & problemlist);

	  virtual std::ostream & dumpOn(std::ostream & str ) const;

	  friend std::ostream& operator<<(std::ostream&, const ResolverProblem & problem);

	  std::string asString (void ) const;

	  // ---------------------------------- accessors

	    /**
	     * Return a one-line description of the problem.
	     **/
	    std::string description() const { return _description; }

	    /**
	     * Return a (possibly muti-line) detailed description of the problem
	     * or an empty string if there are no useful details.
	     **/
	    std::string details() const { return _details; }

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
	    ProblemSolutionList solutions() const;

	    /**
	     * Return the parent dependency resolver.
	     **/
	    Resolver_constPtr resolver() const { return _resolver; }

	  // ---------------------------------- methods

	    /**
	     * Add a solution to this problem. This class takes over ownership of
	     * the problem and will delete it when neccessary.
	     **/
	    void addSolution( ProblemSolution_Ptr solution );

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

#endif // ZYPP_SOLVER_DETAIL_RESOLVERPROBLEM_H

