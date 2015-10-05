/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/Queue.h
 */
#ifndef ZYPP_SAT_QUEUE_H
#define ZYPP_SAT_QUEUE_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/sat/detail/PoolMember.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace sat
  {
    class Queue;
    typedef Queue SolvableQueue;	///< Queue with Solvable ids
    typedef Queue StringQueue;		///< Queue with String ids

    ///////////////////////////////////////////////////////////////////
    /// \class Queue
    /// \brief Libsolv Id queue wrapper.
    /// \todo template value_type to work with IString and other Id based types
    ///////////////////////////////////////////////////////////////////
    class Queue
    {
      public:
	typedef unsigned size_type;
	typedef detail::IdType value_type;
	typedef const value_type* const_iterator;

      public:
        /** Default ctor: empty Queue. */
        Queue();

        /** Dtor */
        ~Queue();

	bool empty() const;
	size_type size() const;
	const_iterator begin() const;
	const_iterator end() const;

	/** Return iterator to the 1st occurance of \a val_r or \ref end. */
	const_iterator find( value_type val_r ) const;

	/** Return whether the Queue contais at lest one element with value \a val_r. */
	bool contains( value_type val_r ) const
	{ return( find( val_r ) != end() ); }

	/** Return the 1st Id in the queue or \c 0 if empty. */
	value_type first() const;

	/** Return the last Id in the queue or \c 0 if empty. */
	value_type last() const;

	/** Return the Id at \a idx_r in the queue
	 * \throws std::out_of_range if \a idx_r is out of range
	 */
	const value_type & at( size_type idx_r ) const;

	/** Return the Id at \a idx_r in the queue
	 * \throws std::out_of_range if \a idx_r is out of range
	 */
	value_type & at( size_type idx_r );

	/** Return the Id at \a idx_r in the queue (no range check) */
	const value_type & operator[]( size_type idx_r ) const;

	/** Return the Id at \a idx_r in the queue (no range check) */
	value_type & operator[]( size_type idx_r );

	/** Clear the queue. */
	void clear();

	/** Remove all occurances of \a val_r from the queue. */
	void remove( value_type val_r );

	/** Push a value to the end off the Queue. */
	void push( value_type val_r );
	/** \overload */
	void push_back( value_type val_r )
	{ push( val_r ); }

	/** Push a value if it's not yet in the Queue. */
	void pushUnique( value_type val_r );

	/** Pop and return the last Id from the queue or \c 0 if empty. */
	value_type pop();
	/** \overload */
	value_type pop_back()
	{ return pop(); }

	/** Push a value to the beginning off the Queue. */
	void push_front( value_type val_r );

	/** Pop and return the 1st Id from the queue or \c 0 if empty. */
	value_type pop_front();

     public:
	operator detail::CQueue *();			///< libsolv backdoor
	operator const detail::CQueue *() const		///< libsolv backdoor
	{ return _pimpl.get(); }
      private:
	RWCOW_pointer<detail::CQueue> _pimpl;		///< Pointer to implementation
    };

    /** \relates Queue Stream output */
    std::ostream & operator<<( std::ostream & str, const Queue & obj );

    /** \relates Queue Stream output assuming a Solvable queue. */
    std::ostream & dumpOn( std::ostream & str, const Queue & obj );

    /** \relates Queue */
    bool operator==( const Queue & lhs, const Queue & rhs );

    /** \relates Queue */
    inline bool operator!=( const Queue & lhs, const Queue & rhs )
    { return !( lhs == rhs ); }

  } // namespace sat
  ///////////////////////////////////////////////////////////////////

  /** \relates Queue Clone function for RWCOW_pointer */
  template<> sat::detail::CQueue * rwcowClone<sat::detail::CQueue>( const sat::detail::CQueue * rhs );

} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_QUEUE_H
