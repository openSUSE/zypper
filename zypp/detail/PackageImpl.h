/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/PackageImpl.h
 *
*/
#ifndef ZYPP_DETAIL_PACKAGEIMPL_H
#define ZYPP_DETAIL_PACKAGEIMPL_H

#include <list>

#include "zypp/detail/ResolvableImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////
    DEFINE_PTR_TYPE(PackageImpl)

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PackageImpl
    //
    /** */
    class PackageImpl : public ResolvableImpl
    {
    public:
      /** */
      PackageImpl( const std::string & name_r,
                   const Edition & edition_r,
                   const Arch & arch_r );
      /** Dtor */
      virtual ~PackageImpl();

    public:
      /** */
      virtual std::string summary() const;
      /** */
      virtual std::list<std::string> description() const;
#if 0
      /** */
      virtual std::list<std::string> insnotify() const;
      /** */
      virtual std::list<std::string> delnotify() const;
      /** */
      virtual FSize size() const;
      /** */
      virtual bool providesSources() const;
      /** */
      virtual std::string instSrcLabel() const;
      /** */
      virtual Vendor instSrcVendor() const;
      /** */
      virtual unsigned instSrcRank() const;
      /** */
      virtual PkgSplitSet splitprovides() const;
      /** */
      virtual Date buildtime() const;
      /** */
      virtual std::string buildhost() const;
      /** */
      virtual Date installtime() const;
      /** */
      virtual std::string distribution() const;
      /** */
      virtual Vendor vendor() const;
      /** */
      virtual std::string license() const;
      /** */
      virtual std::list<std::string> licenseToConfirm() const;
      /** */
      virtual std::string packager() const;
      /** */
      virtual std::string group() const;
      /** */
      virtual YStringTreeItem * group_ptr() const;
      /** */
      virtual std::list<std::string> changelog() const;
      /** */
      virtual std::string url() const;
      /** */
      virtual std::string os() const;
      /** */
      virtual std::list<std::string> prein() const;
      /** */
      virtual std::list<std::string> postin() const;
      /** */
      virtual std::list<std::string> preun() const;
      /** */
      virtual std::list<std::string> postun() const;
      /** */
      virtual std::string sourceloc() const;
      /** */
      virtual FSize sourcesize() const;
      /** */
      virtual FSize archivesize() const;
      /** */
      virtual std::list<std::string> authors() const;
      /** */
      virtual std::list<std::string> filenames() const;
      /** */
      virtual std::list<std::string> recommends() const;
      /** */
      virtual std::list<std::string> suggests() const;
      /** */
      virtual std::string location() const;
      /** */
      virtual unsigned int medianr() const;
      /** */
      virtual std::list<std::string> keywords() const;
      /** */
      virtual std::string md5sum() const;
      /** */
      virtual std::string externalUrl() const;
      /** */
      virtual std::list<PkgEdition> patchRpmBaseVersions() const;
      /** */
      virtual FSize patchRpmSize() const;
      /** */
      virtual bool forceInstall() const;
      /** */
      virtual std::string patchRpmMD5() const;
      /** */
      virtual bool isRemote() const;
      /** */
      virtual PMError providePkgToInstall( Pathname& path_r ) const;
      /** */
      virtual PMError provideSrcPkgToInstall( Pathname& path_r ) const;
      /** */
      virtual constInstSrcPtr source() const;
      /** */
      virtual bool prefererCandidate() const;
      /** */
      virtual void du( PkgDu & dudata_r ) const;
      /** */
      virtual std::list<PMPackageDelta> deltas() const;
#endif
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PACKAGEIMPL_H
