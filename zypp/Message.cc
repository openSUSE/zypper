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
#include "zypp/Message.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE(Message);

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Message::Message
  //	METHOD TYPE : Ctor
  //
  Message::Message( const sat::Solvable & solvable_r )
  : ResObject( solvable_r )
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
  { return TranslatedText(); }

  /** Patch the message belongs to - if any */
  ResTraits<Patch>::constPtrType Message::patch() const
  { return ResTraits<Patch>::constPtrType(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
