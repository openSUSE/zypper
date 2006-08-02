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

    PackageProvider::PackageProvider( const Package::constPtr & package )
    : _package( package )
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

    bool PackageProvider::considerDeltas() const
    {
#warning ADD DOWNLOADING MEDIA CONDITION
      return applydeltarpm::haveApplydeltarpm();
    }

    bool PackageProvider::considerPatches() const
    {
#warning ADD DOWNLOADING MEDIA CONDITION
      return _installedEdition != Edition::noedition;
    }

    ManagedFile PackageProvider::doProvidePackage() const
    {
      if ( considerDeltas() )
        {
          std::list<DeltaRpm> deltaRpms( _implPtr->deltaRpms() );
          if ( ! deltaRpms.empty() )
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

      if ( considerPatches() )
        {
          std::list<PatchRpm> patchRpms( _implPtr->patchRpms() );
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

#warning FIX FIX PATHNAME
      Pathname destination( "/tmp/delta.rpm" );
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
      if ( std::find( baseversions.begin(), baseversions.end(),
                      _installedEdition ) == baseversions.end() )
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

    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
