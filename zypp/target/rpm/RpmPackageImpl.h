/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/rpm/RpmPackageImpl.h
 *
*/
#ifndef ZYPP_TARGET_RPM_RPMPACKAGEIMPL_H
#define ZYPP_TARGET_RPM_RPMPACKAGEIMPL_H

#include "zypp/detail/PackageImplIf.h"
#include "zypp/Changelog.h"
#include "zypp/target/rpm/RpmHeader.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////
    namespace rpm
    { //////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //        CLASS NAME : RPMPackageImpl
      //
      /** Class representing a package
      */
      class RPMPackageImpl : public detail::PackageImplIf
      {
      public:
	/** Default ctor
	*/
	RPMPackageImpl(
	  const RpmHeader::constPtr data
	);

	/** Package summary */
	virtual Label summary() const;
	/** Package description */
	virtual Text description() const;
	virtual Text insnotify() const;
	virtual Text delnotify() const;
	virtual ByteCount size() const;
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
	virtual Changelog changelog() const;
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
	virtual Text authors() const;
	/** */
	virtual Text filenames() const;
        /** */
        virtual License licenseToConfirm() const;
        /** */
        virtual std::string type() const;
        /** */
        virtual std::list<std::string> keywords() const;

      protected:
	Label _summary;
	Text _description;
	Date _buildtime;
	std::string _buildhost;
	std::string _url;
	Vendor _vendor;
	Label _license;
	std::string _packager;
	PackageGroup _group;
	Changelog _changelog;
	std::string _type;
	License _license_to_confirm;
	Text _authors;
	std::list<std::string>_keywords;
	Text _filenames;
       };
      ///////////////////////////////////////////////////////////////////
    } // namespace rpm
    /////////////////////////////////////////////////////////////////
  } // namespace target
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TARGET_RPM_RPMPACKAGEIMPL_H
