/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/MessageImplIf.h
 *
*/
#ifndef ZYPP_DETAIL_MESSAGEIMPLIF_H
#define ZYPP_DETAIL_MESSAGEIMPLIF_H

#include "zypp/detail/ResObjectImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Message;

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : MessageImplIf
    //
    /** Abstact Message implementation interface.
    */
    class MessageImplIf : public ResObjectImplIf
    {
    public:
      typedef Message ResType;

    public:
      /** Get the text of the message */
      virtual std::string text() const = 0;
      /** Get the type of the message (YesNo / OK) */
      virtual std::string type() const = 0;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_MESSAGEIMPLIF_H
