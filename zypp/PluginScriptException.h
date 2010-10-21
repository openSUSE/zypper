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

  /** Base class for \ref PluginScript \ref Exception. */
  class PluginScriptException : public Exception
  {
    public:
      PluginScriptException();
      PluginScriptException( const std::string & msg_r );
      PluginScriptException( const std::string & msg_r, const std::string & hist_r );
      virtual ~PluginScriptException() throw();
  };

  /** Convenience macro to declare more specific PluginScriptExceptions. */
#define declException( EXCP, BASE )								\
  class EXCP : public BASE {									\
    public:											\
      EXCP() : BASE( #EXCP ) {}									\
      EXCP( const std::string & msg_r ) : BASE( msg_r ) {}					\
      EXCP( const std::string & msg_r, const std::string & hist_r ) : BASE( msg_r, hist_r ) {}	\
      virtual ~EXCP() throw() {}								\
  }

  /** Script connection not open. */
  declException( PluginScriptNotConnected, PluginScriptException );

  /** Script died unexpectedly. */
  declException( PluginScriptDiedUnexpectedly, PluginScriptException );


  /** Communication timeout. */
  declException( PluginScriptTimeout, PluginScriptException );

  /** Timeout while sending data. */
  declException( PluginScriptSendTimeout, PluginScriptTimeout );

  /** Timeout while receiving data. */
  declException( PluginScriptReceiveTimeout, PluginScriptTimeout );

#undef declException

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PLUGINSCRIPTEXCEPTION_H
