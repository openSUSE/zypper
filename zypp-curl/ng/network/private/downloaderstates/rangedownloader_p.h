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
#ifndef ZYPP_CURL_NG_NETWORK_PRIVATE_DOWNLOADERSTATES_RANGEDOWNLOADER_P_H_INCLUDED
#define ZYPP_CURL_NG_NETWORK_PRIVATE_DOWNLOADERSTATES_RANGEDOWNLOADER_P_H_INCLUDED

#include "base_p.h"
#include "mirrorhandling_p.h"
#include <zypp-core/zyppng/base/statemachine.h>

namespace zyppng {

  /*!
   * Generic state implementation for all states that download a file in blocks from
   * a fixed set of mirrors.
   */
  struct RangeDownloaderBaseState : public MirrorHandlingStateBase {

    using Request = DownloadPrivateBase::Request;
    using Block   = DownloadPrivateBase::Block;

    RangeDownloaderBaseState ( std::vector<Url> &&mirrors, DownloadPrivate &parent ) :
      MirrorHandlingStateBase(parent) {
      _fileMirrors = std::move(mirrors);
    }

    void ensureDownloadsRunning ();
    void reschedule ();

    const NetworkRequestError &error () const {
      return _error;
    }

    void setFailed  ( NetworkRequestError &&err );
    void setFailed  ( std::string && reason );
    virtual void setFinished ( );
    void cancelAll  ( const NetworkRequestError &err  );

    void onRequestStarted  ( NetworkRequest & );
    void onRequestProgress ( NetworkRequest &, off_t, off_t, off_t, off_t );
    void onRequestFinished ( NetworkRequest &req , const NetworkRequestError &err );

    // MirrorHandlingStateBase interface
    void mirrorReceived(MirrorControl::MirrorPick mirror) override;
    void failedToPrepare() override;

    static zypp::ByteCount makeBlksize ( size_t filesize );

  protected:
    NetworkRequestError _error;
    bool _inEnsureDownloadsRunning = false; //< Flag to prevent multiple entry to ensureDownloadsRunning

    size_t             _fileSize = 0; //< The expected filesize, this is used to make sure we do not write after the end offset of the expected file size
    zypp::ByteCount    _preferredChunkSize = 0; //< The preferred chunk size we want to download per request
    std::list<Block>   _ranges;

    //keep a list with failed blocks in case we run out of mirrors,
    //in that case we can retry to download them once we have a finished download
    std::list<Block>   _failedRanges;

    off_t _downloadedMultiByteCount = 0; //< the number of bytes that were already fetched in RunningMulti state

    std::vector< std::shared_ptr<Request> > _runningRequests;

    // we only define the signals here and add the accessor functions in the subclasses, static casting of
    // the class type is not allowed at compile time, so they would not be useable in the transition table otherwise
    Signal< void () > _sigFinished;
    Signal< void () > _sigFailed;

  private:
    void handleRequestError( std::shared_ptr<Request> req, const zyppng::NetworkRequestError &err );
    bool addBlockRanges( std::shared_ptr<Request> req, std::vector<Block> &&blocks ) const;
    void addNewRequest     (std::shared_ptr<Request> req, const bool connectSignals = true );
    bool assertExpectedFilesize ( off_t currentFilesize );

    std::vector<Block> getNextBlocks ( const std::string &urlScheme );
    std::vector<Block> getNextFailedBlocks( const std::string &urlScheme );
  };


}

#endif
