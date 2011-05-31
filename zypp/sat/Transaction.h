/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/Transaction.h
 */
extern "C"
{
  struct _Transaction;
}
#ifndef ZYPP_SAT_TRANSACTION_H
#define ZYPP_SAT_TRANSACTION_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/SafeBool.h"
#include "zypp/base/Iterator.h"
#include "zypp/base/DefaultIntegral.h"

#include "zypp/sat/detail/PoolImpl.h"
#include "zypp/sat/SolvIterMixin.h"
#include "zypp/sat/Solvable.h"

#include "zypp/PoolItem.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    namespace detail
    {
      /** Needs to be outside \ref Transaction in order to be usable in SolvIterMixin. */
      class Transaction_iterator;
      /** Needs to be outside \ref Transaction in order to be usable in SolvIterMixin. */
      class Transaction_const_iterator;
    }

    /** Satsolver transaction wrapper.
     * \note Note that Transaction is derived from \ref sat::SolvIterMixin which
     *       makes PoolItem and Selectable iterators automatically available.
     * \note Changing the \ref ResPool content (loading/unloading repositories)
     *       invalidates all outstanding transaction data. \see \ref valid.
     */
    class Transaction : public SolvIterMixin<Transaction, detail::Transaction_const_iterator>
		      , protected base::SafeBool<Transaction>
    {
      friend std::ostream & operator<<( std::ostream & str, const Transaction & obj );
      friend std::ostream & dumpOn( std::ostream & str, const Transaction & obj );
      friend bool operator==( const Transaction & lhs, const Transaction & rhs );

     public:
       /** Represents a single step within a \ref Transaction. */
       class Step;

       /** Type of (rpm) action to perform in a \ref Step. */
       enum StepType
       {
	 TRANSACTION_IGNORE,		/**< [ ] Nothing (includes implicit deletes due to obsoletes and non-package actions) */
	 TRANSACTION_ERASE,		/**< [-] Delete item */
	 TRANSACTION_INSTALL,		/**< [+] Install(update) item */
	 TRANSACTION_MULTIINSTALL	/**< [M] Install(multiversion) item (\see \ref ZConfig::multiversion) */
       };

       /** \ref Step action result. */
       enum StepStage
       {
	 STEP_TODO,		/**< [__] unprocessed */
	 STEP_DONE,		/**< [OK] success */
	 STEP_ERROR,		/**< [**] error */
       };

     public:
        /** Default ctor: empty transaction. */
        Transaction();

        /** Ctor cloning a sat transaction. */
        Transaction( ::_Transaction & trans_r );

        /** Dtor */
        ~Transaction();

      public:
	/** Whether transaction actually contains data and also fits the current pools content. */
	bool valid() const;

        /**  Validate object in a boolean context: valid */
        using base::SafeBool<Transaction>::operator bool_type;

	/** Order transaction steps for commit.
	 * It's cheap to call it for an aleready ordered \ref Transaction.
	 * This invalidates outstanding iterators. Returns whether
	 * \ref Transaction is \ref valid.
	 */
	bool order();

	/** Whether the transaction contains any steps. */
	bool empty() const;

	/** Number of steps in transaction steps. */
	size_t size() const;

       typedef detail::Transaction_iterator iterator;
       typedef detail::Transaction_const_iterator const_iterator;

	/** Iterator to the first \ref TransactionStep */
	const_iterator begin() const;
	/** \overload */
	iterator begin();

	/** Iterator behind the last \ref TransactionStep */
	const_iterator end() const;
	/** \overload */
	iterator end();

	/** Return iterator pointing to \a solv_r or \ref end. */
	const_iterator find( const sat::Solvable & solv_r ) const;
	iterator find( const sat::Solvable & solv_r );
	/** \overload */
	const_iterator find( const ResObject::constPtr & resolvable_r ) const;
	iterator find( const ResObject::constPtr & resolvable_r );
	/** \overload */
	const_iterator find( const PoolItem & pi_r ) const;
	iterator find( const PoolItem & pi_r );

      public:
	/** \name Omit iterating TRANSACTION_IGNORE steps.
	 */
	//@{
	struct FilterAction;
	typedef filter_iterator<FilterAction,const_iterator> action_iterator;
	action_iterator actionBegin() const;
	action_iterator actionEnd() const;
	//@}

      private:
        friend base::SafeBool<Transaction>::operator bool_type() const;
        /**  Validate object in a boolean context. */
        bool boolTest() const
        { return valid(); }
      public:
        /** Implementation  */
        class Impl;
      private:
        /** Pointer to implementation */
        RW_pointer<Impl> _pimpl;
    };

    /** \relates Transaction Stream output */
    std::ostream & operator<<( std::ostream & str, const Transaction & obj );

    /** \relates Transaction Verbose stream output */
    std::ostream & dumpOn( std::ostream & str, const Transaction & obj );

    /** \relates Transaction */
    bool operator==( const Transaction & lhs, const Transaction & rhs );

    /** \relates Transaction */
    inline bool operator!=( const Transaction & lhs, const Transaction & rhs )
    { return !( lhs == rhs ); }


    /** A single step within a \ref Transaction.
     *
     * \note After commit, when the @System repo (rpm database) is reread, all
     * @System solvables within the transaction are invalidated (they got deleted).
     * Thats why we internally store the NVRA, so you can access \ref ident
     * (\see \ref sat::Solvable::ident), \ref edition, \ref arch of a deleted package,
     * even if the \ref satSolvable itself is meanwhile invalid.
     *
     * \see \ref Transaction.
     */
    class Transaction::Step
    {
      friend std::ostream & operator<<( std::ostream & str, const Step & obj );

      public:
	Step();
	Step( const RW_pointer<Impl> & pimpl_r, detail::IdType id_r )
	  : _solv( id_r )
	  , _pimpl( pimpl_r )
	{}

      public:
	/** Type of action to perform in this step. */
	StepType stepType() const;

	/** Step action result. */
	StepStage stepStage() const;

	/** Set step action result. */
	void stepStage( StepStage val_r );

	/** Return the corresponding \ref Solvable (or.
	 *
	 */
	Solvable satSolvable() const
	{ return _solv; }

	/** \name Post mortem acccess to @System solvables
	 * \code
	 *   Transaction::Step step;
	 *   if ( step.satSolvable() )
	 *     std::cout << step.satSolvable() << endl;
	 *   else
	 *     std::cout << step.ident() << endl; // deleted @System solvable
	 * \endcode
	 */
	//@{
	/** \see \ref sat::Solvable::ident. */
	IdString ident() const;

	/** \see \ref sat::Solvable::edition. */
	Edition edition() const;

	/** \see \ref sat::Solvable::arch. */
	Arch arch() const;
	//@}

	/** Implicit conversion to \ref Solvable */
	operator const Solvable &() const { return _solv; }
	/** \overload nonconst */
	operator Solvable &() { return _solv; }

      private:
	Solvable _solv;
	/** Pointer to implementation */
	RW_pointer<Impl> _pimpl;
    };

    /** \relates Transaction::Step Stream output */
    std::ostream & operator<<( std::ostream & str, const Transaction::Step & obj );

    /** \relates Transaction::StepType Stream output */
    std::ostream & operator<<( std::ostream & str, Transaction::StepType obj );

    /** \relates Transaction::StepStage Stream output */
    std::ostream & operator<<( std::ostream & str, Transaction::StepStage obj );

   ///////////////////////////////////////////////////////////////////
    namespace detail
    { /////////////////////////////////////////////////////////////////

      /** \ref Transaction iterator.
       */
      class Transaction_iterator : public boost::iterator_adaptor<
      Transaction_iterator		// Derived
      , const detail::IdType *		// Base
      , Transaction::Step		// Value
      , boost::forward_traversal_tag	// CategoryOrTraversal
      , Transaction::Step		// Reference
      >
      {
	public:
	  Transaction_iterator();
	  Transaction_iterator( const RW_pointer<Transaction::Impl> & pimpl_r, base_type id_r )
	  : Transaction_iterator::iterator_adaptor_( id_r )
	  , _pimpl( pimpl_r )
	  {}

	private:
	  friend class boost::iterator_core_access;

	  reference dereference() const
	  { return Transaction::Step( _pimpl, *base() ); }

	private:
	  friend class Transaction_const_iterator;
	  /** Pointer to implementation */
	  RW_pointer<Transaction::Impl> _pimpl;
      };

     /** \ref Transaction const_iterator.
       */
      class Transaction_const_iterator : public boost::iterator_adaptor<
      Transaction_const_iterator	// Derived
      , const detail::IdType *		// Base
      , const Transaction::Step		// Value
      , boost::forward_traversal_tag	// CategoryOrTraversal
      , const Transaction::Step		// Reference
      >
      {
	public:
	  Transaction_const_iterator();
	  Transaction_const_iterator( const Transaction_iterator & iter_r );
	  Transaction_const_iterator( const RW_pointer<Transaction::Impl> & pimpl_r, base_type id_r )
	  : Transaction_const_iterator::iterator_adaptor_( id_r )
	  , _pimpl( pimpl_r )
	  {}

	private:
	  friend class boost::iterator_core_access;

	  reference dereference() const
	  { return Transaction::Step( _pimpl, *base() ); }

	private:
	  /** Pointer to implementation */
	  RW_pointer<Transaction::Impl> _pimpl;
      };

       /////////////////////////////////////////////////////////////////
    } // namespace detail
    ///////////////////////////////////////////////////////////////////

    inline Transaction::const_iterator Transaction::find( const ResObject::constPtr & resolvable_r ) const
    { return( resolvable_r ? find( resolvable_r->satSolvable() ) : end() ); }

    inline Transaction::iterator Transaction::find( const ResObject::constPtr & resolvable_r )
    { return( resolvable_r ? find( resolvable_r->satSolvable() ) : end() ); }

    inline Transaction::const_iterator Transaction::find( const PoolItem & pi_r ) const
    { return find( pi_r.satSolvable() ); }

    inline Transaction::iterator Transaction::find( const PoolItem & pi_r )
    { return find( pi_r.satSolvable() ); }


    struct  Transaction::FilterAction
    {
      bool operator()( const Transaction::Step & step_r ) const
      { return step_r.stepType() != Transaction::TRANSACTION_IGNORE; }
    };

    inline Transaction::action_iterator Transaction::actionBegin() const
    { return make_filter_begin( FilterAction(), *this ); }

    inline Transaction::action_iterator Transaction::actionEnd() const
    { return make_filter_end( FilterAction(), *this ); }

     /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_TRANSACTION_H
