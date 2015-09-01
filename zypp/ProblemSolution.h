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

#include "zypp/ProblemTypes.h"
#include "zypp/ResolverProblem.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{
  /////////////////////////////////////////////////////////////////////////
  /// \class ProblemSolution
  /// \brief Class representing one possible solution to a problem found during resolving
  ///
  /// All problems should have at least 2-3 (mutually exclusive) solutions:
  ///
  ///    -	 Undo: Do not perform the offending transaction
  ///	 (do not install the package that had unsatisfied requirements,
  ///	  do not remove	 the package that would break other packages' requirements)
  ///
  ///    - Remove referrers: Remove all packages that would break because
  ///	they depend on the package that is requested to be removed
  ///
  ///    - Ignore: Inject artificial "provides" for a missing requirement
  ///	(pretend that requirement is satisfied)
  /////////////////////////////////////////////////////////////////////////
  class ProblemSolution : public base::ReferenceCounted
  {
  public:
    typedef solver::detail::SolutionAction_Ptr SolutionAction_Ptr;
    typedef solver::detail::SolutionActionList SolutionActionList;

    /** Constructor. */
    ProblemSolution();

    /** Constructor. */
    ProblemSolution( std::string description );

    /** Constructor. */
    ProblemSolution( std::string description, std::string details );

   /** Destructor. */
    virtual ~ProblemSolution();


    /**
     * Return a one-line text description of this solution.
     **/
    const std::string & description() const;

    /**
     * Return a (possibly multi-line) detailed description of this
     * solution or an empty string if there are no useful details.
     **/
    const std::string & details() const;

    /**
     * Return the list of actions forming this solution.
     **/
    const SolutionActionList & actions() const;

    /**
     * Set description of the solution.
     **/
    void setDescription( std::string description );

    /**
     * Set detail description of the solution.
     **/
    void setDetails( std::string details );

    /**
     * Collect multiple action descriptions in \ref details (NL separated)
     **/
    void pushDescriptionDetail( std::string description, bool front = false );


    /**
     * Add an action to the actions list.
     **/
    void addAction( SolutionAction_Ptr action );


  private:
    class Impl;
    RWCOW_pointer<Impl> _pimpl;
  };

  /** \relates ProblemSolution Stream output */
  std::ostream& operator<<(std::ostream&, const ProblemSolution & obj );

  /** \relates ProblemSolution Stream output */
  std::ostream& operator<<(std::ostream&, const ProblemSolutionList & obj );

} // namespace zypp
/////////////////////////////////////////////////////////////////////////
#endif // ZYPP_PROBLEMSOLUTION_H

