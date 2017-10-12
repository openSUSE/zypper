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
#include <fstream>
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
#include "zypp/RepoManager.h"
#include "zypp/SrcPackage.h"

#include "zypp/ZYppFactory.h"
#include "zypp/Target.h"
#include "zypp/target/rpm/RpmDb.h"
#include "zypp/FileChecker.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace repo
  {
    ///////////////////////////////////////////////////////////////////
    //	class PackageProviderPolicy
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
    /// \brief PackageProvider implementation interface.
    ///////////////////////////////////////////////////////////////////
    struct PackageProvider::Impl : private base::NonCopyable
    {
      Impl() {}
      virtual ~Impl() {}

      /** Provide the package.
       * The basic workflow.
       * \throws Exception.
       */
      virtual ManagedFile providePackage() const = 0;

      /** Provide the package if it is cached. */
      virtual ManagedFile providePackageFromCache() const = 0;

      /** Whether the package is cached. */
      virtual bool isCached() const = 0;
    };

    ///////////////////////////////////////////////////////////////////
    /// \class PackageProviderImpl<TPackage>
    /// \brief PackageProvider implementation for \c Package and \c SrcPackage
    ///////////////////////////////////////////////////////////////////
    template <class TPackage>
    class PackageProviderImpl : public PackageProvider::Impl
    {
      typedef typename TPackage::constPtr TPackagePtr;	// Package or SrcPackage
      typedef callback::UserData UserData;
    public:
      /** Ctor taking the Package to provide. */
      PackageProviderImpl( RepoMediaAccess & access_r, const TPackagePtr & package_r,
			   const PackageProviderPolicy & policy_r )
      : _policy( policy_r )
      , _package( package_r )
      , _access( access_r )
      , _retry(false)
      {}

      virtual ~PackageProviderImpl() {}

    public:
      /** Provide the package.
       * The basic workflow.
       * \throws Exception.
       */
      virtual ManagedFile providePackage() const;

      /** Provide the package if it is cached. */
      virtual ManagedFile providePackageFromCache() const
      {
	ManagedFile ret( doProvidePackageFromCache() );
	if ( ! ( ret->empty() ||  _package->repoInfo().keepPackages() ) )
	  ret.setDispose( filesystem::unlink );
	return ret;
      }

      /** Whether the package is cached. */
      virtual bool isCached() const
      { return ! doProvidePackageFromCache()->empty(); }

    protected:
      typedef PackageProviderImpl<TPackage>	Base;
      typedef callback::SendReport<repo::DownloadResolvableReport>	Report;

      /** Lookup the final rpm in cache.
       *
       * A cache hit will return a non empty ManagedFile and an empty one on cache miss.
       *
       * \note File disposal depending on the repos keepPackages setting
       * are not set here, but in \ref providePackage or \ref providePackageFromCache.
       */
      ManagedFile doProvidePackageFromCache() const
      { return ManagedFile( _package->cachedLocation() ); }

      /** Actually provide the final rpm.
       * Report start/problem/finish and retry loop are hadled by \ref providePackage.
       * Here you trigger just progress and delta/plugin callbacks as needed.
       *
       * Proxy method for progressPackageDownload is provided here.
       * \code
       * ProvideFilePolicy policy;
       * policy.progressCB( bind( &Base::progressPackageDownload, this, _1 ) );
       * return _access.provideFile( _package->repoInfo(), loc, policy );
       * \endcode
       *
       * \note The provided default implementation retrieves the packages default
       * location.
       */
      virtual ManagedFile doProvidePackage() const
      {
	ManagedFile ret;
	OnMediaLocation loc = _package->location();

	ProvideFilePolicy policy;
	policy.progressCB( bind( &Base::progressPackageDownload, this, _1 ) );
	return _access.provideFile( _package->repoInfo(), loc, policy );
      }

    protected:
      /** Access to the DownloadResolvableReport */
      Report & report() const
      { return *_report; }

      /** Redirect ProvideFilePolicy package download progress to this. */
      bool progressPackageDownload( int value ) const
      {	return report()->progress( value, _package ); }

      typedef target::rpm::RpmDb RpmDb;

      RpmDb::CheckPackageResult packageSigCheck( const Pathname & path_r, bool isMandatory_r, UserData & userData ) const
      {
	if ( !_target )
	  _target = getZYpp()->getTarget();

	RpmDb::CheckPackageResult ret = RpmDb::CHK_ERROR;
	RpmDb::CheckPackageDetail detail;
	if ( _target )
	{
	  ret = _target->rpmDb().checkPackageSignature( path_r, detail );
	  if ( ret == RpmDb::CHK_NOSIG && !isMandatory_r )
	  {
	    WAR << "Relax CHK_NOSIG: Config says unsigned packages are OK" << endl;
	    ret = RpmDb::CHK_OK;
	  }
	}
	else
	  detail.push_back( RpmDb::CheckPackageDetail::value_type( ret, "OOps. Target is not initialized!" ) );

	userData.set( "CheckPackageResult", ret );
	userData.set( "CheckPackageDetail", std::move(detail) );
	return ret;
      }

      /** React on signature verification error user action
       * \note: IGNORE == accept insecure file (no SkipRequestException!)
       */
      void resolveSignatureErrorAction( repo::DownloadResolvableReport::Action action_r ) const
      {
	switch ( action_r )
	{
	  case repo::DownloadResolvableReport::RETRY:
	    _retry = true;
	    break;
	  case repo::DownloadResolvableReport::IGNORE:
	    WAR << _package->asUserString() << ": " << "User requested to accept insecure file" << endl;
	    break;
	  default:
	  case repo::DownloadResolvableReport::ABORT:
	    ZYPP_THROW(AbortRequestException("User requested to abort"));
	    break;
	}
      }

      /** Default signature verification error handling. */
      void defaultReportSignatureError( RpmDb::CheckPackageResult ret, const std::string & detail_r = std::string() ) const
      {
	str::Str msg;
	msg << _package->asUserString() << ": " << _("Signature verification failed") << " " << ret;
	if ( ! detail_r.empty() )
	  msg << "\n" << detail_r;
	resolveSignatureErrorAction( report()->problem( _package, repo::DownloadResolvableReport::INVALID, msg.str() ) );
      }

    protected:
      PackageProviderPolicy	_policy;
      TPackagePtr		_package;
      RepoMediaAccess &		_access;

    private:
      typedef shared_ptr<void>	ScopedGuard;

      ScopedGuard newReport() const
      {
	_report.reset( new Report );
	// Use a custom deleter calling _report.reset() when guard goes out of
	// scope (cast required as reset is overloaded). We want report to end
	// when leaving providePackage and not wait for *this going out of scope.
	return shared_ptr<void>( static_cast<void*>(0),
				 bind( mem_fun_ref( static_cast<void (shared_ptr<Report>::*)()>(&shared_ptr<Report>::reset) ),
				       ref(_report) ) );
      }

      mutable bool               _retry;
      mutable shared_ptr<Report> _report;
      mutable Target_Ptr         _target;
    };
    ///////////////////////////////////////////////////////////////////

    template <class TPackage>
    ManagedFile PackageProviderImpl<TPackage>::providePackage() const
    {
      ScopedGuard guardReport( newReport() );

      // check for cache hit:
      ManagedFile ret( providePackageFromCache() );
      if ( ! ret->empty() )
      {
	MIL << "provided Package from cache " << _package << " at " << ret << endl;
	report()->infoInCache( _package, ret );
	return ret; // <-- cache hit
      }

      // HERE: cache misss, check toplevel cache or do download:
      RepoInfo info = _package->repoInfo();

      // Check toplevel cache
      {
	RepoManagerOptions topCache;
	if ( info.packagesPath().dirname() != topCache.repoPackagesCachePath )	// not using toplevel cache
	{
	  const OnMediaLocation & loc( _package->location() );
	  if ( ! loc.checksum().empty() )	// no cache hit without checksum
	  {
	    PathInfo pi( topCache.repoPackagesCachePath / info.packagesPath().basename() / info.path() / loc.filename() );
	    if ( pi.isExist() && loc.checksum() == CheckSum( loc.checksum().type(), std::ifstream( pi.c_str() ) ) )
	    {
	      report()->start( _package, pi.path().asFileUrl() );
	      const Pathname & dest( info.packagesPath() / info.path() / loc.filename() );
	      if ( filesystem::assert_dir( dest.dirname() ) == 0 && filesystem::hardlinkCopy( pi.path(), dest ) == 0 )
	      {
		ret = ManagedFile( dest );
		if ( ! info.keepPackages() )
		  ret.setDispose( filesystem::unlink );

		MIL << "provided Package from toplevel cache " << _package << " at " << ret << endl;
		report()->finish( _package, repo::DownloadResolvableReport::NO_ERROR, std::string() );
		return ret; // <-- toplevel cache hit
	      }
	    }
	  }
	}
      }

      // FIXME we only support the first url for now.
      if ( info.baseUrlsEmpty() )
        ZYPP_THROW(Exception("No url in repository."));

      MIL << "provide Package " << _package << endl;
      Url url = * info.baseUrlsBegin();
      try {
      do {
        _retry = false;
	if ( ! ret->empty() )
	{
	  ret.setDispose( filesystem::unlink );
	  ret.reset();
	}
        report()->start( _package, url );
        try
          {
            ret = doProvidePackage();

	    if ( info.pkgGpgCheck() )
	    {
	      UserData userData( "pkgGpgCheck" );
	      ResObject::constPtr roptr( _package );	// gcc6 needs it more explcit. Has problem deducing
	      userData.set( "ResObject", roptr );	// a type for '_package->asKind<ResObject>()'...
	      /*legacy:*/userData.set( "Package", roptr->asKind<Package>() );
	      userData.set( "Localpath", ret.value() );
#if ( 1 )
	      bool pkgGpgCheckIsMandatory = info.pkgGpgCheckIsMandatory();
	      if ( str::startsWith( VERSION, "16.15." ) )
	      {
		// BSC#1038984: For a short period of time, libzypp-16.15.x
		// will silently accept unsigned packages IFF a repositories gpgcheck
		// configuration is explicitly turned OFF like this:
		//     gpgcheck      = 0
		//     repo_gpgcheck = 0
		//     pkg_gpgcheck  = 1
		// This will allow some already released products to adapt to the behavioral
		// changes introduced by fixing BSC#1038984, while systems with a default
		// configuration (gpgcheck = 1) already benefit from the fix.
		// With libzypp-16.16.x the above configuration will reject unsigned packages
		// as it should.
		if ( pkgGpgCheckIsMandatory && !info.gpgCheck() && !info.repoGpgCheck() )
		  pkgGpgCheckIsMandatory = false;
	      }
	      RpmDb::CheckPackageResult res = packageSigCheck( ret, pkgGpgCheckIsMandatory, userData );
#else
	      RpmDb::CheckPackageResult res = packageSigCheck( ret, info.pkgGpgCheckIsMandatory(), userData );
#endif
	      // publish the checkresult, even if it is OK. Apps may want to report something...
	      report()->pkgGpgCheck( userData );

	      if ( res != RpmDb::CHK_OK )
	      {
		if ( userData.hasvalue( "Action" ) )	// pkgGpgCheck report provided an user error action
		{
		  resolveSignatureErrorAction( userData.get( "Action", repo::DownloadResolvableReport::ABORT ) );
		}
		else if ( userData.haskey( "Action" ) )	// pkgGpgCheck requests the default problem report (wo. details)
		{
		  defaultReportSignatureError( res );
		}
		else					// no advice from user => usedefaults
		{
		  switch ( res )
		  {
		    case RpmDb::CHK_OK:		// Signature is OK
		      break;

		    case RpmDb::CHK_NOKEY:	// Public key is unavailable
		    case RpmDb::CHK_NOTFOUND:	// Signature is unknown type
		    case RpmDb::CHK_FAIL:	// Signature does not verify
		    case RpmDb::CHK_NOTTRUSTED:	// Signature is OK, but key is not trusted
		    case RpmDb::CHK_ERROR:	// File does not exist or can't be opened
		    case RpmDb::CHK_NOSIG:	// File is unsigned
		    default:
		      // report problem (w. details), throw if to abort, else retry/ignore
		      defaultReportSignatureError( res, str::Str() << userData.get<RpmDb::CheckPackageDetail>( "CheckPackageDetail" ) );
		      break;
		  }
		}
	      }
	    }
          }
        catch ( const UserRequestException & excpt )
          {
            ERR << "Failed to provide Package " << _package << endl;
	    if ( ! _retry )
	      ZYPP_RETHROW( excpt );
          }
        catch ( const FileCheckException & excpt )
          {
	    ERR << "Failed to provide Package " << _package << endl;
	    if ( ! _retry )
	    {
	      const std::string & package_str = _package->asUserString();
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
	    }
	  }
        catch ( const Exception & excpt )
          {
            ERR << "Failed to provide Package " << _package << endl;
            if ( ! _retry )
	    {
                // Aything else gets reported
                const std::string & package_str = _package->asUserString();

                // TranslatorExplanation %s = name of the package being processed.
                std::string detail_str( str::form(_("Failed to provide Package %s. Do you want to retry retrieval?"), package_str.c_str() ) );
                detail_str += str::form( "\n\n%s", excpt.asUserHistory().c_str() );

                switch ( report()->problem( _package, repo::DownloadResolvableReport::IO, detail_str.c_str() ) )
                {
                      case repo::DownloadResolvableReport::RETRY:
                        _retry = true;
                        break;
                      case repo::DownloadResolvableReport::IGNORE:
                        ZYPP_THROW(SkipRequestException("User requested skip of file", excpt));
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
      } catch(...){
	// bsc#1045735: Be sure no invalid files stay in the cache!
	// TODO: It would be better to provide a filechecker passed to the
	// fetcher that performs the pkgGpgCheck. This way a bad file would be
	// discarded before it's moved to the cache.
	// For now make sure the file gets deleted (even if keeppackages=1).
	if ( ! ret->empty() )
	  ret.setDispose( filesystem::unlink );
	throw;
      }

      report()->finish( _package, repo::DownloadResolvableReport::NO_ERROR, std::string() );
      MIL << "provided Package " << _package << " at " << ret << endl;
      return ret;
    }


    ///////////////////////////////////////////////////////////////////
    /// \class RpmPackageProvider
    /// \brief RPM PackageProvider implementation (with deltarpm processing).
    ///////////////////////////////////////////////////////////////////
    class RpmPackageProvider : public PackageProviderImpl<Package>
    {
    public:
      RpmPackageProvider( RepoMediaAccess & access_r,
			  const Package::constPtr & package_r,
			  const DeltaCandidates & deltas_r,
			  const PackageProviderPolicy & policy_r )
      : PackageProviderImpl<Package>( access_r, package_r, policy_r )
      , _deltas( deltas_r )
      {}

    protected:
      virtual ManagedFile doProvidePackage() const;

    private:
      typedef packagedelta::DeltaRpm	DeltaRpm;

      ManagedFile tryDelta( const DeltaRpm & delta_r ) const;

      bool progressDeltaDownload( int value ) const
      { return report()->progressDeltaDownload( value ); }

      void progressDeltaApply( int value ) const
      { return report()->progressDeltaApply( value ); }

      bool queryInstalled( const Edition & ed_r = Edition() ) const
      { return _policy.queryInstalled( _package->name(), ed_r, _package->arch() ); }

    private:
      DeltaCandidates		_deltas;
    };
    ///////////////////////////////////////////////////////////////////

    ManagedFile RpmPackageProvider::doProvidePackage() const
    {
      Url url;
      RepoInfo info = _package->repoInfo();
      // FIXME we only support the first url for now.
      if ( info.baseUrlsEmpty() )
        ZYPP_THROW(Exception("No url in repository."));
      else
        url = * info.baseUrlsBegin();

      // check whether to process patch/delta rpms
      if ( ZConfig::instance().download_use_deltarpm()
	&& ( url.schemeIsDownloading() || ZConfig::instance().download_use_deltarpm_always() ) )
      {
	std::list<DeltaRpm> deltaRpms;
	_deltas.deltaRpms( _package ).swap( deltaRpms );

	if ( ! deltaRpms.empty() && queryInstalled() && applydeltarpm::haveApplydeltarpm() )
	{
	  for_( it, deltaRpms.begin(), deltaRpms.end())
	  {
	    DBG << "tryDelta " << *it << endl;
	    ManagedFile ret( tryDelta( *it ) );
	    if ( ! ret->empty() )
	      return ret;
	  }
	}
      }

      // no patch/delta -> provide full package
      return Base::doProvidePackage();
    }

    ManagedFile RpmPackageProvider::tryDelta( const DeltaRpm & delta_r ) const
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
          policy.progressCB( bind( &RpmPackageProvider::progressDeltaDownload, this, _1 ) );
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
      Pathname destination( _package->repoInfo().packagesPath() / _package->repoInfo().path() / _package->location().filename() );

      if ( ! applydeltarpm::provide( delta, destination,
                                     bind( &RpmPackageProvider::progressDeltaApply, this, _1 ) ) )
        {
          report()->problemDeltaApply( _("applydeltarpm failed.") );
          return ManagedFile();
        }
      report()->finishDeltaApply();

      return ManagedFile( destination, filesystem::unlink );
    }

    ///////////////////////////////////////////////////////////////////
    //	class PackageProvider
    ///////////////////////////////////////////////////////////////////
    namespace factory
    {
      inline PackageProvider::Impl * make( RepoMediaAccess & access_r, const PoolItem & pi_r,
					   const DeltaCandidates & deltas_r,
					   const PackageProviderPolicy & policy_r )
      {
	if ( pi_r.isKind<Package>() )
	  return new RpmPackageProvider( access_r, pi_r->asKind<Package>(), deltas_r, policy_r );
	else if ( pi_r.isKind<SrcPackage>() )
	  return new PackageProviderImpl<SrcPackage>( access_r, pi_r->asKind<SrcPackage>(), policy_r );
	else
	  ZYPP_THROW( Exception( str::Str() << "Don't know how to cache non-package " << pi_r.asUserString() ) );
      }

      inline PackageProvider::Impl * make( RepoMediaAccess & access_r, const PoolItem & pi_r,
						  const PackageProviderPolicy & policy_r )
      {
	if ( pi_r.isKind<Package>() )
	  return new PackageProviderImpl<Package>( access_r, pi_r->asKind<Package>(), policy_r );
	else if ( pi_r.isKind<SrcPackage>() )
	  return new PackageProviderImpl<SrcPackage>( access_r, pi_r->asKind<SrcPackage>(), policy_r );
	else
	  ZYPP_THROW( Exception( str::Str() << "Don't know how to cache non-package " << pi_r.asUserString() ) );
      }

      inline PackageProvider::Impl * make( RepoMediaAccess & access_r, const Package::constPtr & package_r,
					   const DeltaCandidates & deltas_r,
					   const PackageProviderPolicy & policy_r )
      { return new RpmPackageProvider( access_r, package_r, deltas_r, policy_r ); }

    } // namespace factory
    ///////////////////////////////////////////////////////////////////

    PackageProvider::PackageProvider( RepoMediaAccess & access_r, const PoolItem & pi_r,
				      const DeltaCandidates & deltas_r, const PackageProviderPolicy & policy_r )

    : _pimpl( factory::make( access_r, pi_r, deltas_r, policy_r ) )
    {}

    PackageProvider::PackageProvider( RepoMediaAccess & access_r, const PoolItem & pi_r,
				      const PackageProviderPolicy & policy_r )
    : _pimpl( factory::make( access_r, pi_r, policy_r ) )
    {}

    /* legacy */
    PackageProvider::PackageProvider( RepoMediaAccess & access_r,
				      const Package::constPtr & package_r,
				      const DeltaCandidates & deltas_r,
				      const PackageProviderPolicy & policy_r )
    : _pimpl( factory::make( access_r, package_r, deltas_r, policy_r ) )
    {}

    PackageProvider::~PackageProvider()
    {}

    ManagedFile PackageProvider::providePackage() const
    { return _pimpl->providePackage(); }

    ManagedFile PackageProvider::providePackageFromCache() const
    { return _pimpl->providePackageFromCache(); }

    bool PackageProvider::isCached() const
    { return _pimpl->isCached(); }

  } // namespace repo
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
