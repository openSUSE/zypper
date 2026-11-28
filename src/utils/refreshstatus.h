/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_UTILS_REFRESHSTATUS_H
#define ZYPPER_UTILS_REFRESHSTATUS_H

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <string>

#include <unistd.h>

/** Reporting of progress from a parallel refresh child to its parent.
 *
 * The parent sets ZYPPER_REFRESH_STATUS_FD to a pipe it reads. Status
 * messages are simple lines:
 *   PROGRESS <percent|-1> <label>
 *   PROMPT_START
 *   PROMPT_END
 *
 * Outside of a parallel refresh (fd not set) all functions are no-ops.
 */
namespace refreshstatus
{
  inline int statusFd()
  {
    static const int fd = []() {
      const char * fdStr = ::getenv( "ZYPPER_REFRESH_STATUS_FD" );
      return fdStr ? std::atoi( fdStr ) : -1;
    }();
    return fd;
  }

  inline bool enabled()
  { return statusFd() >= 0; }

  inline void writeLine( const std::string & line_r )
  {
    if ( !enabled() )
      return;
    std::string line( line_r );
    line += '\n';
    if ( ::write( statusFd(), line.data(), line.size() ) < 0 )
    { /* parent gone, ignore */ }
  }

  /** Strip checksum prefixes from metadata file names (<sha>-primary.xml.gz). */
  inline std::string shortenLabel( const std::string & label_r )
  {
    const size_t dash = label_r.find( '-' );
    if ( dash == std::string::npos || dash < 32 )
      return label_r;
    for ( size_t i = 0; i < dash; ++i )
      if ( !std::isxdigit( static_cast<unsigned char>( label_r[i] ) ) )
        return label_r;
    return label_r.substr( dash + 1 );
  }

  /** Label of the current phase, used when only a percentage is reported. */
  inline std::string & currentLabel()
  {
    static thread_local std::string _label;
    return _label;
  }

  inline void setLabel( const std::string & label_r )
  { currentLabel() = shortenLabel( label_r ); }

  inline void progress( int percent_r, const std::string & label_r )
  {
    if ( !enabled() )
      return;
    std::string label( shortenLabel( label_r ) );
    currentLabel() = label;
    for ( char & c : label )
      if ( static_cast<unsigned char>( c ) < 0x20 )
        c = ' ';
    if ( label.size() > 200 )
      label.resize( 200 );

    // Avoid flooding the parent with identical updates
    static thread_local int lastPercent = -2;
    static thread_local std::string lastLabel;
    if ( percent_r == lastPercent && label == lastLabel )
      return;
    lastPercent = percent_r;
    lastLabel = label;

    writeLine( "PROGRESS " + std::to_string( percent_r ) + " " + label );
  }

  /** Report progress for the current phase (see \ref setLabel). */
  inline void progress( int percent_r )
  { progress( percent_r, currentLabel() ); }

  inline void flushOutput()
  {
    std::cout.flush();
    std::cerr.flush();
  }

  inline void promptStart()
  {
    if ( !enabled() )
      return;
    flushOutput();
    writeLine( "PROMPT_START" );
  }

  inline void promptEnd()
  {
    writeLine( "PROMPT_END" );
  }
}

#endif // ZYPPER_UTILS_REFRESHSTATUS_H
