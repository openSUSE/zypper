/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/PackageImpl.h
 *
*/
#ifndef ZYPP_SOURCE_SUSETAGSPACKAGEIMPL_H
#define ZYPP_SOURCE_SUSETAGSPACKAGEIMPL_H

#include "zypp/detail/PackageImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////
  
      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : PackageImpl
      //
      /**
      */
      struct SuseTagsPackageImpl : public zypp::detail::PackageImplIf
      {
        SuseTagsPackageImpl();
        virtual ~SuseTagsPackageImpl();

        /** \name Rpm Package Attributes. */
        //@{
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
        virtual std::list<DeltaRpm> deltaRpms() const;
        /** */
        virtual std::list<PatchRpm> patchRpms() const;
        /** */
        virtual bool installOnly() const;

        virtual License licenseToConfirm() const PURE_VIRTUAL;

        PackageGroup _group;
        std::list<std::string> _authors;
        std::list<std::string> _keywords;
        ByteCount _sourcesize;
        ByteCount _archivesize;
        Label _license;
        Date _buildtime;
        //=Tim: 1111489970
        // what is this property

/*
=Grp: System/Base
=Lic: GPL
=Src: 3ddiag 0.724 3 src
=Tim: 1111489970
=Loc: 1 3ddiag-0.724-3.i586.rpm

*/

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
