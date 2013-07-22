/**
 *
 * Easy-to use interface to the ZYPP dependency resolver
 *
 * Author: Stefan Hundhammer <sh@suse.de>
 *
 **/

#ifndef ZYPP_PROBLEMSOLUTION_H
#define ZYPP_PROBLEMSOLUTION_H

#include <list>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/solver/detail/Resolver.h"
#include "zypp/ResolverProblem.h"
#include "zypp/solver/detail/SolutionAction.h"
#include "zypp/solver/detail/Types.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////


    /**
     * Class representing one possible solution to one problem found during resolving
     *
     * All problems should have at least 2-3 (mutually exclusive) solutions:
     *
     *    -	 Undo: Do not perform the offending transaction
     *	 (do not install the package that had unsatisfied requirements,
     *	  do not remove	 the package that would break other packages' requirements)
     *
     *    - Remove referrers: Remove all packages that would break because
     *	they depend on the package that is requested to be removed
     *
     *    - Ignore: Inject artificial "provides" for a missing requirement
     *	(pretend that requirement is satisfied)
     **/
    class ProblemSolution : public base::ReferenceCounted
    {
    protected:

	/**
	 * Clear all data.
	 * In particular, delete all members of _actions.
	 **/
	void clear();

	//
	// Data members
	//
	ResolverProblem_Ptr	_problem;
	solver::detail::CSolutionActionList	_actions;
	std::string		_description;
	std::string		_details;

    public:

	/**
	 * Constructor.
	 **/
	ProblemSolution( ResolverProblem_Ptr parent, const  std::string & description, const std::string & details );

	/**
	 * Destructor.
	 **/
	~ProblemSolution();

	// ---------------------------------- I/O

	friend std::ostream& operator<<(std::ostream&, const ProblemSolution & solution);
	friend std::ostream& operator<<(std::ostream&, const ProblemSolutionList & solutionlist);
	friend std::ostream& operator<<(std::ostream&, const CProblemSolutionList & solutionlist);

	// ---------------------------------- accessors
	/**
	 * Return a one-line text description of this solution.
	 **/
	std::string description() const { return _description; }

	/**
	 * Return a (possibly multi-line) detailed description of this
	 * solution or an empty string if there are no useful details.
	 **/
	std::string details() const { return _details; }

	/**
	 * Return the parent dependency problem.
	 **/
	ResolverProblem_Ptr problem() const { return _problem; }

	// ---------------------------------- methods

	/**
	 * Apply this solution, i.e. execute all of its actions.
	 *
	 * Returns 'true' on success, 'false' if actions could not be performed.
	 **/
	bool apply (solver::detail::Resolver & resolver);

	/**
	 * Add an action to the actions list.
	 **/
	void addAction( solver::detail::SolutionAction_constPtr action );

	solver::detail::CSolutionActionList actions() {return _actions;}

    };


    ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////

#endif // ZYPP_PROBLEMSOLUTION_H

