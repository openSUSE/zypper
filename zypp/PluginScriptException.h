/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/PluginScriptException.h
 *
*/
#ifndef ZYPP_PLUGINSCRIPTEXCEPTION_H
#define ZYPP_PLUGINSCRIPTEXCEPTION_H

#include <iosfwd>

#include "zypp/base/Exception.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : PluginScriptException
  //
  /** Base class for \ref PluginScript \ref Exception. */
  class PluginScriptException : public Exception
  {
    public:
      PluginScriptException();
      PluginScriptException( const std::string & msg_r );
      PluginScriptException( const std::string & msg_r, const std::string & hist_r );
      virtual ~PluginScriptException() throw();
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PLUGINSCRIPTEXCEPTION_H
