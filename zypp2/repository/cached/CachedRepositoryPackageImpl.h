/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zmd/backend/dbrepository/CachedRepositoryPackageImpl.h
 *
*/
#ifndef CachedRepositoryPackageImpl_H
#define CachedRepositoryPackageImpl_H

#include "zypp/detail/PackageImpl.h"
#include "zypp2/repository/cached/CachedRepositoryImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //        CLASS NAME : CachedRepositoryPackageImpl
  //
  class CachedRepositoryPackageImpl : public detail::PackageImplIf
  {
  public:

    CachedRepositoryPackageImpl( const data::RecordId &id, repository::cached::CachedRepositoryImpl::Ptr repository_r );
    
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
    repository::cached::CachedRepositoryImpl::Ptr _repository;
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
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZMD_BACKEND_DBSOURCE_DBPACKAGEIMPL_H

