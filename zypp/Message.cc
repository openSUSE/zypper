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

  IMPL_PTR_TYPE(Message);

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Message::Message
  //	METHOD TYPE : Ctor
  //
  Message::Message( const NVRAD & nvrad_r )
  : ResObject( TraitsType::kind, nvrad_r )
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

  TranslatedText Message::text() const
  { return pimpl().text(); }

  /** Patch the message belongs to - if any */
  Patch::constPtr Message::patch() const
  { return pimpl().patch(); }
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
