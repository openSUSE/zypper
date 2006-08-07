/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/PackageProvider.cc
 *
*/
#include <iostream>
#include <sstream>
#include "zypp/base/Logger.h"

#include "zypp/source/PackageProvider.h"
#include "zypp/source/SourceProvideFile.h"
#include "zypp/source/Applydeltarpm.h"
#include "zypp/source/PackageDelta.h"
#include "zypp/detail/ImplConnect.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PackageProviderPolicy
    //
    ///////////////////////////////////////////////////////////////////

    bool PackageProviderPolicy::queryInstalled( const std::string & name_r, const Edition & ed_r ) const
    {
      if ( _queryInstalledCB )
        return _queryInstalledCB( name_r, ed_r );
      return false;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PackageProvider
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace
    { /////////////////////////////////////////////////////////////////

      inline std::string defRpmFileName( const Package::constPtr & package )
      {
        std::ostringstream ret;
        ret << package->name() << '-' << package->edition() << '.' << package->arch() << ".rpm";
        return ret.str();
      }

      /////////////////////////////////////////////////////////////////
    } // namespace source
    ///////////////////////////////////////////////////////////////////
    PackageProvider::PackageProvider( const Package::constPtr & package,
                                      const PackageProviderPolicy & policy_r )
    : _policy( policy_r )
    , _package( package )
    , _implPtr( detail::ImplConnect::resimpl( _package ) )
    {}

    PackageProvider::~PackageProvider()
    {}

    ManagedFile PackageProvider::providePackage() const
    {
      MIL << "provide Package " << _package << endl;
      ScopedGuard guardReport( newReport() );
      ManagedFile ret;
      _retry = false;
      do {
        report()->start( _package, _package->source().url() );
        try  // ELIMINATE try/catch by providing a log-guard
          {
            ret = doProvidePackage();
          }
        catch ( const Exception & excpt )
          {
            ERR << "Failed to provide Package " << _package << endl;
            ZYPP_RETHROW( excpt );
          }
      } while ( _retry );

      report()->finish( _package, source::DownloadResolvableReport::NO_ERROR, std::string() );
      MIL << "provided Package " << _package << " at " << ret << endl;
      return ret;
    }

    ManagedFile PackageProvider::doProvidePackage() const
    {
      // check whether to process patch/delta rpms
      if ( _package->source().remote() )
        {
          std::list<DeltaRpm> deltaRpms( _implPtr->deltaRpms() );
          std::list<PatchRpm> patchRpms( _implPtr->patchRpms() );

          if ( ! ( deltaRpms.empty() && patchRpms.empty() )
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

              if ( ! patchRpms.empty() )
                {
                  for( std::list<PatchRpm>::const_iterator it = patchRpms.begin();
                       it != patchRpms.end(); ++it )
                    {
                      DBG << "tryPatch " << *it << endl;
                      ManagedFile ret( tryPatch( *it ) );
                      if ( ! ret->empty() )
                        return ret;
                    }
                }
            }
        }

      // no patch/delta -> provide full package
      ManagedFile ret;
      source::OnMediaLocation loc;
      loc.medianr( _package->sourceMediaNr() )
      .filename( _package->location() )
      .checksum( _package->checksum() )
      .downloadsize( _package->archivesize() );

      source::ProvideFilePolicy policy;
      policy.progressCB( bind( &PackageProvider::progressPackageDownload, this, _1 ) );
      policy.failOnChecksumErrorCB( bind( &PackageProvider::failOnChecksumError, this ) );

      return source::provideFile( _package->source(), loc, policy );
    }

    ManagedFile PackageProvider::tryDelta( const DeltaRpm & delta_r ) const
    {
      if ( delta_r.baseversion().edition() != Edition::noedition
           && ! queryInstalled( delta_r.baseversion().edition() ) )
        return ManagedFile();

      if ( ! applydeltarpm::quickcheck( delta_r.baseversion().sequenceinfo() ) )
        return ManagedFile();

      report()->startDeltaDownload( delta_r.location().filename(),
                                    delta_r.location().downloadsize() );
      ManagedFile delta;
      try
        {
          source::ProvideFilePolicy policy;
          policy.progressCB( bind( &PackageProvider::progressDeltaDownload, this, _1 ) );
          delta = source::provideFile( _package->source(), delta_r.location(), policy );
        }
      catch ( const Exception & excpt )
        {
          report()->problemDeltaDownload( excpt.asUserString() );
          return ManagedFile();
        }

      report()->startDeltaApply( delta );
      if ( ! applydeltarpm::check( delta_r.baseversion().sequenceinfo() ) )
        {
          report()->problemDeltaApply( "applydeltarpm check failed." );
          return ManagedFile();
        }


      Pathname destination( Pathname::dirname( delta ) / defRpmFileName( _package ) );
      /* just to ease testing with non remote sources */
      if ( ! _package->source().remote() )
        destination = Pathname("/tmp") / defRpmFileName( _package );
      /**/

      if ( ! applydeltarpm::provide( delta, destination,
                                     bind( &PackageProvider::progressDeltaApply, this, _1 ) ) )
        {
          report()->problemDeltaApply( "applydeltarpm failed." );
          return ManagedFile();
        }

      return ManagedFile( destination, filesystem::unlink );
    }

    ManagedFile PackageProvider::tryPatch( const PatchRpm & patch_r ) const
    {
      // installed edition is in baseversions?
      const PatchRpm::BaseVersions & baseversions( patch_r.baseversions() );

      if ( std::find_if( baseversions.begin(), baseversions.end(),
                         bind( &PackageProvider::queryInstalled, this, _1 ) )
           == baseversions.end() )
        return ManagedFile();

      report()->startPatchDownload( patch_r.location().filename(),
                                    patch_r.location().downloadsize() );
      ManagedFile patch;
      try
        {
          source::ProvideFilePolicy policy;
          policy.progressCB( bind( &PackageProvider::progressPatchDownload, this, _1 ) );
          patch = source::provideFile( _package->source(), patch_r.location(), policy );
        }
      catch ( const Exception & excpt )
        {
          report()->problemPatchDownload( excpt.asUserString() );
          return ManagedFile();
        }

      return patch;
    }

    PackageProvider::ScopedGuard PackageProvider::newReport() const
    {
      _report.reset( new Report );
      return shared_ptr<void>( static_cast<void*>(0),
                               // custom deleter calling _report.reset()
                               // (cast required as reset is overloaded)
                               bind( mem_fun_ref( static_cast<void (shared_ptr<Report>::*)()>(&shared_ptr<Report>::reset) ),
                                     ref(_report) ) );
    }

    PackageProvider::Report & PackageProvider::report() const
    { return *_report; }

    bool PackageProvider::progressDeltaDownload( int value ) const
    { return report()->progressDeltaDownload( value ); }

    void PackageProvider::progressDeltaApply( int value ) const
    { return report()->progressDeltaApply( value ); }

    bool PackageProvider::progressPatchDownload( int value ) const
    { return report()->progressPatchDownload( value ); }

    bool PackageProvider::progressPackageDownload( int value ) const
    { return report()->progress( value, _package ); }

    bool PackageProvider::failOnChecksumError() const
    {
      std::string package_str = _package->name() + "-" + _package->edition().asString();
      switch ( report()->problem( _package, source::DownloadResolvableReport::INVALID,
                                   "Package "
                                   + package_str
                                   + " fails integrity check. Do you want to retry downloading it?" ) )
        {
        case source::DownloadResolvableReport::RETRY:
          _retry = true;
          break;
        default:
          break;
        }
      return true; // anyway a failure
    }

    bool PackageProvider::queryInstalled( const Edition & ed_r ) const
    { return _policy.queryInstalled( _package->name(), ed_r ); }


    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
