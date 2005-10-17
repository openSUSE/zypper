/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Message.cc
 *
*/
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/Message.h"
#include "zypp/detail/ResolvableImpl.h"
#include "zypp//Resolvable.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Message::Message
  //	METHOD TYPE : Ctor
  //
  Message::Message( detail::MessageImplPtr impl_r )
  : Resolvable ( impl_r )
  , _pimpl( impl_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Message::~Message
  //	METHOD TYPE : Dtor
  //
  Message::~Message()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Message::text
  //	Get the text of the message
  //
  std::string Message::text ()
  {
    return _pimpl->text ();
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Message::text
  //	Get the type of the message ("OK", "YesNo")
  //
  std::string Message::type ()
  {
    return _pimpl->type ();
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
