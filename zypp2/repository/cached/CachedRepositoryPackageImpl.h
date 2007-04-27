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
#include "zypp2/Repository.h"
//#include <sqlite3.h>

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

    CachedRepositoryPackageImpl( Repository repository_r );
    
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
    Repository _repository;
    TranslatedText _summary;
    TranslatedText _description;
    PackageGroup _group;
    Pathname _location;
    bool _install_only;
    unsigned _media_nr;

    ByteCount _size_installed;
    ByteCount _size_archive;

    bool _data_loaded;
  };
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZMD_BACKEND_DBSOURCE_DBPACKAGEIMPL_H

