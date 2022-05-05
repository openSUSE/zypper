/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#include "smbprovider.h"
#include <zypp-media/ng/private/providedbg_p.h>
#include <zypp-media/Mount>
#include <zypp-media/ng/MediaVerifier>

#include <zypp-core/fs/TmpPath.h>

#include <iostream>
#include <fstream>

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "SmbProvider"

using namespace std::literals::string_view_literals;


/*!
 * Get the 1st path component (CIFS share name).
 */
inline std::string getShare( zypp::Pathname spath_r )
{
  if ( spath_r.empty() )
    return std::string();

  std::string share( spath_r.absolutename().asString() );
  std::string::size_type sep = share.find( "/", 1 );
  if ( sep == std::string::npos )
    share = share.erase( 0, 1 ); // nothing but the share name in spath_r
  else
    share = share.substr( 1, sep-1 );

  // deescape %2f in sharename
  while ( (sep = share.find( "%2f" )) != std::string::npos ) {
    share.replace( sep, 3, "/" );
  }

  return share;
}

/*!
 *  Strip off the 1st path component (CIFS share name).
 */
inline zypp::Pathname stripShare( zypp::Pathname spath_r )
{
  if ( spath_r.empty() )
    return zypp::Pathname();

  std::string striped( spath_r.absolutename().asString() );
  std::string::size_type sep = striped.find( "/", 1 );
  if ( sep == std::string::npos )
    return "/"; // nothing but the share name in spath_r

  return striped.substr( sep );
}


SmbProvider::SmbProvider( )
  : DeviceDriver( zyppng::worker::WorkerCaps::SimpleMount )
{ }

SmbProvider::~SmbProvider()
{ }

zyppng::worker::AttachResult SmbProvider::mountDevice ( const uint32_t id, const zypp::Url &attachUrl, const std::string &attachId, const std::string &label, const zyppng::HeaderValueMap &extras )
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
    std::string path = "//";
    path += attachUrl.getHost() + "/" + getShare( attachUrl.getPathName() );

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
            , e
          );
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
    zypp::media::Mount::Options options( attachUrl.getQueryParam("mountoptions") );

    int64_t lastTimestamp = -1;
    std::string username = attachUrl.getUsername();
    std::string password = attachUrl.getPassword();

    if ( ! options.has( "rw" ) ) {
      options["ro"];
    }

    // look for a workgroup
    std::string workgroup = attachUrl.getQueryParam("workgroup");
    if ( workgroup.empty() )
      workgroup = attachUrl.getQueryParam("domain");
    if ( !workgroup.empty() )
      options["domain"] = workgroup;

    // extract 'username', do not overwrite any _url.username

    zypp::media::Mount::Options::iterator toEnv;
    toEnv = options.find("username");
    if ( toEnv != options.end() ) {
      if ( username.empty() )
        username = toEnv->second;
      options.erase( toEnv );
    }

    toEnv = options.find("user"); // actually cifs specific
    if ( toEnv != options.end() ) {
      if ( username.empty() )
        username = toEnv->second;
      options.erase( toEnv );
    }

    // extract 'password', do not overwrite any _url.password

    toEnv = options.find("password");
    if ( toEnv != options.end() ) {
      if ( password.empty() )
        password = toEnv->second;
      options.erase( toEnv );
    }

    toEnv = options.find("pass"); // actually cifs specific
    if ( toEnv != options.end() ) {
      if ( password.empty() )
        password = toEnv->second;
      options.erase( toEnv );
    }

    bool canContinue = true;
    while ( canContinue ) {

      canContinue = false;

      // pass 'username' and 'password' via environment
      zypp::media::Mount::Environment environment;
      if ( !username.empty() )
        environment["USER"] = username;
      if ( !password.empty() )
        environment["PASSWD"] = password;

      // In case we need a tmpfile, credentials will remove
      // it in its destructor after the mount call below.
      zypp::filesystem::TmpPath credentials;
      if ( !username.empty() || !password.empty() )
      {
        zypp::filesystem::TmpFile tmp;
        std::ofstream outs( tmp.path().asString().c_str() );
        outs << "username=" <<  username << std::endl;
        outs << "password=" <<  password << std::endl;
        outs.close();

        credentials = tmp;
        options["credentials"] = credentials.path().asString();

      } else {
        // Use 'guest' option unless explicitly disabled (bnc #547354)
        if ( options.has( "noguest" ) )
          options.erase( "noguest" );
        else
          // prevent smbmount from asking for password
          // only add this option if 'credentials' is not used (bnc #560496)
          options["guest"];
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

        mount.mount( path, newAp.asString(), "cifs", options.asString(), environment );

        // wait for /etc/mtab update ...
        // (shouldn't be needed)
        int limit = 3;
        bool mountsucceeded = false;

        while( !(mountsucceeded=checkAttached( newAp, DeviceDriver::fstypePredicate( path, {"smb", "cifs"} ) )) && --limit) {
          MIL << "Mount did not appear yet, sleeping for 1s" << std::endl;
          sleep(1);
        }

        if ( !mountsucceeded ) {
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

      } catch (const zypp::media::MediaMountException & e) {
        ZYPP_CAUGHT( e );

        if ( e.mountError() == "Permission denied" ) {
          auto parent = parentWorker();
          if ( parent ) {
            auto res = parent->requireAuthorization( id, attachUrl, username, lastTimestamp );
            if ( res ) {
              canContinue = true;
              username = res->username;
              password = res->password;
              lastTimestamp = res->last_auth_timestamp;
            }
          }
          if ( !canContinue ) {
            removeAttachPoint(newAp);
            return zyppng::worker::AttachResult::error( zyppng::ProvideMessage::Code::Forbidden, "No authentication data", false );
          }
        } else {
          removeAttachPoint(newAp);
          return zyppng::worker::AttachResult::error( zyppng::ProvideMessage::Code::MountFailed, false, e );
        }
      } catch (const zypp::media::MediaException & e) {
        removeAttachPoint(newAp);
        ZYPP_CAUGHT(e);
        return zyppng::worker::AttachResult::error(
          zyppng::ProvideMessage::Code::MountFailed
          , false
          , e );
      }
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
      , e );;
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
