/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef zypp_repo_cached_PatchImpl_H
#define zypp_repo_cached_PatchImpl_H

#include "zypp/detail/PatchImpl.h"
#include "zypp/repo/cached/RepoImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace repo
{ /////////////////////////////////////////////////////////////////
namespace cached
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //        CLASS NAME : PatchImpl
  //
  class PatchImpl : public detail::PatchImplIf
  {
  public:

    PatchImpl( const data::RecordId &id, repo::cached::RepoImpl::Ptr repository_r );
    
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
    
    virtual Source_Ref source() const;
    virtual unsigned mediaNr() const;
    
    // PATCH
    virtual std::string id() const;
    virtual Date timestamp() const;
    virtual std::string category() const;
    virtual bool reboot_needed() const;
    virtual bool affects_pkg_manager() const;
    virtual bool interactive() const;
    virtual AtomList all_atoms() const;
       
    virtual Repository repository() const;
    
  protected:
    repo::cached::RepoImpl::Ptr _repository;
    data::RecordId _id;
  };
  /////////////////////////////////////////////////////////////////
} // namespace cached
} // namespace repository
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZMD_BACKEND_DBSOURCE_DBPACKAGEIMPL_H

