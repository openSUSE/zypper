/*
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

#include "zypp/Dep.h"
#include "zypp/Capability.h"

#include "zypp/solver/detail/Types.h"
#include "zypp/solver/detail/Resolver.h"
#include "zypp/solver/detail/SolverQueueItem.h"

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
	protected:
	    SolutionAction ();
	public:
	    virtual ~SolutionAction();

	    // ---------------------------------- I/O
	    virtual std::ostream & dumpOn( std::ostream & str ) const;
	    friend std::ostream& operator<<(std::ostream & str, const SolutionAction & action)
		{ return action.dumpOn (str); }
	    friend std::ostream& operator<<(std::ostream & str, const SolutionActionList & actionlist);
	    friend std::ostream& operator<<(std::ostream & str, const CSolutionActionList & actionlist);

	    // ---------------------------------- methods
	    /**
	     * Execute this action.
	     * Returns 'true' on success, 'false' on error.
	     **/
	    virtual bool execute (Resolver & resolver) const = 0;
	};


	/**
	 * A problem solution action that performs a transaction
	 * (installs, removes, keep ...)  one resolvable
	 * (package, patch, pattern, product).
	 **/
	typedef enum
	{
	    KEEP,
	    INSTALL,
	    REMOVE,
	    UNLOCK,
	    LOCK,
	    REMOVE_EXTRA_REQUIRE,
	    REMOVE_EXTRA_CONFLICT,
	    ADD_SOLVE_QUEUE_ITEM,
	    REMOVE_SOLVE_QUEUE_ITEM,
	} TransactionKind;


	class TransactionSolutionAction: public SolutionAction
	{
	public:
	    TransactionSolutionAction( PoolItem item,
				       TransactionKind action )
		: SolutionAction(),
		  _item( item ), _action( action ) {}

	    TransactionSolutionAction( Capability capability,
				       TransactionKind action )
		: SolutionAction(),
		  _capability( capability ), _action( action ) {}


	    TransactionSolutionAction( SolverQueueItem_Ptr item,
				       TransactionKind action )
		: SolutionAction(),
		  _solverQueueItem( item ), _action( action ) {}

	    TransactionSolutionAction( TransactionKind action )
		: SolutionAction(),
		  _item(), _action( action ) {}

	  // ---------------------------------- I/O
	  virtual std::ostream & dumpOn( std::ostream & str ) const;
	  friend std::ostream& operator<<(std::ostream& str, const TransactionSolutionAction & action)
		{ return action.dumpOn (str); }

	  // ---------------------------------- accessors

	  const PoolItem item() const { return _item; }
	  const Capability capability() const { return _capability; }
	  TransactionKind action() const { return _action; }

	  // ---------------------------------- methods
	    virtual bool execute(Resolver & resolver) const;

	protected:

	    PoolItem _item;
	    Capability _capability;
	    SolverQueueItem_Ptr _solverQueueItem;

	    const TransactionKind _action;
	};


	/**
	 * Type of ignoring; currently only WEAK
	 **/

	typedef enum
	{
	    WEAK
	} InjectSolutionKind;


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

	    InjectSolutionAction( PoolItem item,
				  const InjectSolutionKind & kind)
		: SolutionAction(),
		  _item( item ),
		  _kind( kind ) {}

	  // ---------------------------------- I/O
	  virtual std::ostream & dumpOn( std::ostream & str ) const;
	  friend std::ostream& operator<<(std::ostream& str, const InjectSolutionAction & action)
		{ return action.dumpOn (str); }

	  // ---------------------------------- accessors
	    const PoolItem item() const { return _item; }

	  // ---------------------------------- methods
	    virtual bool execute(Resolver & resolver) const;

	protected:
	    PoolItem _item;
	    const InjectSolutionKind _kind;
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

