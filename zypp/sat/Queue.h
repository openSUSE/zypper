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

extern "C"
{
  struct _Queue;
}
#include <iosfwd>

#include "zypp/base/NonCopyable.h"
#include "zypp/sat/detail/PoolMember.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    /** Libsolv Id queue wrapper.
     */
    class Queue : private base::NonCopyable
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

	/** Clear the queue. */
	void clear();

	/** Remove all occurances of \a val_r from the queue. */
	void remove( value_type val_r );

	/** Push a value to the end off the Queue. */
	void push( value_type val_r );
	/** \overload */
	void push_back( value_type val_r )
	{ push( val_r ); }

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
	/** Backdoor */
	operator struct ::_Queue *()
	{ return _pimpl; }
	/** Backdoor */
	operator const struct ::_Queue *() const
	{ return _pimpl; }

      private:
        /** Pointer to implementation */
	struct ::_Queue * _pimpl;
    };

    /** \relates Queue Stream output */
    std::ostream & operator<<( std::ostream & str, const Queue & obj );

    /** \relates Queue Verbose stream output */
    std::ostream & dumpOn( std::ostream & str, const Queue & obj );

    /** \relates Queue */
    bool operator==( const Queue & lhs, const Queue & rhs );

    /** \relates Queue */
    inline bool operator!=( const Queue & lhs, const Queue & rhs )
    { return !( lhs == rhs ); }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_QUEUE_H
