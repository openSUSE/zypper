/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef zypp_repo_memory_ScriptImpl_H
#define zypp_repo_memory_ScriptImpl_H

#include "zypp/detail/ScriptImpl.h"
#include "zypp/repo/memory/RepoImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace repo
{ /////////////////////////////////////////////////////////////////
namespace memory
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //        CLASS NAME : ScriptImpl
  //
  class ScriptImpl : public detail::ScriptImplIf
  {
  public:

    ScriptImpl( memory::RepoImpl::Ptr repo, data::Script_Ptr ptr);

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

    // SCRIPT: TODO script attr

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
  };
  /////////////////////////////////////////////////////////////////
} // namespace memory
} // namespace repository
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZMD_BACKEND_DBSOURCE_DBPACKAGEIMPL_H

