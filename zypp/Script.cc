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

  std::string Script::do_script() const
  { return pimpl().do_script(); }

  std::string Script::undo_script() const
  { return pimpl().undo_script(); }

  bool Script::undo_available() const
  { return pimpl().undo_available(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
