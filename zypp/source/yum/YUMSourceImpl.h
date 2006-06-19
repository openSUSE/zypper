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
#include "zypp/detail/ResImplTraits.h"
#include "zypp/source/yum/YUMPackageImpl.h"
#include "zypp/parser/yum/YUMParserData.h"
#include "zypp/Package.h"
#include "zypp/Atom.h"
#include "zypp/Message.h"
#include "zypp/Script.h"
#include "zypp/Patch.h"
#include "zypp/Product.h"
#include "zypp/Selection.h"
#include "zypp/Pattern.h"

using namespace zypp::parser::yum;
using namespace zypp::filesystem;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    namespace yum
    { //////////////////////////////////////////////////////////////

      /**
       * class representing a YUM collection of metadata
       */
      
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

        virtual Date timestamp() const;
        virtual void storeMetadata(const Pathname & cache_dir_r);
	
	virtual std::string type(void) const
	{ return typeString(); }

	/** Text used for identifying the type of the source.
  	 * Used by the \ref SourceFactory when creating a 
	 * source of a given type only.
	 */
	static std::string typeString(void)
	{ return "YUM"; }

	virtual void createResolvables(Source_Ref source_r);

        /**
         * is the download of metadata from the url needed
         * \param localdir
         */
        bool downloadNeeded(const Pathname & localdir);
        
	Package::Ptr createPackage(
	  Source_Ref source_r,
	  const zypp::parser::yum::YUMPrimaryData & parsed,
	  const zypp::parser::yum::YUMFileListData & filelist,
	  const zypp::parser::yum::YUMOtherData & other,
	  zypp::detail::ResImplTraits<zypp::source::yum::YUMPackageImpl>::Ptr & impl
	);
        Atom::Ptr augmentPackage(
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
	  const zypp::parser::yum::YUMPatchMessage & parsed,
	  Patch::constPtr patch
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
        
        /** Check checksums of metadata files
         * \throw EXCEPTION on fail
         */
        void checkMetadataChecksums() const;
      private:
        
        const Pathname metadataRoot() const;
        bool cacheExists();

        const TmpDir downloadMetadata();
        void saveMetadataTo(const Pathname & dir_r);
        const Pathname repomdFile() const;
        const Pathname repomdFileSignature() const;
        const Pathname repomdFileKey() const;
        
	typedef struct {
	    zypp::detail::ResImplTraits<zypp::source::yum::YUMPackageImpl>::Ptr impl;
	    zypp::Package::Ptr package;
	} ImplAndPackage;

	typedef std::map<zypp::NVRA, ImplAndPackage> PackageImplMapT;
	PackageImplMapT _package_impl;

      };

      ///////////////////////////////////////////////////////////////////
    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_YUM_YUMSOURCEIMPL_H
