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

#include "zypp/ResObject.h"
#include "zypp/detail/MessageImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Message
  //
  /** Class representing the message to be shown during update.
  */
  class Message : public ResObject
  {
  public:
    typedef Message                         Self;
    typedef detail::MessageImplIf           Impl;
    typedef base::intrusive_ptr<Self>       Ptr;
    typedef base::intrusive_ptr<const Self> constPtr;

  public:
    /** Get the text of the message */
    std::string text();
    /** Get the type of the message (YesNo / OK) */
    std::string type();

  protected:
    /** Ctor */
    Message( const std::string & name_r,
             const Edition & edition_r,
             const Arch & arch_r );
    /** Dtor */
    virtual ~Message();

  private:
    /** Access implementation */
    virtual Impl & pimpl() = 0;
    /** Access implementation */
    virtual const Impl & pimpl() const = 0;
  };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_MESSAGE_H
