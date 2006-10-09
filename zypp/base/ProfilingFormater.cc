/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/LogControl.cc
 *
*/
#include <iostream>
#include <fstream>
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/base/LogControl.h"
#include "zypp/base/String.h"
#include "zypp/Date.h"
#include "zypp/PathInfo.h"


#include "zypp/base/ProfilingFormater.h"

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
      static char hostname[1024];
      static char nohostname[] = "unknown";
      std::string now( Date::now().form( "%Y-%m-%d %H:%M:%S" ) );
      return str::form( "%s <%d> %s(%d) [%s] %s(%s):%d %s",
                        now.c_str(), level_r,
                        ( gethostname( hostname, 1024 ) ? nohostname : hostname ),
                        getpid(),
                        group_r.c_str(),
                        file_r, func_r, line_r,
                        message_r.c_str() );
    }
    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
