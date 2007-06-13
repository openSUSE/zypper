/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repo/memory/PackageImpl.h
 *
*/
#ifndef ZYPP_SOURCE_MEMORYPACKAGEIMPL_H
#define ZYPP_SOURCE_MEMORYPACKAGEIMPL_H

#include "zypp/detail/PackageImplIf.h"
#include "zypp/Source.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/DiskUsage.h"
#include "zypp/CheckSum.h"
#include "zypp/Repository.h"
#include "zypp/repo/memory/RepoImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////
    namespace memory
    { /////////////////////////////////////////////////////////////////

      DEFINE_PTR_TYPE(DImpl);

      struct PackageImpl : public zypp::detail::PackageImplIf
      {
        PackageImpl(  repo::memory::RepoImpl::Ptr repo, data::Package_Ptr ptr );
        virtual ~PackageImpl();

        virtual Repository repository() const;
        
        virtual TranslatedText summary() const;
        virtual TranslatedText description() const;
        virtual TranslatedText insnotify() const;
        virtual TranslatedText delnotify() const;
        virtual TranslatedText licenseToConfirm() const;
        virtual Vendor vendor() const;
        virtual ByteCount size() const;
        virtual ByteCount archivesize() const;
        virtual bool installOnly() const;
        virtual Date buildtime() const;
        virtual Date installtime() const;
        virtual unsigned mediaNr() const;

        virtual CheckSum checksum() const;
        virtual std::string buildhost() const;
        virtual std::string distribution() const;
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
        virtual ByteCount sourcesize() const;
        virtual DiskUsage diskusage() const;
        virtual std::list<std::string> authors() const;
        virtual std::list<std::string> filenames() const;
        
        repo::memory::RepoImpl::Ptr _repository;
        
        //ResObject
        TranslatedText _summary;
        TranslatedText _description;
        TranslatedText _insnotify;
        TranslatedText _delnotify;
        TranslatedText _license_to_confirm;
        Vendor _vendor;
        ByteCount _size;
        ByteCount _archivesize;
        bool _install_only;
        Date _buildtime;
        Date _installtime;
        unsigned _media_nr;
        
        PackageGroup _group;
	Keywords _keywords;
        std::list<std::string> _authors;
        Label _license;
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
