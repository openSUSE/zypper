/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/Errno.h
 *
*/
#ifndef ZYPP_BASE_ERRNO_H
#define ZYPP_BASE_ERRNO_H

#include <cerrno>
#include <iosfwd>

#include "zypp/base/String.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /** Convenience \c errno wrapper. */
  class Errno
  {
    public:
      /** Default ctor: \c errno */
      Errno() : _errno( errno ) {}

      /** Ctor set to \c errno if error condition, else \c 0.
       * \code
       *  int ret = ::write( fd, buffer, size );
       *  DBG << "write returns: " << Errno( ret != size ) << end;
       *  // on success:    "write returns: [0-Success]"
       *  // on error e.g.: "write returns: [11-Resource temporarily unavailable]"
       * \endcode
       */
      Errno( bool error_r ) : _errno( error_r ? errno : 0 ) {}

      /** Ctor taking an explicit errno value. */
      Errno( int errno_r ) : _errno( errno_r ) {}

    public:
      /** Return the stored errno. */
      int get() const { return _errno; }

      /** Allow implicit conversion to \c int. */
      operator int() const { return get(); }

      /** Return human readable error string. */
      std::string asString() const { return str::form( "[%d-%s]", _errno, ::strerror(_errno) ); }

    private:
      int _errno;
  };

  /** \relates Errno Stream output */
  inline std::ostream & operator<<( std::ostream & str, const Errno & obj )
  { return str << obj.asString(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_ERRNO_H
