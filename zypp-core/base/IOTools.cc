/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/IOTools.cc
 *
*/

#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <glib.h>

#include <zypp-core/AutoDispose.h>
#include <zypp-core/base/IOTools.h>
#include <zypp-core/base/LogTools.h>

namespace zypp::io {

  BlockingMode setFILEBlocking (FILE * file, bool mode )
  {
    if ( !file ) return BlockingMode::FailedToSetMode;

    int fd = ::fileno( file );

    if ( fd == -1 )
    { ERR << strerror( errno ) << std::endl; return BlockingMode::FailedToSetMode; }

    int flags = ::fcntl( fd, F_GETFL );

    if ( flags == -1 )
    { ERR << strerror( errno ) << std::endl; return BlockingMode::FailedToSetMode; }

    BlockingMode oldMode = ( flags & O_NONBLOCK ) == O_NONBLOCK ? BlockingMode::WasNonBlocking : BlockingMode::WasBlocking;
    if ( !mode )
      flags = flags | O_NONBLOCK;
    else if ( flags & O_NONBLOCK )
      flags = flags ^ O_NONBLOCK;

    flags = ::fcntl( fd,F_SETFL,flags );

    if ( flags == -1 )
    { ERR << strerror(errno) << std::endl; return BlockingMode::FailedToSetMode; }

    return oldMode;
  }

  std::pair<ReceiveUpToResult, std::string> receiveUpto(FILE *file, char c, timeout_type timeout, bool failOnUnblockError )
  {
    FILE * inputfile = file;
    if ( !file )
      return std::make_pair( ReceiveUpToResult::Error, std::string() );

    int    inputfileFd = ::fileno( inputfile );

    size_t linebuffer_size = 0;
    zypp::AutoFREE<char> linebuf;

    const auto prevMode = setFILEBlocking( file, false );
    if ( prevMode == BlockingMode::FailedToSetMode && failOnUnblockError )
      return std::make_pair( ReceiveUpToResult::Error, std::string() );

    // reset the blocking mode when we are done
    zypp::OnScopeExit resetMode([ prevMode, fd = file ]( ){
      if ( prevMode == BlockingMode::WasBlocking )
        setFILEBlocking( fd, true );
    });

    bool haveTimeout = (timeout != no_timeout);
    int remainingTimeout = static_cast<int>( timeout );
    zypp::AutoDispose<GTimer *> timer( nullptr );
    if ( haveTimeout )
      timer = zypp::AutoDispose<GTimer *>( g_timer_new(), &g_free );

    std::string line;
    do
    {
      /* Watch inputFile to see when it has input. */

      GPollFD fd;
      fd.fd = inputfileFd;
      fd.events =  G_IO_IN | G_IO_HUP | G_IO_ERR;
      fd.revents = 0;

      if ( timer )
        g_timer_start( timer );

      clearerr( inputfile );

      int retval = g_poll( &fd, 1, remainingTimeout );
      if ( retval == -1 )
      {
        if ( errno != EINTR ) {
          ERR << "select error: " << strerror(errno) << std::endl;
          return std::make_pair( ReceiveUpToResult::Error, std::string() );
        }
      }
      else if ( retval )
      {
        // Data is available now.
        ssize_t nread = getdelim( &linebuf.value(), &linebuffer_size, c, inputfile );
        if ( nread == -1 ) {
          if ( ::feof( inputfile ) )
            return std::make_pair( ReceiveUpToResult::EndOfFile, line );
        }
        else
        {
          if ( nread > 0 )
            line += std::string( linebuf, nread );

          if ( ! ::ferror( inputfile ) || ::feof( inputfile ) ) {
            return std::make_pair( ReceiveUpToResult::Success, line ); // complete line
          }
        }
      }

      // we timed out, or were interrupted for some reason
      // check if we can wait more
      if ( timer ) {
        remainingTimeout -= g_timer_elapsed( timer, nullptr ) * 1000;
        if ( remainingTimeout <= 0 )
          return std::make_pair( ReceiveUpToResult::Timeout, line );
      }
    } while ( true );
  }

  TimeoutException::~TimeoutException() noexcept
  { }
}
