/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/ProfilingFormater.cc
 *
*/

#include <sys/time.h>

#include <iostream>
#include <fstream>
#include <string>

#include <zypp-core/base/String.h>

#include <zypp-core/base/ProfilingFormater.h>

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace base
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    // ProfilingFormater
    ///////////////////////////////////////////////////////////////////

    std::string ProfilingFormater::format( const std::string & group_r,
                                                  logger::LogLevel    level_r,
                                                  const char *        file_r,
                                                  const char *        func_r,
                                                  int                 line_r,
                                                  const std::string & message_r )
    {
      struct timeval tp;
      gettimeofday( &tp, NULL);

      return str::form( "%ld.%ld [%d] <%d> %s(%s):%d %s",
                        tp.tv_sec,
                        tp.tv_usec,
                        level_r,
                        getpid(),
                        /*group_r.c_str(),*/
                        file_r, func_r, line_r,
                        message_r.c_str() );
    }
    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
