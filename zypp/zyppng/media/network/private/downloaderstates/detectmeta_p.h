/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* You have been warned!
*
*/
#ifndef ZYPP_NG_MEDIA_HTTP_PRIVATE_DOWNLOADERSTATES_DETECTMETA_P_H_INCLUDED
#define ZYPP_NG_MEDIA_HTTP_PRIVATE_DOWNLOADERSTATES_DETECTMETA_P_H_INCLUDED

#include "base_p.h"
#include <zypp-core/zyppng/base/statemachine.h>

namespace zyppng {

#if ENABLE_ZCHUNK_COMPRESSION
  struct DLZckHeadState;
#endif

  /*!
   * State implementation for the metalink detection phase,
   * this state issues a HEAD request while setting the magic
   * "Accept: *\/*, application/metalink+xml, application/metalink4+xml"
   * header in the request to figure out if a metalink file is available or not.
   *
   * In order to use metalink support the server
   * needs to correctly return the metalink file content type,
   * otherwise we proceed to not downloading a metalink file
   */
  struct DetectMetalinkState : public zyppng::SimpleState< DownloadPrivate, Download::DetectMetaLink, false > {

    using Request = DownloadPrivateBase::Request;

    DetectMetalinkState ( DownloadPrivate &parent );

    void enter ();
    void exit ();

    void onRequestStarted  ( NetworkRequest & );
    void onRequestProgress ( NetworkRequest &, off_t, off_t dlnow, off_t, off_t );
    void onRequestFinished ( NetworkRequest &req , const NetworkRequestError &err );


    const NetworkRequestError &error () const {
      return _error;
    }

    SignalProxy< void () > sigFinished() {
      return _sigFinished;
    }

    bool toMetalinkGuard () const {
      return _gotMetalink;
    }

    bool toSimpleDownloadGuard () const;

#if ENABLE_ZCHUNK_COMPRESSION
    bool toZckHeadDownloadGuard () const;
    std::shared_ptr<DLZckHeadState> toDLZckHeadState();
#endif

    std::shared_ptr<Request> _request;

  private:
    NetworkRequestError _error;
    bool _gotMetalink = false;
    Signal< void () > _sigFinished;
  };

}

#endif
