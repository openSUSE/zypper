/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/SerialNumber.h
 *
*/
#ifndef ZYPP_BASE_SERIALNUMBER_H
#define ZYPP_BASE_SERIALNUMBER_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : SerialNumber
  //
  /** Simple serial number provider.
   *
   * \ref serial returns a serial number. The number returned stays
   * the same unless \ref setDirty was called to bring the object
   * into \c dirty state. The next call to \ref serial will increment
   * the serial number and bring the object into \c clean state.
   *
   * \code
   * SerialNumber sno;
   * sno.serial();             // SERIAL(0); () = clean
   * sno.setDirty();           // SERIAL*0*; ** = dirty
   * sno.serial();             // SERIAL(1)
   * sno.setDirty();           // SERIAL*1*
   * sno.setDirty();           // SERIAL*1*
   * sno.serial();             // SERIAL(2)
   * \endcode
   */
  class SerialNumber
  {
    friend std::ostream & operator<<( std::ostream & str, const SerialNumber & obj );

    public:
      /** Ctor taking initial \c dirty value. */
      SerialNumber( bool dirty_r = false );
      /** Dtor */
      virtual ~SerialNumber();

    public:
      void setDirty()
      { _dirty = true; }

    public:
      bool dirty() const
      { return _dirty; }

      bool clean() const
      { return !_dirty; }

      unsigned serial() const
      {
        if ( _dirty )
        {
          ++_serial;
          _dirty = false;
        }
        return _serial;
      }

    private:
      mutable bool     _dirty;
      mutable unsigned _serial;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates SerialNumber Stream output */
  std::ostream & operator<<( std::ostream & str, const SerialNumber & obj );

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : SerialNumberWatcher
  //
  /** Simple serial number watcher.
   *
   * \ref SerialNumberWatcher remembers a serial number
   * and tells whenever new numbers you feed change.
   *
   * All methods are overloaded to take an \unsigned or a
   * <tt>const SerialNumber &</tt> as argument.
   *
   * \code
   * SerialNumber sno;
   *
   * void check()
   * {
   *   static SerialNumberWatcher watcher( sno );
   *
   *   if ( watcher.remember( sno ) )
   *   {
   *     cout << "Serial number changed." << endl;
   *   }
   * }
   *
   * int main()
   * {
   *   check();          // This call would trigger, if check used a
   *                     // default constructed SerialNumberWatcher.
   *
   *   check();          //
   *   sno.dirty();
   *   check();          // "Serial number changed."
   *   check();          //
   *   sno.dirty();
   *   check();          // "Serial number changed."
   * \endcode
   */
  class SerialNumberWatcher
  {
    friend std::ostream & operator<<( std::ostream & str, const SerialNumberWatcher & obj );

    public:
      /** Ctor taking an initial \c serial value.
       *
       * A default constructed SerialNumberWatcher remembers the serial
       * number <tt>(unsigned)-1</tt>. So it is most likely the the 1st
       * call to \ref remember returns \ref isDirty.
       *
       * Vice versa, initializing the SerialNumberWatcher with the current
       * SerialNumber, most likely prevents the 1st to \ref remember to
       * return \ref isDirty.
      */
      SerialNumberWatcher( unsigned serial_r = (unsigned)-1 );
      /** Ctor taking an initial \c serial value. */
      SerialNumberWatcher( const SerialNumber & serial_r );
      /** Dtor */
      virtual ~SerialNumberWatcher();

    public:
      /** Return whether \c serial_r differs. */
      bool isDirty( unsigned serial_r ) const
      { return( _serial != serial_r ); }
      /** \overload */
      bool isDirty( const SerialNumber & serial_r ) const
      { return( _serial != serial_r.serial() ); }

      /** Return whether \c serial_r is still unchanged. */
      bool isClean( unsigned serial_r ) const
      { return( _serial == serial_r ); }
      /** \overload */
      bool isClean( const SerialNumber & serial_r ) const
      { return( _serial == serial_r.serial() ); }

    public:
      /** Return \ref isDirty, storing \c serial_r as new value. */
      bool remember( unsigned serial_r ) const
      {
        if ( isDirty( serial_r ) )
        {
          _serial = serial_r;
          return true;
        }
        return false;
      }
      /** \overload */
      bool remember( const SerialNumber & serial_r ) const
      { return remember( serial_r.serial() ); }

    private:
      mutable unsigned _serial;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates SerialNumberWatcher Stream output */
  std::ostream & operator<<( std::ostream & str, const SerialNumberWatcher & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_SERIALNUMBER_H
