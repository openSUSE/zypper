/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef zypp_repo_cached_ProductImpl_H
#define zypp_repo_cached_ProductImpl_H

#include "zypp/detail/ProductImpl.h"
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
  //        CLASS NAME : ProductImpl
  //
  class ProductImpl : public detail::ProductImplIf
  {
  public:

    ProductImpl( const data::RecordId &id, repo::cached::RepoImpl::Ptr repository_r );
    
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
    virtual unsigned sourceMediaNr() const;
    
    // PRODUCT
    virtual std::string category() const;
    virtual Url releaseNotesUrl() const;
    virtual std::list<Url> updateUrls() const;
    virtual std::list<Url> extraUrls() const ;
    virtual std::list<Url> optionalUrls() const ;
    virtual std::list<std::string> flags() const;
    virtual TranslatedText shortName() const;
    virtual std::string distributionName() const;
    virtual Edition distributionEdition() const;   
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

