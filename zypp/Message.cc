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
#include "zypp/Message.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Message::Message
  //	METHOD TYPE : Ctor
  //
  Message::Message( const std::string & name_r,
                    const Edition & edition_r,
                    const Arch & arch_r )
  : ResObject( ResKind("Message"), name_r, edition_r, arch_r )
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
  //	Message interface forwarded to implementation
  //
  ///////////////////////////////////////////////////////////////////

  std::string Message::text()
  { return pimpl().text(); }

  std::string Message::type()
  { return pimpl().type(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
