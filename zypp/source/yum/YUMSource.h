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
#include "zypp/Selection.h"

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

	void parseSourceMetadata(std::string path);

	Package::Ptr createPackage(
	  const zypp::parser::yum::YUMPrimaryData & parsed,
	  const zypp::parser::yum::YUMFileListData & filelist,
	  const zypp::parser::yum::YUMOtherData & other
	);
	Package::Ptr createPackage(
	  const zypp::parser::yum::YUMPatchPackage & parsed
	);
	Selection::Ptr createGroup(
	  const zypp::parser::yum::YUMGroupData & parsed
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

	class PackageID {
	public:
	  PackageID(std::string name,
		    std::string ver,
		    std::string rel,
		    std::string arch)
	  : _name(name)
	  , _ver(ver)
	  , _rel(rel)
	  , _arch(arch)
	  {};
	  static int compare( const PackageID & lhs, const PackageID & rhs )
	  {
	    if (lhs._name < rhs._name)
	      return 1;
	    if (lhs._name > rhs._name)
	      return -1;
	    if (lhs._ver < rhs._ver)
	      return 1;
	    if (lhs._ver > rhs._ver)
	      return -1;
	    if (lhs._rel < rhs._rel)
	      return 1;
	    if (lhs._rel > rhs._rel)
	      return -1;
	    if (lhs._arch < rhs._arch)
	      return 1;
	    if (lhs._arch > rhs._arch)
	      return -1;
	    return 0;
	  }
	  std::string name() { return _name; }
	  std::string ver() { return _ver; }
	  std::string rel() { return _rel; }
	  std::string arch() { return _arch; }
	private:
	  std::string _name;
	  std::string _ver;
	  std::string _rel;
	  std::string _arch;
	};

      };
      inline bool operator<( const YUMSource::PackageID & lhs, const YUMSource::PackageID & rhs )
      { return YUMSource::PackageID::compare( lhs, rhs ) == -1; }

      ///////////////////////////////////////////////////////////////////
    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_YUM_YUMSOURCE_H
