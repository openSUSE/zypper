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
#include "satsolver/queue.h"
}
#include <iostream>
#include "zypp/base/LogTools.h"

#include "zypp/sat/Queue.h"
#include "zypp/sat/Solvable.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    Queue::Queue()
      : _pimpl( new struct ::_Queue )
    {
      ::queue_init( _pimpl );
    }

    Queue::~Queue()
    {
      ::queue_free( _pimpl );
      delete( _pimpl );
    }

    bool Queue::empty() const
    { return( _pimpl->count == 0 ); }

    Queue::size_type Queue::size() const
    { return _pimpl->count; }

    Queue::const_iterator Queue::begin() const
    { return _pimpl->elements; }

    Queue::const_iterator Queue::end() const
    { return _pimpl->elements + _pimpl->count;}

    void Queue::push( value_type val_r )
    { ::queue_push( _pimpl, val_r ); }

    Queue::value_type Queue::pop()
    { ::queue_pop( _pimpl ); }

    Queue::value_type Queue::first() const
    {
      if ( empty() )
	return 0;
      return *_pimpl->elements;
    }

    void Queue::clear()
    { ::queue_empty( *this ); }

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

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
