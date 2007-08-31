/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef zypp_repo_memory_PatchImpl_H
#define zypp_repo_memory_PatchImpl_H

#include "zypp/detail/PatchImpl.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/repo/memory/RepoImpl.h"
#include "zypp/Repository.h"
///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace repo
{ /////////////////////////////////////////////////////////////////
namespace memory
{ /////////////////////////////////////////////////////////////////

  class PatchImpl : public detail::PatchImplIf
  {
  public:

    PatchImpl( repo::memory::RepoImpl::Ptr repo, data::Patch_Ptr ptr);

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

    // PATCH
    virtual std::string id() const;
    virtual Date timestamp() const;
    virtual std::string category() const;
    virtual bool reboot_needed() const;
    virtual bool affects_pkg_manager() const;

    virtual Repository repository() const;

  protected:
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

    // patch
    std::string _patch_id;
    Date _timestamp;
    std::string _category;
    bool _reboot_needed;
    bool _affects_pkg_manager;
  };
  /////////////////////////////////////////////////////////////////
} // namespace memory
} // namespace repository
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZMD_BACKEND_DBSOURCE_DBPACKAGEIMPL_H

