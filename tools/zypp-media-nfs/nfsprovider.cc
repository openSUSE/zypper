/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#include "nfsprovider.h"
#include <zypp-media/ng/private/providedbg_p.h>
#include <zypp-media/Mount>
#include <zypp-media/ng/MediaVerifier>

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "NfsProvider"

using namespace std::literals::string_view_literals;

/**
 * Value of NFS mount minor timeout (passed to <code>timeo</code> option
 * of the NFS mount) in tenths of a second.
 *
 * The value of 300 should give a major timeout after 3.5 minutes
 * for UDP and 1.5 minutes for TCP. (#350309)
 */
constexpr auto NFS_MOUNT_TIMEOUT = 300;
constexpr std::array<std::string_view, 2> NFS_MOUNT_FSTYPES = { "nfs"sv, "nfs4"sv };

NfsProvider::NfsProvider( )
  : DeviceDriver( zyppng::worker::WorkerCaps::SimpleMount )
{ }

NfsProvider::~NfsProvider()
{ }

zyppng::worker::AttachResult NfsProvider::mountDevice ( const uint32_t id, const zypp::Url &attachUrl, const std::string &attachId, const std::string &label, const zyppng::HeaderValueMap &extras )
{
  try
  {
    if ( attachUrl.getHost().empty() ) {
      return zyppng::worker::AttachResult::error(
        zyppng::ProvideMessage::Code::MountFailed
        , "Host can not be empty"
        , false
      );
    }

    // set up the verifier
    zyppng::MediaDataVerifierRef verifier;
    zyppng::MediaDataVerifierRef devVerifier;
    if ( extras.value( zyppng::AttachMsgFields::VerifyType ).valid() ) {
      verifier = zyppng::MediaDataVerifier::createVerifier( extras.value(zyppng::AttachMsgFields::VerifyType).asString() );
      if ( !verifier ) {
        return zyppng::worker::AttachResult::error(
          zyppng::ProvideMessage::Code::MountFailed
          , "Invalid verifier type"
          , false
        );
      }

      if ( !verifier->load( extras.value(zyppng::AttachMsgFields::VerifyData).asString() ) ) {
        return zyppng::worker::AttachResult::error(
          zyppng::ProvideMessage::Code::MountFailed
          , "Failed to create verifier from file"
          , false
        );
      }
    }
    const auto &devs = knownDevices();

    // build a device name the same way the old media backend did
    std::string path = attachUrl.getHost();
    path += ':';
    path += zypp::Pathname(attachUrl.getPathName()).asString();

    // first check if we have that device already
    auto i = std::find_if( devs.begin(), devs.end(), [&]( const auto &d ) { return d->_name == path; } );
    if ( i != devs.end() ) {
      auto res = isDesiredMedium( attachUrl, (*i)->_mountPoint, verifier, extras.value( zyppng::AttachMsgFields::MediaNr, 1 ).asInt() );
      if ( !res ) {
        try {
          std::rethrow_exception( res.error() );
        } catch( const zypp::Exception& e ) {
          return zyppng::worker::AttachResult::error(
            zyppng::ProvideMessage::Code::MediumNotDesired
            , false
            , e );
        } catch ( ... ) {
          return zyppng::worker::AttachResult::error(
            zyppng::ProvideMessage::Code::MediumNotDesired
            , "Checking the medium failed with an uknown error"
            , false
            );
        }
      } else {
        attachedMedia().insert( std::make_pair( attachId, zyppng::worker::AttachedMedia{ *i, "/" } ) );
        return zyppng::worker::AttachResult::success();
      }
    }

    // we did not find a existing mount, well lets make a new one
    // start with some legacy code:
    std::string filesystem( attachUrl.getScheme() );
    if ( filesystem != "nfs4" && attachUrl.getQueryParam("type") == "nfs4" ) {
      filesystem = "nfs4";
    }

    std::string options = attachUrl.getQueryParam("mountoptions");
    if(options.empty()) {
      options="ro";
    }

    std::vector<std::string> optionList;
    zypp::str::split( options, std::back_inserter(optionList), "," );
    std::vector<std::string>::const_iterator it;
    bool contains_lock  = false, contains_soft = false,
          contains_timeo = false, contains_hard = false;

    for( it = optionList.begin(); it != optionList.end(); ++it ) {
      if ( *it == "lock" || *it == "nolock" ) contains_lock = true;
      else if ( *it == "soft") contains_soft = true;
      else if ( *it == "hard") contains_hard = true;
      else if ( it->find("timeo") != std::string::npos ) contains_timeo = true;
    }

    if ( !(contains_lock && contains_soft) ) {
      // Add option "nolock", unless option "lock" or "unlock" is already set.
      // This should prevent the mount command hanging when the portmapper isn't
      // running.
      if ( !contains_lock ) {
        optionList.push_back( "nolock" );
      }
      // Add options "soft,timeo=NFS_MOUNT_TIMEOUT", unless they are set
      // already or "hard" option is explicitly specified. This prevent
      // the mount command from hanging when the nfs server is not responding
      // and file transactions from an unresponsive to throw an error after
      // a short time instead of hanging forever
      if ( !(contains_soft || contains_hard) ) {
        optionList.push_back( "soft" );
        if ( !contains_timeo ) {
          std::ostringstream s;
          s << "timeo=" << NFS_MOUNT_TIMEOUT;
          optionList.push_back( s.str() );
        }
      }
      options = zypp::str::join( optionList, "," );
    }

    zypp::Pathname newAp;
    zypp::media::Mount mount;
    try
    {
      newAp = createAttachPoint( attachRoot() );
      if ( newAp.empty() ) {
        return zyppng::worker::AttachResult::error(
          zyppng::ProvideMessage::Code::MountFailed
          , "Failed to create mount directory."
          , false
        );
      }

      mount.mount( path, newAp.asString(), filesystem, options );

      // wait for /etc/mtab update ...
      // (shouldn't be needed)
      int limit = 3;
      bool mountsucceeded = false;

      // we only check if our attachpoint is in mounts and of type nfs/nfs4, everything else is not reliable anyways
      constexpr auto checkFsTypePredicate = []( const zypp::media::MountEntry &entry ){
        return ( std::find( NFS_MOUNT_FSTYPES.begin(), NFS_MOUNT_FSTYPES.end(), entry.type ) != NFS_MOUNT_FSTYPES.end() );
      };
      while( !(mountsucceeded=checkAttached( newAp, checkFsTypePredicate )) && --limit) {
        MIL << "Mount did not appear yet, sleeping for 1s" << std::endl;
        sleep(1);
      }

      if( !mountsucceeded) {
        try {
          mount.umount( newAp.asString() );
        } catch (const zypp::media::MediaException & excpt_r) {
          ZYPP_CAUGHT(excpt_r);
        }
        ZYPP_THROW( zypp::media::MediaMountException(
          "Unable to verify that the media was mounted",
          path, newAp.asString()
        ));
      }

      // if we reach this place, mount worked -> YAY, lets see if that is the desired medium!
      const auto &isDesired = isDesiredMedium( attachUrl, newAp, verifier, extras.value( zyppng::AttachMsgFields::MediaNr, 1 ).asInt() );
      if ( !isDesired ) {
        try {
          mount.umount( newAp.asString() );
        } catch (const zypp::media::MediaException & excpt_r) {
          ZYPP_CAUGHT(excpt_r);
        }
        ZYPP_THROW( zypp::media::MediaNotDesiredException( attachUrl ) );
      }

      MIL << "New device " << path << " mounted on " << newAp << std::endl;
      auto newDev = std::shared_ptr<zyppng::worker::Device>( new zyppng::worker::Device{
        ._name = path,
        ._maj_nr = 0,
        ._min_nr = 0,
        ._mountPoint = newAp,
        ._ephemeral = true, // device should be removed after the last attachment was released
        ._properties = {}
      });
      knownDevices().push_back( newDev );
      attachedMedia().insert( std::make_pair( attachId, zyppng::worker::AttachedMedia{ newDev, "/" } ) );
      return zyppng::worker::AttachResult::success();

    }
    catch (const zypp::media::MediaMountException &e )
    {
      removeAttachPoint(newAp);
      ZYPP_CAUGHT(e);
      return zyppng::worker::AttachResult::error(
        zyppng::ProvideMessage::Code::MountFailed
        , false
        , e
      );
    }
    catch (const zypp::media::MediaException & e)
    {
      removeAttachPoint(newAp);
      ZYPP_CAUGHT(e);
      return zyppng::worker::AttachResult::error(
        zyppng::ProvideMessage::Code::MountFailed
        , false
        , e
      );
    }

    // We should never reach this place, but if we do fail the attach request
    return zyppng::worker::AttachResult::error(
      zyppng::ProvideMessage::Code::MountFailed
      , zypp::str::Str()<<"Mounting the medium " << attachUrl << " failed for an unknown reason"
      , false
    );

  }  catch ( const zypp::Exception &e  ) {
    return zyppng::worker::AttachResult::error(
      zyppng::ProvideMessage::Code::BadRequest
      , false
      , e
    );
  }  catch ( const std::exception &e  ) {
    return zyppng::worker::AttachResult::error(
      zyppng::ProvideMessage::Code::BadRequest
      , e.what()
      , false
    );
  }  catch ( ... ) {
    return zyppng::worker::AttachResult::error(
      zyppng::ProvideMessage::Code::BadRequest
      , "Unknown exception"
      , false
    );
  }
}
