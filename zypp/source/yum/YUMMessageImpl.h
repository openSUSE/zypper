/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMMessageImpl.h
 *
*/
#ifndef ZYPP_SOURCE_YUM_YUMMESSAGEIMPL_H
#define ZYPP_SOURCE_YUM_YUMMESSAGEIMPL_H

#include "zypp/source/SourceImpl.h"
#include "zypp/detail/MessageImpl.h"
#include "zypp/parser/yum/YUMParserData.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace source
{ /////////////////////////////////////////////////////////////////
namespace yum
{ //////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : YUMMessageImpl
//
/** Class representing a message
*/
class YUMMessageImpl : public detail::MessageImplIf
{
public:
  /** Default ctor */
  YUMMessageImpl(
    Source_Ref source_r,
    const zypp::parser::yum::YUMPatchMessage & parsed,
    Patch::constPtr patch
  );
  /** Get the text of the message */
  virtual TranslatedText text() const;
  /** Patch the message belongs to - if any */
  Patch::constPtr patch() const;


protected:
  /** The text of the message */
  TranslatedText _text;
private:
  Source_Ref _source;
  Patch::constPtr _patch;
public:
  Source_Ref source() const;
  friend class YUMSourceImpl;
};
///////////////////////////////////////////////////////////////////
} // namespace yum
/////////////////////////////////////////////////////////////////
} // namespace source
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_YUM_YUMMESSAGEIMPL_H
