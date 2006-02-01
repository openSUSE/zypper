/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/base/Logger.cc
 *
*/
#include <cstdlib>

#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace base
  { /////////////////////////////////////////////////////////////////

    namespace
    {
      std::ostream & noStream()
      {
        static std::ostream no_stream( 0 );
        return no_stream;
      }
    }

    ///////////////////////////////////////////////////////////////////
    namespace logger
    { /////////////////////////////////////////////////////////////////

      std::ostream & getStream( const char * group_r,
                                LogLevel     level_r,
                                const char * file_r,
                                const char * func_r,
                                const int    line_r )
      {
        static std::ostream & outStr( getenv("ZYPP_NOLOG") ? noStream()
                                                           : std::cerr );
        static std::ostream & fullStr( getenv("ZYPP_FULLLOG") ? outStr
                                                              : noStream() );
        return (level_r != E_XXX ? outStr : fullStr )
                 << str::form( "<%d> [%s] %s(%s):%d ",
                               level_r, group_r,
                               file_r, func_r, line_r );
      }

      /////////////////////////////////////////////////////////////////
    } // namespace logger
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
