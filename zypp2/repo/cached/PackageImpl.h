/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef zypp_repo_cached_PackageImpl_H
#define zypp_repo_cached_PackageImpl_H

#include "zypp/detail/PackageImpl.h"
#include "zypp2/repo/cached/RepoImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace repo
{ /////////////////////////////////////////////////////////////////
namespace cached
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //        CLASS NAME : PackageImpl
  //
  class PackageImpl : public detail::PackageImplIf
  {
  public:

    PackageImpl( const data::RecordId &id, repo::cached::RepoImpl::Ptr repository_r );
    
    virtual TranslatedText summary() const;
    virtual TranslatedText description() const;
    virtual ByteCount size() const;
    virtual PackageGroup group() const;
    virtual ByteCount archivesize() const;
    virtual Pathname location() const;
    virtual bool installOnly() const;
    virtual Repository repository() const;
    virtual unsigned repositoryMediaNr() const;
    virtual Vendor vendor() const;

  protected:
    repo::cached::RepoImpl::Ptr _repository;
    TranslatedText _summary;
    TranslatedText _description;
    PackageGroup _group;
    Pathname _location;
    bool _install_only;
    unsigned _media_nr;

    ByteCount _size_installed;
    ByteCount _size_archive;

    bool _data_loaded;
    data::RecordId _id;
  };
  /////////////////////////////////////////////////////////////////
} // namespace cached
} // namespace repository
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZMD_BACKEND_DBSOURCE_DBPACKAGEIMPL_H

