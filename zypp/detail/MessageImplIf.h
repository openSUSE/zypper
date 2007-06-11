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
#include "zypp/Patch.h"

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
    /** Abstract Message implementation interface.
    */
    class MessageImplIf : public ResObjectImplIf
    {
    public:
      typedef Message ResType;

    public:
      /** Get the text of the message */
      virtual TranslatedText text() const PURE_VIRTUAL;
      /** Patch the message belongs to - if any */
      virtual Patch::constPtr patch() const; // TODO make it abstract = 0;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_MESSAGEIMPLIF_H
