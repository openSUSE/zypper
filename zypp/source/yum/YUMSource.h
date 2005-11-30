/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMSource.h
 *
*/
#ifndef ZYPP_SOURCE_YUM_YUMSOURCE_H
#define ZYPP_SOURCE_YUM_YUMSOURCE_H

#include "zypp/source/Source.h"
#include "zypp/parser/yum/YUMParserData.h"
#include "zypp/Package.h"
#include "zypp/Message.h"
#include "zypp/Script.h"
#include "zypp/Patch.h"
#include "zypp/Product.h"

using namespace zypp::parser::yum;

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
      //        CLASS NAME : YUMSource
      //
      /** Class representing a YUM installation source
      */
      class YUMSource : public source::Source
      {
      public:
        /** Default ctor */
        YUMSource();

	Package::Ptr createPackage(
	  const zypp::parser::yum::YUMPrimaryData & parsed,
	  const zypp::parser::yum::YUMFileListData & filelist,
	  const zypp::parser::yum::YUMOtherData & other
	);
	Package::Ptr createPackage(
	  const zypp::parser::yum::YUMPatchPackage & parsed
	);
	Message::Ptr createMessage(
	  const zypp::parser::yum::YUMPatchMessage & parsed
	);
	Script::Ptr createScript(
	  const zypp::parser::yum::YUMPatchScript & parsed
	);
	Patch::Ptr createPatch(
	  const zypp::parser::yum::YUMPatchData & parsed
	);
	Product::Ptr createProduct(
	  const zypp::parser::yum::YUMProductData & parsed
	);


	Dependencies createDependencies(
	  const zypp::parser::yum::YUMObjectData & parsed,
	  const Resolvable::Kind my_kind
	);

	Capability createCapability(const YUMDependency & dep,
				    const Resolvable::Kind & my_kind);


       };
      ///////////////////////////////////////////////////////////////////
    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_YUM_YUMSOURCE_H
