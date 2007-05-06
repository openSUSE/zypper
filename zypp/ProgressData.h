/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ProgressData.h
 *
*/
#ifndef ZYPP_PROGRESSDATA_H
#define ZYPP_PROGRESSDATA_H

#include <iosfwd>
#include <string>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/ProvideNumericId.h"

#include "zypp/Date.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ProgressData
  //
  /** Maintain <tt>[min,max]</tt> and counter <tt>(value)</tt> for progress counting.
   *
   * This class should provide everything the producer of progress data
   * needs. As a convention, a zero sizes range indicates that you are just
   * able to send <em>'still alive'</em> triggers.
   *
   * The counter should be updated in reasonable intervals. Don't mind wheter
   * the counter value actually increased or not. ProgressData will recognize
   * your triggers and knows when to actually send notification to a consumer.
   *
   * Each ProgressData object provides a unique numeric id and you may assign
   * it a name.
   *
   * \todo complete the progess sending.
   */
  class ProgressData : public base::ProvideNumericId<ProgressData,unsigned>
  {
    public:
      typedef long long value_type;
      enum State { INIT, RUN, END };

    private:
      class Data
      {
	public:
	  Data( value_type min_r, value_type max_r, value_type val_r )
	  : _state( INIT ), _min( min_r ), _max( max_r ), _val( val_r )
	  , _last_val( 0 ), _last_send( 0 )
	  {}

	public:
	  State _state;
	  std::string _name;
	  value_type _min;
	  value_type _max;
	  value_type _val;

	  value_type _last_val;
	  Date       _last_send;

	private:
	  /** clone for RWCOW_pointer */
	  friend Data * rwcowClone<Data>( const Data * rhs );
	  Data * clone() const { return new Data( *this ); }
      };

    public:
      /** Ctor no range <tt>[0,0](0)</tt>. */
      ProgressData()
      : _d( new Data( 0, 0, 0 ) )
      {}

      /** Ctor <tt>[0,max](0)</tt>. */
      ProgressData( value_type max_r )
      : _d( new Data( 0, max_r, 0 ) )
      {}

      /** Ctor <tt>[min,max](min)</tt>. */
      ProgressData( value_type min_r, value_type max_r )
      : _d( new Data( min_r, max_r, min_r ) )
      {}

      /** Ctor <tt>[min,max](val)</tt>. */
      ProgressData( value_type min_r, value_type max_r, value_type val_r )
      : _d( new Data( min_r, max_r, val_r ) )
      {}

      ~ProgressData()
      {
	if ( _d->_state == RUN )
	{
	  _d->_state = END;
	  report();
	}
      }

    public:
      /** @return Current \c min value. */
      value_type min() const
      { return _d->_min; }

      /** @return Current \c max value. */
      value_type max() const
      { return _d->_max; }

      /** @return Current counter \c value. */
      value_type val() const
      { return _d->_val; }

      /** @return Wheter <tt>[min,max]</tt> defines a nonempty range. */
      bool hasRange() const
      { return min() != max(); }

      /** @return The counters name. */
      const std::string & name() const
      { return _d->_name; }

    public:
      /** Set new \c min value. */
      void min( value_type min_r )
      { _d->_min = min_r; }

      /** Set new \c max value. */
      void max( value_type max_r )
      { _d->_max = max_r; }

      /** Set no range <tt>[0,0]</tt>. */
      void noRange()
      { range( 0, 0 ); }

      /** Set new <tt>[0,max]</tt>. */
      void range( value_type max_r )
      { range( 0, max_r ); }

      /** Set new <tt>[min,max]</tt>. */
      void range( value_type min_r, value_type max_r )
      { min( min_r ); max( max_r ); }

    public:
      /** Set new counter \c value. */
      void set( value_type val_r )
      {
	_d->_val = val_r;
	report();
      }

      /** Increment counter \c value (default by 1). */
      void incr( value_type val_r = 1 )
      { set( val() + val_r ); }

      /** Decrement counter \c value (default by 1). */
      void decr( value_type val_r = 1 )
      { set( val() - val_r ); }

      /** Set counter value to current \c min value. */
      void toMin()
      { set( min() ); }

      /** Set counter value to current \c max value (unless no range). */
      void toMax()
      { if ( hasRange() ) set( max() ); }

      /** Leave counter value unchanged (still alive). */
      void tick()
      { set( val() ); }

    public:
      /** Set counter name. */
      void name( const std::string & name_r )
      { _d->_name = name_r; }

    private:
      /** Send report if necessary. */
      void report();

      /** Pointer to data. */
      RWCOW_pointer<Data> _d;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates ProgressData Stream output */
  std::ostream & operator<<( std::ostream & str, const ProgressData & obj );

  ///////////////////////////////////////////////////////////////////

  class InputStream;
  /** \relates ProgressData Setup from \ref InputStream. */
  ProgressData makeProgressData( const InputStream & input_r );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PROGRESSDATA_H
