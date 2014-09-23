/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/Queue.cc
 */
extern "C"
{
#include <solv/queue.h>
}
#include <iostream>
#include "zypp/base/LogTools.h"

#include "zypp/sat/Queue.h"
#include "zypp/sat/Solvable.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{

  template<>
  struct ::_Queue * rwcowClone<struct ::_Queue>( const struct ::_Queue * rhs )
  {
    struct ::_Queue * ret = new ::_Queue;
    ::queue_init_clone( ret, const_cast<struct ::_Queue *>(rhs) );
    return ret;
  }

  ///////////////////////////////////////////////////////////////////
  namespace sat
  {

    Queue::Queue()
      : _pimpl( new ::_Queue )
    { ::queue_init( _pimpl.get() ); }

    Queue::~Queue()
    { ::queue_free( _pimpl.get() ); }

    bool Queue::empty() const
    { return( _pimpl->count == 0 ); }

    Queue::size_type Queue::size() const
    { return _pimpl->count; }

    Queue::const_iterator Queue::begin() const
    { return _pimpl->elements; }

    Queue::const_iterator Queue::end() const
    { return _pimpl->elements + _pimpl->count;}

    Queue::const_iterator Queue::find( value_type val_r ) const
    {
      for_( it, begin(), end() )
	if ( *it == val_r )
	  return it;
      return end();
    }

    Queue::value_type Queue::first() const
    {
      if ( _pimpl->count )
	return *_pimpl->elements;
      return 0;
    }

    Queue::value_type Queue::last() const
    {
      if ( _pimpl->count )
	return _pimpl->elements[_pimpl->count-1];
      return 0;
    }

#define M_RANGE_CKECK(IDX,LOC) if ( IDX >= size_type(_pimpl->count) ) throw std::out_of_range( "zypp::sat::Queue::" LOC )

    const Queue::value_type & Queue::at( size_type idx_r ) const
    { M_RANGE_CKECK( idx_r, "at" ); return _pimpl->elements[idx_r]; }

    Queue::value_type & Queue::at( size_type idx_r )
    { M_RANGE_CKECK( idx_r, "at" ); return _pimpl->elements[idx_r]; }

    const Queue::value_type & Queue::operator[]( size_type idx_r ) const
    { return _pimpl->elements[idx_r]; }

    Queue::value_type & Queue::operator[]( size_type idx_r )
    { return _pimpl->elements[idx_r]; }

    void Queue::clear()
    { ::queue_empty( *this ); }

    void Queue::remove( value_type val_r )
    {
      for ( const_iterator it( find( val_r ) ); it != end(); it = find( val_r ) )
	::queue_delete( _pimpl.get(), it - begin() );
    }

    void Queue::push( value_type val_r )
    { ::queue_push( _pimpl.get(), val_r ); }

    void Queue::pushUnique( value_type val_r )
    { ::queue_pushunique( _pimpl.get(), val_r ); }

    Queue::value_type Queue::pop()
    { return ::queue_pop( _pimpl.get() ); }

    void Queue::push_front( value_type val_r )
    { ::queue_unshift( _pimpl.get(), val_r ); }

    Queue::value_type Queue::pop_front()
    { return ::queue_shift( _pimpl.get() ); }

    Queue::operator struct ::_Queue *()		// COW: nonconst version can't be inlined
    { return _pimpl.get(); }			// without exposing struct ::_Queue

    std::ostream & operator<<( std::ostream & str, const Queue & obj )
    { return dumpRangeLine( str << "Queue ", obj.begin(), obj.end() );  }

    std::ostream & dumpOn( std::ostream & str, const Queue & obj )
    {
      str << "Queue {";
      if ( ! obj.empty() )
      {
	str << endl;
	for_( it, obj.begin(), obj.end() )
	  str << "  " << Solvable(*it) << endl;
      }
      return str << "}";
    }

    bool operator==( const Queue & lhs, const Queue & rhs )
    {
      const struct ::_Queue * l = lhs;
      const struct ::_Queue * r = rhs;
      return( l == r || ( l->count == r->count && ::memcmp( l->elements, r->elements, l->count ) == 0 ) );
    }

  } // namespace sat
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
