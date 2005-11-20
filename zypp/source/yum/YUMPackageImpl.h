/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMPackageImpl.h
 *
*/
#ifndef ZYPP_SOURCE_YUM_YUMPACKAGEIMPL_H
#define ZYPP_SOURCE_YUM_YUMPACKAGEIMPL_H

#include "zypp/detail/PackageImpl.h"
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
      //        CLASS NAME : YUMPackageImpl
      //
      /** Class representing an update script
       * \todo Fix brief descriptions, or delete them. This is not an update
       * script, It's implementation of zypp::Package for YUM source.
      */
      class YUMPackageImpl : public detail::PackageImpl
      {
      public:
        /** Default ctor */
        YUMPackageImpl( const zypp::parser::yum::YUMPatchPackage & parsed );
       };
      ///////////////////////////////////////////////////////////////////
    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_YUM_YUMPACKAGEIMPL_H
