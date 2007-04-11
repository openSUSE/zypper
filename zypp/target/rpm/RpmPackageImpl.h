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
#include "zypp/Source.h"
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
	virtual TranslatedText summary() const;
	/** Package description */
	virtual TranslatedText description() const;
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
	virtual std::list<std::string> filenames() const;
        /** */
        virtual std::string type() const;
        /** */
        virtual DiskUsage diskUsage() const;
        /** */
        virtual Source_Ref source() const;

	/** for 'local' RPMs  */
	void setLocation (const Pathname & pathname) { _location = pathname; }
	void setSource (Source_Ref source) { _source = source; }

      protected:
	TranslatedText _summary;
	TranslatedText _description;
	Date _buildtime;
	Date _installtime;
	std::string _buildhost;
	std::string _url;
	Vendor _vendor;
	Label _license;
	std::string _packager;
	PackageGroup _group;
	Changelog _changelog;
	Pathname _location;			// for 'local' rpms
	std::string _type;
	std::list<std::string> _filenames;
	DiskUsage _disk_usage;
	ByteCount _size;
	ByteCount _archivesize;
	Source_Ref _source;
       };
      ///////////////////////////////////////////////////////////////////
    } // namespace rpm
    /////////////////////////////////////////////////////////////////
  } // namespace target
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TARGET_RPM_RPMPACKAGEIMPL_H
