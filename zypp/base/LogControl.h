/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/LogControl.h
 *
*/
#ifndef ZYPP_BASE_LOGCONTROL_H
#define ZYPP_BASE_LOGCONTROL_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace base
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : LogControl
    //
    /** Maintain logfile related options.
     * \note A Singleton using a Singleton implementation class,
     * that's why there is no _pimpl like in other classes.
    */
    class LogControl
    {
      friend std::ostream & operator<<( std::ostream & str, const LogControl & obj );

    public:
      /** Singleton access. */
      static LogControl instance()
      { return LogControl(); }

    public:
      /** Return path to the logfile.
       * An emty pathname for std::err.
      */
      const Pathname & logfile() const;

      /** Set path for the logfile.
       * An emty pathname for std::err.
       * \throw if \a logfile_r is not usable.
      */
      void logfile( const Pathname & logfile_r );

    public:
      /** Turn on excessive logging for the lifetime of this object.*/
      struct TmpExcessive
      {
        TmpExcessive();
        ~TmpExcessive();
      };

    private:
      /** Default ctor: Singleton */
      LogControl()
      {}
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates LogControl Stream output */
    std::ostream & operator<<( std::ostream & str, const LogControl & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_LOGCONTROL_H
