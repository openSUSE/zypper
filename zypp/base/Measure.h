/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/Measure.h
 *
*/
#ifndef ZYPP_BASE_MEASURE_H
#define ZYPP_BASE_MEASURE_H

#include <iosfwd>
#include <string>

#include "zypp/base/PtrTypes.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace debug
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Measure
    //
    /** Tool to measure elapsed real and process times.
     *
     * Timer is started by either passing a string to the ctor,
     * or callign \ref start. The string passed is printed on
     * all messages to help identifying the timer.
     *
     * Elapsed time is printed on calling \ref elapsed (timer
     * keeps running) or \ref stop.
     *
     * Calling \ref stop, stops the timer. The same, if the timer
     * goes out of scope.
     *
     * Elapsed time is printed as:
     * \code
     * 'REAL TIME' (u 'USER TIME' s 'SYSTEM TIME' c 'TIME OF CHILDREN')
     * \endcode
     * In brackets the time elapsed since a previous call to \ref elapsed.
     * All units are seconds.
     *
     * \code
     * Measure m( "Parse" );
     * ...
     * m.elapsed();
     * ...
     * m.elapsed();
     * ...
     * m.elapsed();
     * ...
     * m.stop();
     *
     * // START MEASURE(Parse)
     * // ELAPSED(Parse)  0 (u 0.13 s 0.00 c 0.00)
     * // ELAPSED(Parse)  0 (u 0.15 s 0.02 c 0.00) [ 0 (u 0.02 s 0.02 c 0.00)]
     * // ELAPSED(Parse)  0 (u 0.17 s 0.02 c 0.00) [ 0 (u 0.02 s 0.00 c 0.00)]
     * // MEASURE(Parse)  0 (u 0.17 s 0.02 c 0.00) [ 0 (u 0.00 s 0.00 c 0.00)]
     * \endcode
    */
    class Measure
    {
    public:
      /** Default Ctor does nothing. */
      Measure();

      /** Ctor taking \a ident_r string and auto starts timer. */
      explicit
      Measure( const std::string & ident_r );

      /** Dtor. */
      ~Measure();

      /** Start timer for \a ident_r string.
       * Implies stoping a running timer.
      */
      void start( const std::string & ident_r = std::string() );

      /** re start the timer without reset-ing it. */
      void restart();
      
      /** Print elapsed time for a running timer.
       * Timer keeps on running.
      */
      void elapsed() const;
      /** \overload Tagging the time with some text
       * \code
       * elapsed( "after action foo..." );
       * \endcode
       */
      void elapsed( const std::string & tag_r ) const;
      /** \overload Tagging the time with e.g. a line number
       * \code
       * elapsed( __LINE__ );
       * \endcode
       */
      void elapsed( long tag_r ) const;

      /** Stop a running timer. */
      void stop();

    private:
      /** Implementation. */
      class Impl;
      /** Pointer to implementation. */
      RW_pointer<Impl> _pimpl;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace debug
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_MEASURE_H
