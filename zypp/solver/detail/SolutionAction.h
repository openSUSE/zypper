/**
 *
 * Easy-to use interface to the ZYPP dependency resolver
 *
 * Author: Stefan Hundhammer <sh@suse.de>
 *
 **/

#ifndef ZYPP_SOLVER_DETAIL_SOLUTIONACTION_H
#define ZYPP_SOLVER_DETAIL_SOLUTIONACTION_H

#include <list>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/Capability.h"
#include "zypp/solver/detail/Resolver.h"
#include "zypp/solver/detail/ProblemSolutionPtr.h"
#include "zypp/solver/detail/ResolverProblemPtr.h"
#include "zypp/solver/detail/SolutionActionPtr.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

	/**
	 * Abstract base class for one action of a problem solution.
	 **/
	class SolutionAction : public base::ReferenceCounted
	{
	public:
	  SolutionAction();
	  virtual ~SolutionAction();

	  // ---------------------------------- I/O

	  static std::string toString (const SolutionAction & action);
	  static std::string toString (const SolutionActionList & actionlist);
	  static std::string toString (const CSolutionActionList & actionlist);

	  virtual std::ostream & dumpOn(std::ostream & str ) const = 0;

	  friend std::ostream& operator<<(std::ostream&, const SolutionAction & action);

	  virtual std::string asString (void) const = 0;

	  // ---------------------------------- methods
	    /**
	     * Execute this action.
	     * Returns 'true' on success, 'false' on error.
	     **/
	    virtual bool execute() = 0;
	};


	/**
	 * A problem solution action that performs a transaction
	 * (installs, updates, removes, ...)  one resolvable
	 * (package, patch, pattern, product).
	 **/
	class TransactionSolutionAction: public SolutionAction
	{
	public:

	    typedef enum
	    {
		// TO DO: use some already existing enum (?)
		Keep,
		Install,
		Update,
		Remove
	    } TransactionKind;


	    TransactionSolutionAction( ResItem_constPtr resolvable,
				       TransactionKind action )
		: SolutionAction(), _resolvable( resolvable ), _action( action ) {}

	  // ---------------------------------- I/O

	  static std::string toString (const TransactionSolutionAction & action);

	  virtual std::ostream & dumpOn(std::ostream & str ) const;

	  friend std::ostream& operator<<(std::ostream&, const TransactionSolutionAction & action);

	  std::string asString (void) const;

	  // ---------------------------------- accessors
	    ResItem_constPtr resolvable() const { return _resolvable; }
	    TransactionKind action()     const { return _action;     }

	  // ---------------------------------- methods
	    virtual bool execute();


	protected:

	    ResItem_constPtr	_resolvable;
	    TransactionKind	_action;
	};


	/**
	 * A problem solution action that injects an artificial "provides" to
	 * the pool to satisfy open requirements or remove the conflict of
	 * concerning resolvable
	 *
	 * This is typically used by "ignore" (user override) solutions.
	 **/
	class InjectSolutionAction: public SolutionAction
	{
	public:
	    
	    typedef enum
	    {
		PROVIDE,
		CONFLICT
	    } CapabilityKind;
	    
	    InjectSolutionAction( const Capability & capability, const CapabilityKind & kind )
		: SolutionAction(), _capability( capability ), _kind( kind ) {}

	  // ---------------------------------- I/O

	  static std::string toString (const InjectSolutionAction & action);

	  virtual std::ostream & dumpOn(std::ostream & str ) const;

	  friend std::ostream& operator<<(std::ostream&, const InjectSolutionAction & action);

	  std::string asString (void) const;

	  // ---------------------------------- accessors
	    const Capability & capability() const { return _capability; };

	  // ---------------------------------- methods
	    virtual bool execute();

	protected:

	    const Capability & _capability;
	    const CapabilityKind & _kind;
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

#endif // ZYPP_SOLVER_DETAIL_SOLUTIONACTION_H

