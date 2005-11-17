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
  Script::Script( const std::string & name_r,
                  const Edition & edition_r,
                  const Arch & arch_r )
  : ResObject( ResTraits<Self>::_kind, name_r, edition_r, arch_r )
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

  std::string Script::do_script()
  { return pimpl().do_script(); }

  std::string Script::undo_script()
  { return pimpl().undo_script(); }

  bool Script::undo_available()
  { return pimpl().undo_available(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
