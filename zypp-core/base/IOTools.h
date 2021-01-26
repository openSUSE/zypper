/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/IOTools.h
 *
*/
#ifndef ZYPP_BASE_IOTOOLS_H
#define ZYPP_BASE_IOTOOLS_H

#include <stdio.h>
#include <utility>
#include <string>
#include <zypp-core/base/Exception.h>

namespace zypp::io {

  enum class BlockingMode{
    FailedToSetMode = -1, ///< Failed to block or unblock the fd
    WasBlocking,          ///< FD was blocking before
    WasNonBlocking        ///< FD was non blocking before
  };

  /**
   * Enables or disabled non blocking mode on a file descriptor.
   * The return value is one of the \ref zypp::io::BlockingMode values
   */
  BlockingMode setFILEBlocking ( FILE *file, bool mode = true );


  class TimeoutException : public Exception
  {
  public:
    /** Ctor taking message.
       * Use \ref ZYPP_THROW to throw exceptions.
      */
    TimeoutException() : Exception( "Timeout Exception" )
    {}
    /** Ctor taking message.
       * Use \ref ZYPP_THROW to throw exceptions.
      */
    TimeoutException( const std::string & msg_r )
      : Exception( msg_r )
    {}

    /** Dtor. */
    virtual ~TimeoutException() noexcept override;
  };

  enum ReceiveUpToResult {
    Success,
    Timeout,
    EndOfFile,
    Error,
  };

  using timeout_type = size_t;
  static const timeout_type no_timeout = static_cast<timeout_type>(-1);

  /*!
   * Reads data from \a file until it finds a seperator \a c, hits the end of the file or times out.
   * The \a timeout value is specified in milliseconds, a timeout of -1 means no timeout.
   * If \a failOnUnblockError is set to false the function will not error out if unblocking
   * the file descriptor did not work. The default is to fail if it's not possible to unblock the file.
   */
  std::pair<ReceiveUpToResult, std::string> receiveUpto( FILE * file, char c, timeout_type timeout, bool failOnUnblockError = true );

}

#endif
