/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp-media/mediaconfig.cc
 *
*/

#include "mediaconfig.h"
#include <zypp-core/Pathname.h>
#include <zypp-core/base/String.h>

#define  CONNECT_TIMEOUT        60

namespace zypp {

  class MediaConfigPrivate {
  public:

    MediaConfigPrivate()
      : download_max_concurrent_connections( 5 )
      , download_min_download_speed	( 0 )
      , download_max_download_speed	( 0 )
      , download_max_silent_tries	( 5 )
      , download_transfer_timeout	( 180 )
      , download_connect_timeout        ( CONNECT_TIMEOUT )
    { }

    Pathname credentials_global_dir_path;
    Pathname credentials_global_file_path;

    int download_max_concurrent_connections;
    int download_min_download_speed;
    int download_max_download_speed;
    int download_max_silent_tries;
    int download_transfer_timeout;
    int download_connect_timeout;

  };

  MediaConfig::MediaConfig() : d_ptr( new MediaConfigPrivate() )
  { }

  MediaConfig &MediaConfig::instance()
  {
    static MediaConfig instance;
    return instance;
  }

  bool MediaConfig::setConfigValue( const std::string &section, const std::string &entry, const std::string &value )
  {
    Z_D();
    if ( section == "main" ) {
      if ( entry == "credentials.global.dir" ) {
        d->credentials_global_dir_path = Pathname(value);
        return true;
      } else if ( entry == "credentials.global.file" ) {
        d->credentials_global_file_path = Pathname(value);
        return true;

      } else if ( entry == "download.max_concurrent_connections" ) {
        str::strtonum(value, d->download_max_concurrent_connections);
        return true;

      } else if ( entry == "download.min_download_speed" ) {
        str::strtonum(value, d->download_min_download_speed);
        return true;

      } else if ( entry == "download.max_download_speed" ) {
        str::strtonum(value, d->download_max_download_speed);
        return true;

      } else if ( entry == "download.max_silent_tries" ) {
        str::strtonum(value, d->download_max_silent_tries);
        return true;

      } else if ( entry == "download.transfer_timeout" ) {
        str::strtonum(value, d->download_transfer_timeout);
        if ( d->download_transfer_timeout < 0 )		d->download_transfer_timeout = 0;
        else if ( d->download_transfer_timeout > 3600 )	d->download_transfer_timeout = 3600;
        return true;
      }
    }
    return false;
  }

  Pathname MediaConfig::credentialsGlobalDir() const
  {
    Z_D();
    return ( d->credentials_global_dir_path.empty() ?
               Pathname("/etc/zypp/credentials.d") : d->credentials_global_dir_path );
  }

  Pathname MediaConfig::credentialsGlobalFile() const
  {
    Z_D();
    return ( d->credentials_global_file_path.empty() ?
               Pathname("/etc/zypp/credentials.cat") : d->credentials_global_file_path );
  }

  long MediaConfig::download_max_concurrent_connections() const
  { return d_func()->download_max_concurrent_connections; }

  long MediaConfig::download_min_download_speed() const
  { return d_func()->download_min_download_speed; }

  long MediaConfig::download_max_download_speed() const
  { return d_func()->download_max_download_speed; }

  long MediaConfig::download_max_silent_tries() const
  { return d_func()->download_max_silent_tries; }

  long MediaConfig::download_transfer_timeout() const
  { return d_func()->download_transfer_timeout; }

  long MediaConfig::download_connect_timeout() const
  { return d_func()->download_connect_timeout; }

  ZYPP_IMPL_PRIVATE(MediaConfig)
}


