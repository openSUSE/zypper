/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repo/PackageProvider.cc
 *
*/
#include <iostream>
#include <sstream>
#include "zypp/repo/PackageDelta.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/UserRequestException.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/repo/PackageProvider.h"
#include "zypp/repo/Applydeltarpm.h"
#include "zypp/repo/PackageDelta.h"

#include "zypp/TmpPath.h"
#include "zypp/ZConfig.h"
#include "zypp/RepoInfo.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace repo
  {
    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PackageProviderPolicy
    //
    ///////////////////////////////////////////////////////////////////

    bool PackageProviderPolicy::queryInstalled( const std::string & name_r,
                                                const Edition &     ed_r,
                                                const Arch &        arch_r ) const
    {
      if ( _queryInstalledCB )
        return _queryInstalledCB( name_r, ed_r, arch_r );
      return false;
    }

    ///////////////////////////////////////////////////////////////////
    /// \class PackageProvider::Impl
    /// \brief PackageProvider implementation.
    ///////////////////////////////////////////////////////////////////
    class PackageProvider::Impl : private base::NonCopyable
    {
      typedef shared_ptr<void>                                     ScopedGuard;
      typedef callback::SendReport<repo::DownloadResolvableReport> Report;

      typedef packagedelta::DeltaRpm                         DeltaRpm;

    public:
      /** Ctor taking the Package to provide. */
      Impl( RepoMediaAccess &access,
	    const Package::constPtr & package,
	    const DeltaCandidates & deltas,
	    const PackageProviderPolicy & policy_r );

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

    PackageProvider::Impl::Impl(  RepoMediaAccess &access,
				  const Package::constPtr & package,
				  const DeltaCandidates & deltas,
				  const PackageProviderPolicy & policy_r )
    : _policy( policy_r )
    , _package( package )
    , _retry(false)
    , _deltas(deltas)
    , _access(access)
    {}

    ManagedFile PackageProvider::Impl::providePackage() const
    {
      Url url;
      RepoInfo info = _package->repoInfo();
      // FIXME we only support the first url for now.
      if ( info.baseUrlsEmpty() )
        ZYPP_THROW(Exception("No url in repository."));
      else
        url = * info.baseUrlsBegin();

      { // check for cache hit:
        OnMediaLocation loc( _package->location() );
        PathInfo cachepath( info.packagesPath() / loc.filename() );

        if ( cachepath.isFile() && ! loc.checksum().empty() ) // accept cache hit with matching checksum only!
             // Tempting to do a quick check for matching .rpm-filesize before computing checksum,
             // but real life shows that loc.downloadSize() and the .rpm-filesize frequently do not
             // match, even if loc.checksum() and the .rpm-files checksum do. Blame the metadata generator(s).
        {
          CheckSum cachechecksum( loc.checksum().type(), filesystem::checksum( cachepath.path(), loc.checksum().type() ) );
          if ( cachechecksum == loc.checksum() )
          {
            ManagedFile ret( cachepath.path() );
            if ( ! info.keepPackages() )
            {
              ret.setDispose( filesystem::unlink );
            }
            MIL << "provided Package from cache " << _package << " at " << ret << endl;
            return ret; // <-- cache hit
          }
        }
      }

      // HERE: cache misss, do download:
      MIL << "provide Package " << _package << endl;
      ScopedGuard guardReport( newReport() );
      ManagedFile ret;
      do {
        _retry = false;
        report()->start( _package, url );
        try  // ELIMINATE try/catch by providing a log-guard
          {
            ret = doProvidePackage();
          }
        catch ( const UserRequestException & excpt )
          {
            // UserRequestException e.g. from failOnChecksumError was already reported.
            ERR << "Failed to provide Package " << _package << endl;
            if ( ! _retry )
              {
                ZYPP_RETHROW( excpt );
              }
          }
        catch ( const Exception & excpt )
          {
            ERR << "Failed to provide Package " << _package << endl;
            if ( ! _retry )
              {
                // Aything else gets reported
                std::string package_str = _package->name() + "-" + _package->edition().asString();

                // TranslatorExplanation %s = name of the package being processed.
                std::string detail_str( str::form(_("Failed to provide Package %s. Do you want to retry retrieval?"), package_str.c_str() ) );
                detail_str += str::form( "\n\n%s", excpt.asUserHistory().c_str() );

                switch ( report()->problem( _package, repo::DownloadResolvableReport::IO, detail_str.c_str() ) )
                {
                      case repo::DownloadResolvableReport::RETRY:
                        _retry = true;
                        break;
                      case repo::DownloadResolvableReport::IGNORE:
                        ZYPP_THROW(SkipRequestException("User requested skip of corrupted file", excpt));
                        break;
                      case repo::DownloadResolvableReport::ABORT:
                        ZYPP_THROW(AbortRequestException("User requested to abort", excpt));
                        break;
                      default:
                        ZYPP_RETHROW( excpt );
                        break;
                }
              }
          }
      } while ( _retry );

      report()->finish( _package, repo::DownloadResolvableReport::NO_ERROR, std::string() );
      MIL << "provided Package " << _package << " at " << ret << endl;
      return ret;
    }

    ManagedFile PackageProvider::Impl::doProvidePackage() const
    {
      Url url;
      RepoInfo info = _package->repoInfo();
      // FIXME we only support the first url for now.
      if ( info.baseUrlsEmpty() )
        ZYPP_THROW(Exception("No url in repository."));
      else
        url = * info.baseUrlsBegin();

      // check whether to process patch/delta rpms
      if ( url.schemeIsDownloading() || ZConfig::instance().download_use_deltarpm_always() )
        {
          std::list<DeltaRpm> deltaRpms;
          if ( ZConfig::instance().download_use_deltarpm() )
          {
            _deltas.deltaRpms( _package ).swap( deltaRpms );
          }

          if ( ! ( deltaRpms.empty() )
               && queryInstalled() )
            {
              if ( ! deltaRpms.empty() && applydeltarpm::haveApplydeltarpm() )
                {
                  for( std::list<DeltaRpm>::const_iterator it = deltaRpms.begin();
                       it != deltaRpms.end(); ++it )
                    {
                      DBG << "tryDelta " << *it << endl;
                      ManagedFile ret( tryDelta( *it ) );
                      if ( ! ret->empty() )
                        return ret;
                    }
                }
            }
        }

      // no patch/delta -> provide full package
      ManagedFile ret;
      OnMediaLocation loc = _package->location();

      ProvideFilePolicy policy;
      policy.progressCB( bind( &PackageProvider::Impl::progressPackageDownload, this, _1 ) );
      policy.failOnChecksumErrorCB( bind( &PackageProvider::Impl::failOnChecksumError, this ) );
      return _access.provideFile( _package->repoInfo(), loc, policy );
    }

    ManagedFile PackageProvider::Impl::tryDelta( const DeltaRpm & delta_r ) const
    {
      if ( delta_r.baseversion().edition() != Edition::noedition
           && ! queryInstalled( delta_r.baseversion().edition() ) )
        return ManagedFile();

      if ( ! applydeltarpm::quickcheck( delta_r.baseversion().sequenceinfo() ) )
        return ManagedFile();

      report()->startDeltaDownload( delta_r.location().filename(),
                                    delta_r.location().downloadSize() );
      ManagedFile delta;
      try
        {
          ProvideFilePolicy policy;
          policy.progressCB( bind( &PackageProvider::Impl::progressDeltaDownload, this, _1 ) );
          delta = _access.provideFile( delta_r.repository().info(), delta_r.location(), policy );
        }
      catch ( const Exception & excpt )
        {
          report()->problemDeltaDownload( excpt.asUserHistory() );
          return ManagedFile();
        }
      report()->finishDeltaDownload();

      report()->startDeltaApply( delta );
      if ( ! applydeltarpm::check( delta_r.baseversion().sequenceinfo() ) )
        {
          report()->problemDeltaApply( _("applydeltarpm check failed.") );
          return ManagedFile();
        }

      // build the package and put it into the cache
      Pathname destination( _package->repoInfo().packagesPath() / _package->location().filename() );

      if ( ! applydeltarpm::provide( delta, destination,
                                     bind( &PackageProvider::Impl::progressDeltaApply, this, _1 ) ) )
        {
          report()->problemDeltaApply( _("applydeltarpm failed.") );
          return ManagedFile();
        }
      report()->finishDeltaApply();

      return ManagedFile( destination, filesystem::unlink );
    }

    PackageProvider::Impl::ScopedGuard PackageProvider::Impl::newReport() const
    {
      _report.reset( new Report );
      return shared_ptr<void>( static_cast<void*>(0),
                               // custom deleter calling _report.reset()
                               // (cast required as reset is overloaded)
                               bind( mem_fun_ref( static_cast<void (shared_ptr<Report>::*)()>(&shared_ptr<Report>::reset) ),
                                     ref(_report) ) );
    }

    PackageProvider::Impl::Report & PackageProvider::Impl::report() const
    { return *_report; }

    bool PackageProvider::Impl::progressDeltaDownload( int value ) const
    { return report()->progressDeltaDownload( value ); }

    void PackageProvider::Impl::progressDeltaApply( int value ) const
    { return report()->progressDeltaApply( value ); }

    bool PackageProvider::Impl::progressPackageDownload( int value ) const
    { return report()->progress( value, _package ); }

    bool PackageProvider::Impl::failOnChecksumError() const
    {
      std::string package_str = _package->name() + "-" + _package->edition().asString();

      // TranslatorExplanation %s = package being checked for integrity
      switch ( report()->problem( _package, repo::DownloadResolvableReport::INVALID, str::form(_("Package %s seems to be corrupted during transfer. Do you want to retry retrieval?"), package_str.c_str() ) ) )
        {
        case repo::DownloadResolvableReport::RETRY:
          _retry = true;
          break;
          case repo::DownloadResolvableReport::IGNORE:
          ZYPP_THROW(SkipRequestException("User requested skip of corrupted file"));
          break;
          case repo::DownloadResolvableReport::ABORT:
          ZYPP_THROW(AbortRequestException("User requested to abort"));
          break;
        default:
          break;
        }
      return true; // anyway a failure
    }

    bool PackageProvider::Impl::queryInstalled( const Edition & ed_r ) const
    { return _policy.queryInstalled( _package->name(), ed_r, _package->arch() ); }

    ///////////////////////////////////////////////////////////////////
    //	class PackageProvider
    ///////////////////////////////////////////////////////////////////

    PackageProvider::PackageProvider( RepoMediaAccess & access,
				      const Package::constPtr & package,
				      const DeltaCandidates & deltas,
				      const PackageProviderPolicy & policy_r )
    : _pimpl( new Impl( access, package, deltas, policy_r ) )
    {}

    PackageProvider::~PackageProvider()
    {}

    ManagedFile PackageProvider::providePackage() const
    { return _pimpl->providePackage(); }


  } // namespace repo
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
