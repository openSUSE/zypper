/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef CachedRepoSrcPackageImpl_H
#define CachedRepoSrcPackageImpl_H

#include "zypp/detail/SrcPackageImpl.h"
#include "zypp/repo/cached/RepoImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace repo
{ /////////////////////////////////////////////////////////////////
namespace cached
{
  ///////////////////////////////////////////////////////////////////
  //
  //        CLASS NAME : SourcePackageImpl
  //
  class SrcPackageImpl : public detail::SrcPackageImplIf
  {
  public:

    SrcPackageImpl( const data::RecordId &id, repo::cached::RepoImpl::Ptr repository_r );

   public:
     /** Overloaded ResObjectImpl attribute.
      * \return The \ref location media number.
      */
     virtual unsigned mediaNr() const;

     /** Overloaded ResObjectImpl attribute.
      * \return The \ref location downloadSize.
      */
     virtual ByteCount downloadSize() const;

   public:
    virtual Repository repository() const;

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

    virtual OnMediaLocation location() const;
  private:
    repo::cached::RepoImpl::Ptr _repository;
    data::RecordId              _id;
  };
  /////////////////////////////////////////////////////////////////
} // ns cached
} // ns repo
} // namespace zypp

///////////////////////////////////////////////////////////////////
#endif // ZMD_BACKEND_DBSOURCE_DBPACKAGEIMPL_H
