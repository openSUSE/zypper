/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMPatchImpl.h
 *
*/
#ifndef ZYPP_SOURCE_YUM_YUMPATCHIMPL_H
#define ZYPP_SOURCE_YUM_YUMPATCHIMPL_H

#include "zypp/detail/PatchImpl.h"
#include "zypp/parser/yum/YUMParserData.h"
#include "zypp/source/yum/YUMSource.h"

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
      //        CLASS NAME : YUMPatchImpl
      //
      /** Class representing a patch
      */
      class YUMPatchImpl : public detail::PatchImpl
      {
      public:
        /** Default ctor */
        YUMPatchImpl(
	  const zypp::parser::yum::YUMPatchData & parsed,
	  YUMSource * src
	);
       };
      ///////////////////////////////////////////////////////////////////
    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_YUM_YUMPATCHIMPL_H
