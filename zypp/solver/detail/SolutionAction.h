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
	    ALLBRANCHES
	} TransactionKind;


	class TransactionSolutionAction: public SolutionAction
	{
	public:
	    TransactionSolutionAction( PoolItem_Ref item,
				       TransactionKind action )
		: SolutionAction(),
		  _item( item ), _action( action ) {}

	    TransactionSolutionAction( TransactionKind action )
		: SolutionAction(),
		  _item(), _action( action ) {}

	  // ---------------------------------- I/O
	  virtual std::ostream & dumpOn( std::ostream & str ) const;
	  friend std::ostream& operator<<(std::ostream& str, const TransactionSolutionAction & action)
		{ return action.dumpOn (str); }

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
	 * Type of ignoring dependencies, architectures and vendor
	 **/

	typedef enum
	{
	    REQUIRES,
	    CONFLICTS,
	    OBSOLETES,
	    INSTALLED,
	    ARCHITECTURE,
	    VENDOR
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

	    InjectSolutionAction( PoolItem_Ref item,
				  const Capability & capability,
				  const InjectSolutionKind & kind)
		: SolutionAction(),
		  _item( item ), _capability( capability ),
		  _kind( kind ), _otherItem() {}

	    InjectSolutionAction( PoolItem_Ref item,
				  const InjectSolutionKind & kind)
		: SolutionAction(),
		  _item( item ), _capability(),
		  _kind( kind ), _otherItem() {}

	    InjectSolutionAction( PoolItem_Ref item,
				  const Capability & capability,
				  const InjectSolutionKind & kind,
				  PoolItem_Ref otherItem)
		: SolutionAction(),
		  _item( item ), _capability( capability ),
		  _kind( kind ), _otherItem( otherItem ) {}

	  // ---------------------------------- I/O
	  virtual std::ostream & dumpOn( std::ostream & str ) const;
	  friend std::ostream& operator<<(std::ostream& str, const InjectSolutionAction & action)
		{ return action.dumpOn (str); }

	  // ---------------------------------- accessors
	    const Capability & capability() const { return _capability; };
	    const PoolItem_Ref item() const { return _item; }

	  // ---------------------------------- methods
	    virtual bool execute(Resolver & resolver) const;

	protected:
	    PoolItem_Ref _item;
	    const Capability _capability;
	    const InjectSolutionKind _kind;
	    PoolItem_Ref _otherItem;
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

