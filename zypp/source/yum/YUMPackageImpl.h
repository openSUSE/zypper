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

#include "zypp/source/SourceImpl.h"
#include "zypp/detail/PackageImpl.h"
#include "zypp/parser/yum/YUMParserData.h"
#include "zypp/Changelog.h"
#include "zypp/CheckSum.h"

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
	  Source_Ref source_r,
	  const zypp::parser::yum::YUMPrimaryData & parsed,
	  const zypp::parser::yum::YUMFileListData & filelist,
	  const zypp::parser::yum::YUMOtherData & other
	);
	YUMPackageImpl(
	  Source_Ref source_r,
	  const zypp::parser::yum::YUMPatchPackage & parsed
	);

	/** Package summary */
	virtual TranslatedText summary() const;
	/** Package description */
	virtual TranslatedText description() const;
        /** */
        virtual TranslatedText licenseToConfirm() const;
	/** */
	virtual ByteCount size() const;
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
	virtual Changelog changelog() const;
        /** */
        virtual Pathname location() const;
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
	virtual ByteCount sourcesize() const;
	/** */
	virtual ByteCount archivesize() const;
	/** */
	virtual std::list<std::string> authors() const;
	/** */
	virtual std::list<std::string> filenames() const;
        /** */
        virtual std::string type() const;
        /** */
        virtual std::list<std::string> keywords() const;
        /** */
	virtual bool installOnly() const;
        /** */
	virtual unsigned sourceMediaNr() const;
        /** */
	virtual CheckSum checksum() const;
        /** */
	virtual std::list<DeltaRpm> deltaRpms() const;
        /** */
	virtual std::list<PatchRpm> patchRpms() const;

      protected:
	TranslatedText _summary;
	TranslatedText _description;
	TranslatedText _license_to_confirm;
	Date _buildtime;
	std::string _buildhost;
	std::string _url;
	Vendor _vendor;
	Label _license;
	std::string _packager;
	PackageGroup _group;
	Changelog _changelog;
	std::string _type;
	std::list<std::string> _authors;
	std::list<std::string> _keywords;
	unsigned _mediaNumber;
	CheckSum _checksum;
	std::list<std::string> _filenames;
	Pathname _location;
	std::list<DeltaRpm> _delta_rpms;
	std::list<PatchRpm> _patch_rpms;

	bool _install_only;

        unsigned int _package_size;
        unsigned int _size;

/*
	unsigned _size_package;
	std::string _sourcepkg;
	std::list<DirSize> _dir_sizes;
*/
/*
	std::list<ChangelogEntry> changelog;
*/
      private:
	Source_Ref _source;
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
#endif // ZYPP_SOURCE_YUM_YUMPACKAGEIMPL_H
