/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/MessageImpl.h
 *
*/
#ifndef ZYPP_DETAIL_MESSAGEIMPL_H
#define ZYPP_DETAIL_MESSAGEIMPL_H

#include "zypp/detail/MessageImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : MessageImpl
    //
    /** Class representing the message to be shown during update */
    class MessageImpl : public MessageImplIf
    {
    public:
      /** Default ctor */
      MessageImpl();
      /** Dtor */
      virtual ~MessageImpl();

    public:
      /** Get the text of the message */
      virtual TranslatedText text() const;
    protected:
      /** The text of the message */
      TranslatedText _text;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_MESSAGEIMPL_H
