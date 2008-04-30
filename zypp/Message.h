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

#include "zypp/TranslatedText.h"
#include "zypp/ResObject.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Message
  //
  /** Class representing the message to be shown during update.
   * \deprecated class is obsolete
  */
  class Message : public ResObject
  {
  public:
    typedef Message                         Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;

  public:
    /** Get the text of the message */
    TranslatedText text() const ZYPP_DEPRECATED;
    /** Patch the message belongs to - if any */
    ResTraits<Patch>::constPtrType patch() const ZYPP_DEPRECATED;

  protected:
    friend Ptr make<Self>( const sat::Solvable & solvable_r );
    /** Ctor */
    Message( const sat::Solvable & solvable_r ) ZYPP_DEPRECATED;
    /** Dtor */
    virtual ~Message();
  };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_MESSAGE_H
