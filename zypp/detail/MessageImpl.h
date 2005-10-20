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

#include <iosfwd>

#include "zypp/detail/ResolvableImpl.h"
//#include "zypp/Resolvable.h"
//#include "zypp/Message.h"

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
    class MessageImpl : public ResolvableImpl
    {
    public:
      /** Default ctor */
      MessageImpl( const std::string & name_r,
		   const Edition & edition_r,
		   const Arch & arch_r );
      /** Dtor */
      ~MessageImpl();

    public:
      /** Get the text of the message */
      virtual std::string text () const;
      /** Get the type of the message (YesNo / OK) */
      virtual std::string type () const;
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
