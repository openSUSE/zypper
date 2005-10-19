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

#include <iosfwd>

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
  /** */
  class Message : public Resolvable
  {
  public:
    /** Default ctor */
    Message( detail::MessageImplPtr impl_r );
    /** Dtor */
    ~Message();
  public:
    std::string text ();
    std::string type ();
  private:
    /** Pointer to implementation */
    detail::MessageImplPtr _pimpl;
  };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_MESSAGE_H
