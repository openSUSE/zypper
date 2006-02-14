/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMSourceImpl.h
 *
*/
#ifndef ZYPP_SOURCE_YUM_YUMSOURCEIMPL_H
#define ZYPP_SOURCE_YUM_YUMSOURCEIMPL_H

#include "zypp/source/SourceImpl.h"
#include "zypp/parser/yum/YUMParserData.h"
#include "zypp/Package.h"
#include "zypp/Message.h"
#include "zypp/Script.h"
#include "zypp/Patch.h"
#include "zypp/Product.h"
#include "zypp/Selection.h"
#include "zypp/Pattern.h"

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
      //        CLASS NAME : YUMSourceImpl
      //
      /** Class representing a YUM installation source
      */
      class YUMSourceImpl : public SourceImpl
      {
      public:
        /** Default Ctor.
         * Just initilizes data members. Metadata retrieval
         * is delayed untill \ref factoryInit.
        */
        YUMSourceImpl();

      public:

        virtual void storeMetadata(const Pathname & cache_dir_r);
	
	virtual std::string type(void) const
	{ return "YUM"; }

	virtual void createResolvables(Source_Ref source_r);

	Package::Ptr createPackage(
	  Source_Ref source_r,
	  const zypp::parser::yum::YUMPrimaryData & parsed,
	  const zypp::parser::yum::YUMFileListData & filelist,
	  const zypp::parser::yum::YUMOtherData & other
	);
	Package::Ptr createPackage(
	  Source_Ref source_r,
	  const zypp::parser::yum::YUMPatchPackage & parsed
	);
	Selection::Ptr createGroup(
	  Source_Ref source_r,
	  const zypp::parser::yum::YUMGroupData & parsed
	);
	Pattern::Ptr createPattern(
	  Source_Ref source_r,
	  const zypp::parser::yum::YUMPatternData & parsed
	);
	Message::Ptr createMessage(
	  Source_Ref source_r,
	  const zypp::parser::yum::YUMPatchMessage & parsed
	);
	Script::Ptr createScript(
	  Source_Ref source_r,
	  const zypp::parser::yum::YUMPatchScript & parsed
	);
	Patch::Ptr createPatch(
	  Source_Ref source_r,
	  const zypp::parser::yum::YUMPatchData & parsed
	);
	Product::Ptr createProduct(
	  Source_Ref source_r,
	  const zypp::parser::yum::YUMProductData & parsed
	);


	Dependencies createDependencies(
	  const zypp::parser::yum::YUMObjectData & parsed,
	  const Resolvable::Kind my_kind
	);

	Dependencies createGroupDependencies(
	  const zypp::parser::yum::YUMGroupData & parsed
	);

	Capability createCapability(const YUMDependency & dep,
				    const Resolvable::Kind & my_kind);
      private:
        /** Ctor substitute.
         * Actually get the metadata.
         * \throw EXCEPTION on fail
        */
        virtual void factoryInit();

      private:

	std::list<Pathname> _metadata_files;

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
	friend inline bool operator<( const YUMSourceImpl::PackageID & lhs, const YUMSourceImpl::PackageID & rhs );
      };

      inline bool operator<( const YUMSourceImpl::PackageID & lhs, const YUMSourceImpl::PackageID & rhs )
      { return YUMSourceImpl::PackageID::compare( lhs, rhs ) == -1; }

      ///////////////////////////////////////////////////////////////////
    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_YUM_YUMSOURCEIMPL_H
