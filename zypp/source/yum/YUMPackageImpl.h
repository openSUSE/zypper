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
      /** Class representing a package
       *
       * \todo Deriving from detail::PackageImpl is useless here. PackageImpl
       * is dumb, i.e. it provides nothing, but the dtor needed to instanciate
       * detail::PackageImplIf. All values returned are the PackageImplIf
       * defaults. We should rename the detail::*Impl classes, and classify
       * them into Dumb (holding no real data, provided the ImplIf dtor is
       * the only prure virtual) and FullStore (providing a protected variable
       * and interface methods returning them for each datum),
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
