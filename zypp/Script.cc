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

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE(Script);

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Script::Script
  //	METHOD TYPE : Ctor
  //
  Script::Script( const sat::Solvable & solvable_r )
  : ResObject( solvable_r )
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
  { return false; }

  std::string Script::doScriptInlined() const
  { return std::string(); }

  OnMediaLocation Script::doScriptLocation() const
  { return OnMediaLocation(); }

  bool Script::undoAvailable() const
  { return false; }

  std::string Script::undoScriptInlined() const
  { return std::string(); }

  OnMediaLocation Script::undoScriptLocation() const
  { return OnMediaLocation(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
