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

#include "zypp/detail/PackageImpl.h"
#include "zypp/Source.h"
//#include <sqlite3.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace repository
{
namespace cached
{
  ///////////////////////////////////////////////////////////////////
  //
  //        CLASS NAME : CachedSourcePackageImpl
  //
  class SrcPackageImpl : public detail::SrcPackageImplIf
  {
  public:

    SrcPackageImpl( Source_Ref source_r );
    
    virtual TranslatedText summary() const;
    virtual TranslatedText description() const;
    virtual ByteCount size() const;
    virtual PackageGroup group() const;
    virtual ByteCount archivesize() const;
    virtual Pathname location() const;
    virtual bool installOnly() const;
    virtual Source_Ref source() const;
    virtual unsigned sourceMediaNr() const;
    virtual Vendor vendor() const;

  protected:
    Source_Ref _source;
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
