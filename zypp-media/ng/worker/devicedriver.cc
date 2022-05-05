/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "devicedriver.h"
#include <zypp-media/ng/private/providedbg_p.h>
#include <zypp-media/ng/private/providemessage_p.h>
#include <zypp-media/ng/MediaVerifier>
#include <zypp-media/MediaException>
#include <zypp-core/fs/PathInfo.h>
#include <zypp-core/fs/TmpPath.h>
#include <zypp-core/Date.h>

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zyppng::worker::DeviceDriver"

namespace zyppng::worker
{

  DeviceDriver::DeviceDriver ( WorkerCaps::WorkerType wType )
    : _wType( wType )
  { }

  void DeviceDriver::setProvider ( ProvideWorkerWeakRef workerRef )
  {
    _parentWorker = workerRef;
  }

  zyppng::expected<WorkerCaps> DeviceDriver::initialize(const zyppng::worker::Configuration &conf)
  {
    const auto &values = conf.values();
    if ( const auto &i = values.find( std::string(zyppng::ATTACH_POINT) ); i != values.end() ) {
      const auto &val = i->second;
      MIL << "Got attachpoint from controller: " << val << std::endl;
      _attachRoot = zypp::Pathname(val).realpath();
    } else {
      return zyppng::expected<zyppng::worker::WorkerCaps>::error(ZYPP_EXCPT_PTR( zypp::Exception("Attach point required to work.") ));
    }

    _config = conf;

    zyppng::worker::WorkerCaps caps;
    caps.set_worker_type ( _wType );
    caps.set_cfg_flags(
      zyppng::worker::WorkerCaps::Flags (
        zyppng::worker::WorkerCaps::Pipeline
        | zyppng::worker::WorkerCaps::ZyppLogFormat
        | zyppng::worker::WorkerCaps::SingleInstance
        )
      );

    return zyppng::expected<zyppng::worker::WorkerCaps>::success(caps);
  }

  bool DeviceDriver::detachMedia ( const std::string &attachId )
  {
    auto i = _attachedMedia.find( attachId );
    if ( i == _attachedMedia.end() )
      return false;

    _attachedMedia.erase(i);
    return true;
  }

  void DeviceDriver::releaseIdleDevices ()
  {
    for ( auto i = _sysDevs.begin (); i != _sysDevs.end(); ) {
      if ( i->use_count() == 1 && !(*i)->_mountPoint.empty() ) {
        MIL << "Unmounting device " << (*i)->_name << " since its not used anymore" << std::endl;
        unmountDevice(*(*i));
        if ( (*i)->_ephemeral ) {
          i = _sysDevs.erase(i);
          continue;
        }
      }
      ++i;
    }
  }

  void DeviceDriver::detectDevices()
  {
    return;
  }

  std::vector<std::shared_ptr<Device>> &DeviceDriver::knownDevices()
  {
    return _sysDevs;
  }

  const std::vector<std::shared_ptr<Device>> &DeviceDriver::knownDevices() const
  {
    return _sysDevs;
  }

  std::unordered_map<std::string, AttachedMedia> &DeviceDriver::attachedMedia()
  {
    return _attachedMedia;
  }

  void DeviceDriver::immediateShutdown()
  {
    // here we need to unmount everything
    for ( auto i = _sysDevs.begin (); i != _sysDevs.end(); ) {
      unmountDevice(*(*i));
      if ( (*i)->_ephemeral ) {
          i = _sysDevs.erase(i);
          continue;
      }
      ++i;
    }
    _attachedMedia.clear();
  }

  ProvideWorkerRef DeviceDriver::parentWorker () const
  {
    return _parentWorker.lock();
  }

  void DeviceDriver::unmountDevice ( Device &dev )
  {
    if ( dev._mountPoint.empty () )
      return;
    try {
      zypp::media::Mount mount;
      mount.umount( dev._mountPoint.asString() );
      removeAttachPoint( dev._mountPoint );
    } catch (const zypp::media::MediaException & excpt_r) {
      ERR << "Failed to unmount device: " << dev._name << std::endl;
      ZYPP_CAUGHT(excpt_r);
    }
    dev._mountPoint = zypp::Pathname();
  }

  bool DeviceDriver::isVolatile () const
  {
    return false;
  }

  void DeviceDriver::setAttachRoot ( const zypp::Pathname &root )
  {
    _attachRoot = root;
  }

  zypp::Pathname DeviceDriver::attachRoot () const
  {
    if ( _attachRoot.empty() ) {
      MIL << "Attach root is empty" << std::endl;
      return zypp::Pathname(".").realpath();
    }
    return _attachRoot;
  }

  const zyppng::worker::Configuration &DeviceDriver::config() const
  {
    return _config;
  }

  zyppng::expected<void> DeviceDriver::isDesiredMedium ( const zypp::Url &deviceUrl, const zypp::Pathname &mountPoint, const zyppng::MediaDataVerifierRef &verifier, uint mediaNr )
  {
    if ( !verifier ) {
      // at least the requested path must exist on the medium
      zypp::PathInfo p( mountPoint );
      if ( p.isExist() && p.isDir() )
        return zyppng::expected<void>::success();	// we have no valid data
      return zyppng::expected<void>::error( ZYPP_EXCPT_PTR( zypp::media::MediaNotDesiredException( deviceUrl ) ) );
    }

    auto devVerifier = verifier->clone();
    if ( !devVerifier ) {
      // unlikely to happen
      return zyppng::expected<void>::error( ZYPP_EXCPT_PTR( zypp::Exception("Failed to clone verifier") ) );
    }

    // bsc#1180851: If there is just one not-volatile medium in the set
    // tolerate a missing (vanished) media identifier and let the URL rule.
    bool relaxed = verifier->totalMedia() == 1 && !isVolatile();

    const auto &relMediaPath = devVerifier->mediaFilePath( mediaNr );
    zypp::Pathname mediaFile { mountPoint / relMediaPath };
    zypp::PathInfo pi( mediaFile );
    if ( !pi.isExist() ) {
      if ( relaxed )
        return zyppng::expected<void>::success();
      auto excpt =  zypp::media::MediaFileNotFoundException( deviceUrl, relMediaPath ) ;
      excpt.addHistory( verifier->expectedAsUserString( mediaNr ) );
      return zyppng::expected<void>::error( ZYPP_EXCPT_PTR( std::move(excpt) ) );
    }
    if ( !pi.isFile() ) {
      if ( relaxed )
        return zyppng::expected<void>::success();
      auto excpt =  zypp::media::MediaNotAFileException( deviceUrl, relMediaPath ) ;
      excpt.addHistory( verifier->expectedAsUserString( mediaNr ) );
      return zyppng::expected<void>::error( ZYPP_EXCPT_PTR( std::move(excpt) ) );
    }

    if ( !devVerifier->load( mediaFile ) ) {
      return zyppng::expected<void>::error( ZYPP_EXCPT_PTR( zypp::Exception("Failed to load media information from medium") ) );
    }
    if ( !verifier->matches( devVerifier ) ) {
      return zyppng::expected<void>::error( ZYPP_EXCPT_PTR( zypp::media::MediaNotDesiredException( deviceUrl ) ) );
    }
    return zyppng::expected<void>::success();
  }

  zypp::Pathname DeviceDriver::createAttachPoint( const zypp::Pathname &attach_root ) const
  {
    zypp::Pathname apoint;

    if( attach_root.empty() || !attach_root.absolute()) {
      ERR << "Create attach point: invalid attach root: '"
          << attach_root << "'" << std::endl;
      return apoint;
    }

    zypp::PathInfo adir( attach_root );
    if( !adir.isDir() || (geteuid() != 0 && !adir.userMayRWX())) {
      DBG << "Create attach point: attach root is not a writable directory: '"
          << attach_root << "'" << std::endl;
      return apoint;
    }

    static bool cleanup_once( true );
    if ( cleanup_once )
    {
      cleanup_once = false;
      DBG << "Look for orphaned attach points in " << adir << std::endl;
      std::list<std::string> entries;
      zypp::filesystem::readdir( entries, attach_root, false );
      for ( const std::string & entry : entries )
      {
        if ( ! zypp::str::hasPrefix( entry, "AP_0x" ) )
          continue;
        zypp::PathInfo sdir( attach_root + entry );
        if ( sdir.isDir()
             && sdir.dev() == adir.dev()
             && ( zypp::Date::now()-sdir.mtime() > zypp::Date::month ) )
        {
          DBG << "Remove orphaned attach point " << sdir << std::endl;
          zypp::filesystem::recursive_rmdir( sdir.path() );
        }
      }
    }

    zypp::filesystem::TmpDir tmpdir( attach_root, "AP_0x" );
    if ( tmpdir )
    {
      apoint = tmpdir.path().asString();
      if ( ! apoint.empty() )
      {
        tmpdir.autoCleanup( false );	// Take responsibility for cleanup.
      }
      else
      {
        ERR << "Unable to resolve real path for attach point " << tmpdir << std::endl;
      }
    }
    else
    {
      ERR << "Unable to create attach point below " << attach_root << std::endl;
    }
    return apoint;
  }

  void DeviceDriver::removeAttachPoint( const zypp::filesystem::Pathname &attachRoot ) const
  {
    if( !attachRoot.empty() &&
         zypp::PathInfo(attachRoot).isDir() &&
         attachRoot != "/" ) {
      int res = recursive_rmdir( attachRoot );
      if ( res == 0 ) {
        MIL << "Deleted default attach point " << attachRoot << std::endl;
      } else {
        ERR << "Failed to Delete default attach point " << attachRoot
          << " errno(" << res << ")" << std::endl;
      }
    }
  }

  bool DeviceDriver::checkAttached ( const zypp::filesystem::Pathname &mountPoint, const std::function<bool (const zypp::media::MountEntry &)> predicate )
  {
    bool isAttached = false;
    time_t old_mtime = _attach_mtime;
    _attach_mtime = zypp::media::Mount::getMTime();
    if( !(old_mtime <= 0 || _attach_mtime != old_mtime) ) {
      // OK, skip the check (we've seen it at least once)
      isAttached = true;
    } else {
      if( old_mtime > 0)
        DBG << "Mount table changed - rereading it" << std::endl;
      else
        DBG << "Forced check of the mount table" << std::endl;

      for( const auto &entry : zypp::media::Mount::getEntries() ) {

        if ( mountPoint != zypp::Pathname(entry.dir) )
          continue;	// at least the mount points must match
        if ( predicate(entry) ) {
          isAttached = true;
          break;
        }
      }
    }

    // force recheck
    if ( !isAttached )
      _attach_mtime = 0;

    return isAttached;
  }

  const std::function<bool (const zypp::media::MountEntry &)> DeviceDriver::devicePredicate( unsigned int majNr, unsigned int minNr )
  {
    return [ majNr, minNr ]( const zypp::media::MountEntry &entry ) -> bool {
      if( entry.isBlockDevice() ) {
        zypp::PathInfo dev_info( entry.src );
        if ( dev_info.devMajor () == majNr && dev_info.devMinor () == minNr ) {
          DBG << "Found device "
              << majNr << ":" << minNr
              << " in the mount table as " << entry.src << std::endl;
          return true;
        }
      }
      return false;
    };
  }

  const std::function<bool (const zypp::media::MountEntry &)> DeviceDriver::fstypePredicate( const std::string &src, const std::vector<std::string> &fstypes )
  {
    return [ srcdev=src, fst=fstypes ]( const zypp::media::MountEntry &entry ) -> bool {
      if( !entry.isBlockDevice() ) {
        if ( std::find( fst.begin(), fst.end(), entry.type ) != fst.end() ) {
          if ( srcdev == entry.src ) {
            DBG << "Found media mount"
                << " in the mount table as " << entry.src << std::endl;
            return true;
          }
        }
      }
      return false;
    };
  }

  const std::function<bool (const zypp::media::MountEntry &)> DeviceDriver::bindMountPredicate( const std::string &src )
  {
    return [ srcdev=src ]( const zypp::media::MountEntry &entry ) -> bool {
      if( !entry.isBlockDevice() ) {
        if ( srcdev == entry.src ) {
          DBG << "Found bound media "
              << " in the mount table as " << entry.src << std::endl;
          return true;
        }
      }
      return false;
    };
  }

  AttachError::AttachError ( const uint code, const std::string &reason, const bool transient, const HeaderValueMap &extra)
    : _code( code ),
      _reason( reason ),
      _transient( transient ),
      _extra( extra )
  {

  }

  AttachError::AttachError ( const uint code, const bool transient, const zypp::Exception &e )
    : _code( code ),
      _reason( e.asUserString() ),
      _transient( transient )
  {
    if ( !e.historyEmpty() ) {
      _extra = { { std::string(zyppng::ErrMsgFields::History),  { zyppng::HeaderValueMap::Value(e.historyAsString()) }} };
    }
  }


}
