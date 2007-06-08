/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef zypp_repo_cached_PatternImpl_H
#define zypp_repo_cached_PatternImpl_H

#include "zypp/detail/PatternImpl.h"
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
  //        CLASS NAME : PatternImpl
  //
  class PatternImpl : public detail::PatternImplIf
  {
  public:

    PatternImpl( const data::RecordId &id, repo::cached::RepoImpl::Ptr repository_r );
    
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
    
    // PATTERN
    virtual bool isDefault() const;
    virtual bool userVisible() const;
    virtual TranslatedText category() const;
    virtual Pathname icon() const;
    virtual Pathname script() const;
    virtual Label order() const;
    //virtual std::set<std::string> install_packages( const Locale & lang = Locale("") ) const;
//     virtual const CapSet & includes() const;
//     virtual const CapSet & extends() const;
         
      
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

