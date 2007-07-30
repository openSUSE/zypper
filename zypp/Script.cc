/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Script.cc
 *
*/
#include "zypp/Script.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Script::Script
  //	METHOD TYPE : Ctor
  //
  Script::Script( const NVRAD & nvrad_r )
  : ResObject( TraitsType::kind, nvrad_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Script::~Script
  //	METHOD TYPE : Dtor
  //
  Script::~Script()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	Script interface forwarded to implementation
  //
  ///////////////////////////////////////////////////////////////////

  bool Script::doAvailable() const
  { return pimpl().doAvailable(); }

  std::string Script::doScriptInlined() const
  { return pimpl().doScriptInlined(); }

  OnMediaLocation Script::doScriptLocation() const
  { return pimpl().doScriptLocation(); }

  bool Script::undoAvailable() const
  { return pimpl().undoAvailable(); }

  std::string Script::undoScriptInlined() const
  { return pimpl().undoScriptInlined(); }

  OnMediaLocation Script::undoScriptLocation() const
  { return pimpl().undoScriptLocation(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
