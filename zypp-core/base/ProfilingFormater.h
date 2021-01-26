/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/ProfilingFormater.h
 *
*/
#ifndef ZYPP_BASE_PROFILINGFORMATER_H
#define ZYPP_BASE_PROFILINGFORMATER_H

#include <iosfwd>
#include <string>
#include <zypp-core/base/LogControl.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace base
  { /////////////////////////////////////////////////////////////////

    struct ProfilingFormater : public LogControl::LineFormater
    {
      virtual std::string format( const std::string & /*group_r*/,
                                  logger::LogLevel    /*level_r*/,
                                  const char *        /*file_r*/,
                                  const char *        /*func_r*/,
                                  int                 /*line_r*/,
                                  const std::string & /*message_r*/ );
      virtual ~ProfilingFormater() {}
    };


    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_PROFILINGFORMATER_H
