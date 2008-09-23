/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repo/PackageProvider.h
 *
*/
#ifndef ZYPP_REPO_PACKAGEPROVIDER_H
#define ZYPP_REPO_PACKAGEPROVIDER_H

#include <iosfwd>

#include "zypp/base/NonCopyable.h"

#include "zypp/ZYppCallbacks.h"
#include "zypp/Package.h"
#include "zypp/ManagedFile.h"
#include "zypp/repo/RepoProvideFile.h"
#include "zypp/repo/DeltaCandidates.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PackageProviderPolicy
    //
    /** */
    class PackageProviderPolicy
    {
    public:
      /** Get installed Editions callback signature. */
      typedef function<bool ( const std::string &, const Edition &, const Arch & )> QueryInstalledCB;

      /** Set callback. */
      PackageProviderPolicy & queryInstalledCB( QueryInstalledCB queryInstalledCB_r )
      { _queryInstalledCB = queryInstalledCB_r; return *this; }

      /** Evaluate callback. */
      bool queryInstalled( const std::string & name_r,
                           const Edition &     ed_r,
                           const Arch &        arch_r ) const;

    private:
      QueryInstalledCB _queryInstalledCB;
    };
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PackageProvider
    //
    /** Provide a package from a Source.
     * Use available deltarpm if apropriate.
    */
    class PackageProvider : private base::NonCopyable
    {
      typedef shared_ptr<void>                                       ScopedGuard;
      typedef callback::SendReport<repo::DownloadResolvableReport> Report;

      typedef packagedelta::DeltaRpm                         DeltaRpm;

    public:
      /** Ctor taking the Package to provide. */
      PackageProvider( RepoMediaAccess &access,
                       const Package::constPtr & package,
                       const DeltaCandidates & deltas,
                       const PackageProviderPolicy & policy_r = PackageProviderPolicy() );
      ~PackageProvider();

    public:
      /** Provide the package.
       * \throws Exception.
      */
      ManagedFile providePackage() const;

    private:
      ManagedFile doProvidePackage() const;
      ManagedFile tryDelta( const DeltaRpm & delta_r ) const;

    private:
      ScopedGuard newReport() const;
      Report & report() const;
      bool progressDeltaDownload( int value ) const;
      void progressDeltaApply( int value ) const;
      bool progressPackageDownload( int value ) const;
      bool failOnChecksumError() const;
      bool queryInstalled( const Edition & ed_r = Edition() ) const;

    private:
      PackageProviderPolicy      _policy;
      Package::constPtr          _package;
      mutable bool               _retry;
      mutable shared_ptr<Report> _report;
      DeltaCandidates            _deltas;
      RepoMediaAccess &          _access;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_PACKAGEPROVIDER_H
