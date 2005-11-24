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
      */
      class YUMPackageImpl : public detail::PackageImplIf
      {
      public:
	/** Default ctor
	*/
	YUMPackageImpl(
	  const zypp::parser::yum::YUMPrimaryData & parsed
	);
	YUMPackageImpl(
	  const zypp::parser::yum::YUMPatchPackage & parsed
	);

	/** Package summary */
	virtual Label summary() const;
	/** Package description */
	virtual Text description() const;
	virtual Text insnotify() const;
	virtual Text delnotify() const;
	virtual FSize size() const;
	virtual bool providesSources() const;
	virtual Label instSrcLabel() const;
	virtual Vendor instSrcVendor() const;
	/** */
	virtual Date buildtime() const;
	/** */
	virtual std::string buildhost() const;
	/** */
	virtual Date installtime() const;
	/** */
	virtual std::string distribution() const;
	/** */
	virtual Vendor vendor() const;
	/** */
	virtual Label license() const;
	/** */
	virtual std::string packager() const;
	/** */
	virtual PackageGroup group() const;
	/** */
	virtual Text changelog() const;
	/** Don't ship it as class Url, because it might be
	 * in fact anything but a legal Url. */
	virtual std::string url() const;
	/** */
	virtual std::string os() const;
	/** */
	virtual Text prein() const;
	/** */
	virtual Text postin() const;
	/** */
	virtual Text preun() const;
	/** */
	virtual Text postun() const;
	/** */
	virtual FSize sourcesize() const;
	/** */
	virtual FSize archivesize() const;
	/** */
	virtual Text authors() const;
	/** */
	virtual Text filenames() const;
        virtual License licenseToConfirm() const;

#if 0
        /** */
        virtual std::list<std::string> insnotify() const;
        /** */
        virtual std::list<std::string> delnotify() const;
        /** */
        virtual unsigned packageSize() const;
        /** */
        virtual unsigned installedSize() const;
        /** */
        virtual unsigned archiveSize() const;
        /** */
        virtual bool providesSources() const;
        /** */
        virtual std::string instSrcLabel() const;
        /** */
        virtual std::string instSrcVendor() const;
        /** */
        virtual unsigned instSrcRank() const;
        /** */
        virtual std::string buildhost() const;
        /** */
        virtual std::string distribution() const;
        /** */
        virtual std::string vendor() const;
        /** */
        virtual std::string license() const;
        /** */
        virtual std::list<std::string> licenseToConfirm() const;
        /** */
        virtual std::string packager() const;
        /** */
        virtual std::string group() const;
        /** */
        virtual std::list<std::string> changelog() const;
        /** */
        virtual std::string url() const;
        /** */
        virtual std::string os() const;
        /** */
        virtual std::list<std::string> prein() const;
        /** */
        virtual std::list<std::string> postin() const;
        /** */
        virtual std::list<std::string> preun() const;
        /** */
        virtual std::list<std::string> postun() const;
        /** */
        virtual std::string sourcepkg() const;
        /** */
        virtual std::string type() const;
        /** */
        virtual std::list<std::string> authors() const;
        /** */
        virtual std::list<std::string> filenames() const;
        /** */
        virtual std::list<std::string> recommends() const;
        /** */
        virtual std::list<std::string> suggests() const;
        /** */
        virtual std::string location() const;
        /** */
        virtual unsigned int medianr() const;
        /** */
        virtual std::list<std::string> keywords() const;
        /** */
        virtual std::string md5sum() const;
        /** */
        virtual std::string externalUrl() const;
        /** */
        virtual std::list<Edition> patchRpmBaseVersions() const;
        /** */
        virtual unsigned patchRpmSize() const;
        /** */
        virtual bool forceInstall() const;
        /** */
        virtual std::string patchRpmMD5() const;
        /** */
        virtual bool isRemote() const;
        /** */
        virtual bool prefererCandidate() const;
      protected:
	unsigned _size_package;
	unsigned _size_installed;
	unsigned _size_archive;
	std::list<std::string> _authors;
	std::list<std::string>_keywords;
	std::string _packager;
	std::string _url;
	std::string _buildhost;
	std::string _vendor;
	std::string _group;
	std::string _license;
	std::string _sourcepkg;
	std::string _type;
	CheckSum _checksum;
	std::string _media;
	unsigned _time_file;
	unsigned _time_build;
	std::string _location;
	std::list<FileData> _files;
	std::list<DirSize> _dir_sizes;
	bool _install_only;
/*
	std::string summary;
	std::list<std::string> description;
	// SuSE specific data
	// Change Log
	std::list<ChangelogEntry> changelog;
	// Package Files
	struct {
	  std::string arch;
	  std::string filename;
	  std::string downloadsize;
	  std::string md5sum;
	  std::string buildtime;
	} plainRpm;
	struct {
	  std::string arch;
	  std::string filename;
	  std::string downloadsize;
	  std::string md5sum;
	  std::string buildtime;
	  std::list<YUMBaseVersion> baseVersions;
	} patchRpm;
	struct {
	  std::string arch;
	  std::string filename;
	  std::string downloadsize;


*/
#endif

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
