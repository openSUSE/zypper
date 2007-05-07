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
#include "zypp/base/Function.h"
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
   * \code
   *      bool exampleReceiver( ProgressData::value_type v )
   *      {
   *        DBG << "got ->" << v << endl;
   *        return( v <= 100 ); // Abort if ( v > 100 )
   *      }
   *
   *      class Example
   *      {
   *        public:
   *
   *          Example( const ProgressData::ReceiverFnc & fnc_r = ProgressData::ReceiverFnc() )
   *          : _fnc( fnc_r )
   *          {}
   *
   *          void SendTo( const ProgressData::ReceiverFnc & fnc_r )
   *          { _fnc = fnc_r; }
   *
   *        public:
   *
   *          void action()
   *          {
   *            ProgressData tics( 10 );    // Expect range 0 -> 10
   *            tics.name( "test ticks" );  // Some arbitrary name
   *            tics.sendTo( _fnc );        // Send reports to _fnc
   *            tics.toMin();               // start sending min (0)
   *
   *            for ( int i = 0; i < 10; ++i )
   *            {
   *              if ( ! tics.set( i ) )
   *                return; // user requested abort
   *            }
   *
   *            tics.toMax(); // take care 100% are reported on success
   *          }
   *
   *          void action2()
   *          {
   *            ProgressData tics;          // Just send 'still alive' messages
   *            tics.name( "test ticks" );  // Some arbitrary name
   *            tics.sendTo( _fnc );        // Send reports to _fnc
   *            tics.toMin();               // start sending min (0)
   *
   *            for ( int i = 0; i < 10; ++i )
   *            {
   *              if ( ! tics.set( i ) )
   *                return; // user requested abort
   *            }
   *
   *            tics.toMax(); //
   *          }
   *
   *        private:
   *          ProgressData::ReceiverFnc _fnc;
   *      };
   * \endcode
   * \code
   *   Example t( exampleReceiver );
   *   DBG << "Reporting %:" << endl;
   *   t.action();
   *   DBG << "Reporting 'still alive':" << endl;
   *   t.action2();
   * \endcode
   * \code
   * Reporting %:
   * got ->0
   * got ->10
   * got ->20
   * got ->30
   * got ->40
   * got ->50
   * got ->60
   * got ->70
   * got ->80
   * got ->90
   * got ->100
   * got ->100
   * Reporting 'still alive':
   * got ->0
   * got ->9
   * \endcode
   *
   * The different ammount of triggers is due to different rules for sending
   * percent or 'still alive' messages.
   *
   * \todo Complete the progess sending.
   * \todo Tell recipient whether percentage or keepalive is sent,
   * the id and name might be helpfull, and maybe tell whether task
   * is abortable or not; i.e extend the ReceiverFnc signature.
   */
  class ProgressData : public base::ProvideNumericId<ProgressData,unsigned>
  {
    public:
      typedef long long value_type;
      /** Most simple version of progress reporting - a single value.
       * The percentage in most cases. Sometimes just keepalive.
       */
      typedef function<bool(value_type)> ReceiverFnc;

    private:
      enum State { INIT, RUN, END };

      class Data
      {
	public:
	  Data( value_type min_r, value_type max_r, value_type val_r )
	  : _state( INIT ), _min( min_r ), _max( max_r ), _val( val_r )
	  , _last_val( 0 ), _last_send( 0 )
	  {}

	public:
	  State       _state;
	  std::string _name;
	  value_type  _min;
	  value_type  _max;
	  value_type  _val;

	  ReceiverFnc _receiver;
	  value_type  _last_val;
	  Date        _last_send;

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
      /** Set counter name. */
      void name( const std::string & name_r )
      { _d->_name = name_r; }

      /** Set ReceiverFnc. */
      void sendTo( const ReceiverFnc & fnc_r )
      { _d->_receiver = fnc_r; }

      /** Set no ReceiverFnc. */
      void noSend()
      { _d->_receiver = ReceiverFnc(); }

    public:
      /** \name Progress reporting.
       *
       * These methods may actually cause a progress report to be sent.
       *
       * All methods return \c bool, because a progress receiver may
       * return \c false to indicate the desire to abort the pending
       * action. The incident is logged, but it's finaly up to the caller
       * to honor this.
       */
      //@{

      /** Set new counter \c value. */
      bool set( value_type val_r )
      {
	_d->_val = val_r;
	return report();
      }

      /** Increment counter \c value (default by 1). */
      bool incr( value_type val_r = 1 )
      { return set( val() + val_r ); }

      /** Decrement counter \c value (default by 1). */
      bool decr( value_type val_r = 1 )
      { return set( val() - val_r ); }

      /** Set counter value to current \c min value. */
      bool toMin()
      { return set( min() ); }

      /** Set counter value to current \c max value (unless no range). */
      bool toMax()
      { return set( hasRange() ? max() : val() ); }

      /** Leave counter value unchanged (still alive). */
      bool tick()
      { return set( val() ); }

      //@}

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

      /** @return The ReceiverFnc. */
      const ReceiverFnc & receiver() const
      { return _d->_receiver; }

    private:
      /** Send report if necessary. */
      bool report();

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
