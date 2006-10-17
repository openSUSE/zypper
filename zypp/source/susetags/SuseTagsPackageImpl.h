/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/susetags/SuseTagsPackageImpl.h
 *
*/
#ifndef ZYPP_SOURCE_SUSETAGSPACKAGEIMPL_H
#define ZYPP_SOURCE_SUSETAGSPACKAGEIMPL_H

#include "zypp/detail/PackageImplIf.h"
#include "zypp/Source.h"
#include "zypp/DiskUsage.h"
#include "zypp/CheckSum.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////

      DEFINE_PTR_TYPE(SuseTagsImpl);

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : PackageImpl
      //
      /**
      */
      struct SuseTagsPackageImpl : public zypp::detail::PackageImplIf
      {
        SuseTagsPackageImpl(Source_Ref source_r);
        virtual ~SuseTagsPackageImpl();

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
        virtual ByteCount size() const;
        /** */
        virtual ByteCount sourcesize() const;
        /** */
        virtual ByteCount archivesize() const;
        /** */
        virtual DiskUsage diskusage() const;
        /** */
        virtual std::list<std::string> authors() const;
        /** */
        virtual std::list<std::string> filenames() const;
        /** */
        virtual std::list<DeltaRpm> deltaRpms() const;
        /** */
        virtual std::list<PatchRpm> patchRpms() const;
        /** */
        virtual bool installOnly() const;

        // which entry in sourceImpl::_package_data has
        // the shared data for this package
        NVRA _data_index;
        NVRA _nvra;

        PackageGroup _group;
        std::list<std::string> _authors;
        std::list<std::string> _keywords;
        ByteCount _size;
        ByteCount _archivesize;
        Label _license;
        Date _buildtime;
        unsigned _media_number;
        Pathname _location;
        DiskUsage _diskusage;
        CheckSum _checksum;

        SuseTagsImpl_Ptr _sourceImpl;

private:
        Source_Ref _source;
      };
      ///////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////
    } // namespace susetags
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PACKAGEIMPL_H
