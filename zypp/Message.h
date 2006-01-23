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
    typedef detail::MessageImplIf           Impl;
    typedef Message                         Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;

  public:
    /** Get the text of the message */
    std::string text() const;
    /** Get the type of the message (YesNo / OK) */
    std::string type() const;

  protected:
    Message( const NVRAD & nvrad_r );
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
