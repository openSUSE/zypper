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

#include "zypp/ZYppCallbacks.h"
#include "zypp/Package.h"
#include "zypp/ManagedFile.h"
#include "zypp/repo/DeltaCandidates.h"
#include "zypp/repo/RepoProvideFile.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace repo
  {

    ///////////////////////////////////////////////////////////////////
    /// \class PackageProviderPolicy
    /// \brief Policies and options for \ref PackageProvider
    ///////////////////////////////////////////////////////////////////
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
    /// \class PackageProvider
    /// \brief Provide a package from a Repo.
    ///
    /// Use available deltarpm if apropriate.
    ///////////////////////////////////////////////////////////////////
    class PackageProvider
    {
    public:
      /** Ctor taking the Package to provide. */
      PackageProvider( RepoMediaAccess & access,
                       const Package::constPtr & package,
                       const DeltaCandidates & deltas,
                       const PackageProviderPolicy & policy_r = PackageProviderPolicy() );
      ~PackageProvider();

    public:
      /** Provide the package.
       * \throws Exception.
       */
      ManagedFile providePackage() const;

      /** Provide the package if it is cached. */
      ManagedFile providePackageFromCache() const;

      /** Whether the package is cached. */
      bool isCached() const;

    public:
      class Impl;              ///< Implementation class.
    private:
      RW_pointer<Impl> _pimpl; ///< Pointer to implementation.
    };
    ///////////////////////////////////////////////////////////////////

  } // namespace repo
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_PACKAGEPROVIDER_H
