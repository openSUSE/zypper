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

#include "zypp/Dep.h"
#include "zypp/Capability.h"

#include "zypp/solver/detail/Types.h"
#include "zypp/solver/detail/Resolver.h"

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

	    friend std::ostream& operator<<(std::ostream&, const SolutionAction & action);
	    friend std::ostream& operator<<(std::ostream&, const SolutionActionList & actionlist);
	    friend std::ostream& operator<<(std::ostream&, const CSolutionActionList & actionlist);

	    // ---------------------------------- methods
	    /**
	     * Execute this action.
	     * Returns 'true' on success, 'false' on error.
	     **/
	    virtual bool execute (Resolver & resolver) const = 0;
	};


	/**
	 * A problem solution action that performs a transaction
	 * (installs, updates, removes, ...)  one resolvable
	 * (package, patch, pattern, product).
	 **/
	typedef enum
	{
	    KEEP,
	    INSTALL,
	    UPDATE,
	    REMOVE,
	    UNLOCK
	} TransactionKind;
	
	class TransactionSolutionAction: public SolutionAction
	{
	public:
	    TransactionSolutionAction( PoolItem_Ref item,
				       TransactionKind action )
		: SolutionAction(),
		  _item( item ), _action( action ) {}

	  // ---------------------------------- I/O

	  friend std::ostream& operator<<(std::ostream&, const TransactionSolutionAction & action);

	  // ---------------------------------- accessors

	  const PoolItem_Ref item() const { return _item; }
	  const TransactionKind action() const { return _action; }

	  // ---------------------------------- methods
	    virtual bool execute(Resolver & resolver) const;

	protected:

	    PoolItem_Ref _item;
	    const TransactionKind _action;
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
	    
	    InjectSolutionAction( PoolItem_Ref item, const Capability & capability, const Dep & kind )
		: SolutionAction(),
		  _capability( capability ), _kind( kind ) {}

	  // ---------------------------------- I/O

	  friend std::ostream& operator<<(std::ostream&, const InjectSolutionAction & action);

	  // ---------------------------------- accessors
	    const Capability & capability() const { return _capability; };
	    const PoolItem_Ref item() const { return _item; }	    

	  // ---------------------------------- methods
	    virtual bool execute(Resolver & resolver) const;

	protected:

	    const Capability & _capability;
	    PoolItem_Ref _item;	    
	    const Dep & _kind;
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

