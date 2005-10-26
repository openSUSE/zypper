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
#include <iostream>

#include "zypp/Script.h"
#include "zypp/detail/ScriptImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  IMPL_PTR_TYPE(Script)

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Script::Script
  //	METHOD TYPE : Ctor
  //
  Script::Script( detail::ScriptImplPtr impl_r )
  : Resolvable( impl_r )
  , _pimpl( impl_r )
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
  //	METHOD NAME : Script::do_script
  //	Get the script to perform the action
  //
  std::string Script::do_script()
  {
    return _pimpl->do_script();
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Script::undo_script
  //	Get the script to revert the action
  //
  std::string Script::undo_script()
  {
    return _pimpl->undo_script();
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Script::undo_available
  //	Check whether the action can be reverted
  //
  bool Script::undo_available()
  {
    return _pimpl->undo_available();
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
