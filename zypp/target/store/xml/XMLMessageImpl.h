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
  namespace store
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : XMLMessageImpl
    //
    /** Class representing the message to be shown during update */
    class XMLMessageImpl : public zypp::detail::MessageImplIf
    {
    public:
      /** Default ctor */
      XMLMessageImpl();
      /** Dtor */
      virtual ~XMLMessageImpl();

    public:
      /** Get the text of the message */
      virtual std::string text() const;
      /** Get the type of the message (YesNo / OK) */
      virtual std::string type() const;
    protected:
      /** The text of the message */
      std::string _text;
      /** The type of the message (YesNo / OK) */
      std::string _type;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_MESSAGEIMPL_H
