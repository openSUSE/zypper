/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repository/memory/SrcPackageImpl.h
 *
*/
#ifndef ZYPP_SOURCE_MEMORYSRCPackageIMPL_H
#define ZYPP_SOURCE_MEMORYSRCPackageIMPL_H

#include "zypp/detail/SrcPackageImplIf.h"
#include "zypp/DiskUsage.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/repo/memory/RepoImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////
    namespace memory
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : SrcPackageImpl
      //
      /**
      */
      struct SrcPackageImpl : public zypp::detail::SrcPackageImplIf
      {
        SrcPackageImpl( memory::RepoImpl::Ptr repo, data::SrcPackage_Ptr ptr);
        virtual ~SrcPackageImpl();

        virtual TranslatedText summary() const;
        virtual TranslatedText description() const;
        virtual TranslatedText insnotify() const;
        virtual TranslatedText delnotify() const;
        virtual TranslatedText licenseToConfirm() const;
        virtual Vendor vendor() const;
        virtual ByteCount size() const;
        virtual bool installOnly() const;
        virtual Date buildtime() const;
        virtual Date installtime() const;
        
        virtual DiskUsage diskusage() const;
        virtual OnMediaLocation location() const;
private:
        repo::memory::RepoImpl::Ptr _repository;
        
        //ResObject
        TranslatedText _summary;
        TranslatedText _description;
        TranslatedText _insnotify;
        TranslatedText _delnotify;
        TranslatedText _license_to_confirm;
        Vendor _vendor;
        ByteCount _size;
        bool _install_only;
        Date _buildtime;
        Date _installtime;
        
        OnMediaLocation _location;
        DiskUsage _diskusage;      
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
#endif // ZYPP_SOURCE_MEMORY_SRCPACKAGEIMPL_H
