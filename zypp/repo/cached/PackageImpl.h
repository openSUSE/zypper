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
  //        CLASS NAME : PackageImpl
  //
  class PackageImpl : public detail::PackageImplIf
  {
  public:

    PackageImpl( const data::RecordId &id, repo::cached::RepoImpl::Ptr repository_r );

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

    // PACKAGE
    virtual std::string buildhost() const;
    virtual std::string distribution() const;
    virtual Label license() const;
    virtual std::string packager() const;
    virtual PackageGroup group() const;
    virtual Keywords keywords() const;
    virtual Changelog changelog() const;
    virtual std::string url() const;
    virtual std::string os() const;
    virtual Text prein() const;
    virtual Text postin() const;
    virtual Text preun() const;
    virtual Text postun() const;
    virtual ByteCount sourcesize() const;
    virtual DiskUsage diskusage() const;
    virtual std::list<std::string> authors() const;
    virtual std::list<std::string> filenames() const;
    virtual OnMediaLocation location() const;
    virtual std::string sourcePkgName() const;
    virtual Edition sourcePkgEdition() const;

    virtual Repository repository() const;


  private:
    repo::cached::RepoImpl::Ptr _repository;
    data::RecordId _id;
  };
  /////////////////////////////////////////////////////////////////
} // namespace cached
} // namespace repository
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZMD_BACKEND_DBSOURCE_DBPACKAGEIMPL_H

