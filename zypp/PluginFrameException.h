/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/PluginFrameException.h
 *
*/
#ifndef ZYPP_PLUGINFRAMEEXCEPTION_H
#define ZYPP_PLUGINFRAMEEXCEPTION_H

#include <iosfwd>

#include "zypp/base/Exception.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : PluginFrameException
  //
  /** Base class for \ref PluginFrame \ref Exception. */
  class PluginFrameException : public Exception
  {
    public:
      PluginFrameException();
      PluginFrameException( const std::string & msg_r );
      PluginFrameException( const std::string & msg_r, const std::string & hist_r );
      virtual ~PluginFrameException() throw();
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PLUGINFRAMEEXCEPTION_H
