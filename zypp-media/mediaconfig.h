/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp-media/MediaConfig
 *
*/
#ifndef ZYPP_MEDIA_MEDIACONFIG_H
#define ZYPP_MEDIA_MEDIACONFIG_H

#include <zypp-core/base/NonCopyable.h>
#include <zypp-core/Pathname.h>
#include <zypp-core/zyppng/base/zyppglobal.h>
#include <memory>
#include <string>

namespace zypp {

  class MediaConfigPrivate;

  /*!
   * Helper class to collect global options and settings related to zypp-media.
   * Use it to avoid hardcoded values and calls to getZYpp() just
   * to retrieve some value like credentials path or download related settings.
   *
   * Note, if you add settings to this file, please follow the following
   * convention:
   *
   * namespace.settingname
   *
   * should become
   *
   * namespace_settingName()
   *
   * \ingroup ZyppConfig
   * \ingroup Singleton
  */
  class MediaConfig : private base::NonCopyable
  {
    ZYPP_DECLARE_PRIVATE(MediaConfig)
  public:

    /*! Singleton ctor */
    static MediaConfig & instance();

    bool setConfigValue ( const std::string &section, const std::string &entry, const std::string &value );

    /*!
     * Defaults to /etc/zypp/credentials.d
     */
    Pathname credentialsGlobalDir() const;

    /*!
     * Defaults to /etc/zypp/credentials.cat
     */
    Pathname credentialsGlobalFile() const;

    /*!
     * Maximum number of concurrent connections for a single transfer
     */
    long download_max_concurrent_connections() const;

    /*!
     * Minimum download speed (bytes per second)
     * until the connection is dropped
     */
    long download_min_download_speed() const;

    /*!
     * Maximum download speed (bytes per second)
     */
    long download_max_download_speed() const;

    /*!
     * Maximum silent tries
     */
    long download_max_silent_tries() const;

    /*!
     * Maximum time in seconds that you allow a transfer operation to take.
     */
    long download_transfer_timeout() const;

  private:
    MediaConfig();
    std::unique_ptr<MediaConfigPrivate> d_ptr;
  };

}

#endif
