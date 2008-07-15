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

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/ProblemSolution.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////


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

	friend std::ostream& operator<<(std::ostream&, const ResolverProblem & problem);

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
	 * Set description of the problem.
	 **/
	void setDescription(const std::string & description)
	    { _description=description; }

	/**
	 * Set detail description of the problem.
	 **/
	void setDetails(const std::string & detail)
	    { _details=detail; }

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
	void addSolution( ProblemSolution_Ptr solution, bool inFront = false );

    };
    ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////

#endif // ZYPP_RESOLVERPROBLEM_H

