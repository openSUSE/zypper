/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Message.h
 *
*/
#ifndef ZYPP_MESSAGE_H
#define ZYPP_MESSAGE_H

#include "zypp/Resolvable.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////
    DEFINE_PTR_TYPE(MessageImpl)
    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  DEFINE_PTR_TYPE(Message)

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Message
  //
  /** Class representing the message to be shown during update */
  class Message : public Resolvable
  {
  public:
    /** Default ctor */
    Message( detail::MessageImplPtr impl_r );
    /** Dtor */
    ~Message();
  public:
    /** Get the text of the message */
    std::string text();
    /** Get the type of the message (YesNo / OK) */
    std::string type();
  private:
    /** Pointer to implementation */
    detail::MessageImplPtr _pimpl;
  };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_MESSAGE_H
