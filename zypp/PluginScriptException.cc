/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/PluginScriptException.cc
 *
*/
#include <iostream>
//#include "zypp/base/LogTools.h"

#include "zypp/PluginScriptException.h"
#include "zypp/PluginScript.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  PluginScriptException::PluginScriptException()
    : Exception( "PluginScriptException" )
  {}

  PluginScriptException::PluginScriptException( const std::string & msg_r )
    : Exception( msg_r )
  {}

  PluginScriptException::PluginScriptException( const std::string & msg_r, const std::string & hist_r )
    : Exception( msg_r )
  { addHistory( hist_r ); }

  PluginScriptException::~PluginScriptException() throw()
  {}

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
