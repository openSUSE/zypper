/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/memory/DPackageImpl.h
 *
*/
#ifndef ZYPP_SOURCE_MEMORYPACKAGEIMPL_H
#define ZYPP_SOURCE_MEMORYPACKAGEIMPL_H

#include "zypp/detail/PackageImplIf.h"
#include "zypp/Source.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/DiskUsage.h"
#include "zypp/CheckSum.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repository
  { /////////////////////////////////////////////////////////////////
    namespace memory
    { /////////////////////////////////////////////////////////////////

      DEFINE_PTR_TYPE(DImpl);

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : PackageImpl
      //
      /**
      */
      struct DPackageImpl : public zypp::detail::PackageImplIf
      {
        DPackageImpl( data::Package_Ptr ptr );
        virtual ~DPackageImpl();

        /** \name ResObject attributes. */
        //@{
        virtual TranslatedText summary() const;
        virtual TranslatedText description() const;
        virtual TranslatedText insnotify() const;
        virtual TranslatedText delnotify() const;
        virtual TranslatedText licenseToConfirm() const;
        virtual Source_Ref source() const;
        virtual unsigned sourceMediaNr() const;
        //@}

        virtual CheckSum checksum() const;
        virtual Date buildtime() const;
        virtual std::string buildhost() const;
        virtual Date installtime() const;
        virtual std::string distribution() const;
        virtual Vendor vendor() const;
        virtual Label license() const;
        virtual std::string packager() const;
        virtual PackageGroup group() const;
        virtual Keywords keywords() const;
	virtual Changelog changelog() const;
        virtual Pathname location() const;
        virtual std::string url() const;
        virtual std::string os() const;
        virtual Text prein() const;
        virtual Text postin() const;
        virtual Text preun() const;
        virtual Text postun() const;
        virtual ByteCount size() const;
        virtual ByteCount sourcesize() const;
        virtual ByteCount archivesize() const;
        virtual DiskUsage diskusage() const;
        virtual std::list<std::string> authors() const;
        virtual std::list<std::string> filenames() const;
        virtual std::list<DeltaRpm> deltaRpms() const;
        virtual std::list<PatchRpm> patchRpms() const;
        virtual bool installOnly() const;

        TranslatedText _summary;
        TranslatedText _description;
        TranslatedText _insnotify;
        TranslatedText _delnotify;
        TranslatedText _license_to_confirm;
        
        PackageGroup _group;
	Keywords _keywords;
        std::list<std::string> _authors;
        ByteCount _size;
        ByteCount _archivesize;
        Vendor _vendor;
        Label _license;
        Date _buildtime;
        unsigned _media_number;
        Pathname _location;
        DiskUsage _diskusage;
        CheckSum _checksum;
private:
      };
      ///////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////
    } // namespace memory
    /////////////////////////////////////////////////////////////////
  } // namespace repository
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PACKAGEIMPL_H
