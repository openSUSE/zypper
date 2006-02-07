/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/XMLMessageImpl.h
 *
*/
#ifndef ZYPP_STORE_XMLMESSAGEIMPL_H
#define ZYPP_STORE_XMLMESSAGEIMPL_H

#include "zypp/detail/MessageImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace storage
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : XMLMessageImpl
    //
    /** Class representing the message to be shown during update */
    struct XMLMessageImpl : public zypp::detail::MessageImplIf
    {
    
      /** Default ctor */
      XMLMessageImpl();
      /** Dtor */
      virtual ~XMLMessageImpl();

      /** Get the text of the message */
      virtual TranslatedText text() const;
    
      /** The text of the message */
      TranslatedText _text;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace storage
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_MESSAGEIMPL_H
