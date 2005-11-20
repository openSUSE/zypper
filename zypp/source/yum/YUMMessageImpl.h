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
      /** Class representing an update script
       * \todo Fix brief descriptions, or delete them. This is not an update
       * script, It's implementation of zypp::Message for YUM source.
      */
      class YUMMessageImpl : public detail::MessageImpl
      {
      public:
        /** Default ctor */
        YUMMessageImpl( const zypp::parser::yum::YUMPatchMessage & parsed );
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
