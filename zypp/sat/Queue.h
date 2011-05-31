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

    /** Satsolver Id queue wrapper.
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

	/** Clear the queue. */
	void clear();

	/** Push a value to the end off the Queue. */
	void push( value_type val_r );

	/** Return the 1st Id in the queue or \c 0 if empty. */
	value_type pop();

	/** Remove and return the 1st Id from the queue or \c 0 if empty. */
	value_type first() const;

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

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_QUEUE_H
