/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/ZYppCallbacks.h
 *
*/
#ifndef ZYPP_ZYPPCALLBACKS_H
#define ZYPP_ZYPPCALLBACKS_H

#include "zypp/Callback.h"
#include "zypp/Resolvable.h"
#include "zypp/Source.h"
#include "zypp/Pathname.h"
#include "zypp/Message.h"
#include "zypp/Url.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  namespace source
  {
    // progress for downloading a resolvable
    struct DownloadResolvableReport : public callback::ReportBase
    {
      enum Action {
        ABORT,  // abort and return error
        RETRY,	// retry
        IGNORE, // ignore this resolvable but continue
      };

      enum Error {
	NO_ERROR,
        NOT_FOUND, 	// the requested Url was not found
	IO,		// IO error
	INVALID		// the downloaded file is invalid
      };

      virtual void start(
        Resolvable::constPtr resolvable_ptr
	, Url url
      ) {}


      // Dowmload delta rpm:
      // - path below url reported on start()
      // - expected download size (0 if unknown)
      // - download is interruptable
      // - problems are just informal
      virtual void startDeltaDownload( const Pathname & filename, const ByteCount & downloadsize )
      {}

      virtual bool progressDeltaDownload( int value )
      { return true; }

      virtual void problemDeltaDownload( std::string description )
      {}

      virtual void finishDeltaDownload()
      {}

      // Apply delta rpm:
      // - local path of downloaded delta
      // - aplpy is not interruptable
      // - problems are just informal
      virtual void startDeltaApply( const Pathname & filename )
      {}

      virtual void progressDeltaApply( int value )
      {}

      virtual void problemDeltaApply( std::string description )
      {}

      virtual void finishDeltaApply()
      {}

      // Dowmload patch rpm:
      // - path below url reported on start()
      // - expected download size (0 if unknown)
      // - download is interruptable
      virtual void startPatchDownload( const Pathname & filename, const ByteCount & downloadsize )
      {}

      virtual bool progressPatchDownload( int value )
      { return true; }

      virtual void problemPatchDownload( std::string description )
      {}

      virtual void finishPatchDownload()
      {}


      // return false if the download should be aborted right now
      virtual bool progress(int value, Resolvable::constPtr resolvable_ptr)
      { return true; }

      virtual Action problem(
        Resolvable::constPtr resolvable_ptr
	, Error error
	, std::string description
      ) { return ABORT; }

      virtual void finish(Resolvable::constPtr resolvable_ptr
        , Error error
	, std::string reason
      ) {}
    };


    // progress for downloading a specific file
    struct DownloadFileReport : public callback::ReportBase
    {
      enum Action {
        ABORT,  // abort and return error
        RETRY	// retry
      };

      enum Error {
	NO_ERROR,
        NOT_FOUND, 	// the requested Url was not found
	IO,		// IO error
	INVALID		// the downloaded file is invalid
      };
      virtual void start(
	Source_Ref source
	, Url url
      ) {}

      virtual bool progress(int value, Url url)
      { return true; }

      virtual Action problem(
        Url url
	, Error error
	, std::string description
      ) { return ABORT; }

      virtual void finish(
        Url url
        , Error error
	, std::string reason
      ) {}
    };

    // progress for refreshing a source data
    struct RefreshSourceReport : public callback::ReportBase
    {
      enum Action {
        ABORT,  // abort and return error
        RETRY,	// retry
	IGNORE  // skip refresh, ignore failed refresh
      };

      enum Error {
	NO_ERROR,
        NOT_FOUND, 	// the requested Url was not found
	IO,		// IO error
	INVALID		// th source is invalid
      };
      virtual void start(
	Source_Ref source
	, Url url
      ) {}

      virtual bool progress(int value, Source_Ref source)
      { return true; }

      virtual Action problem(
        Source_Ref source
	, Error error
	, std::string description
      ) { return ABORT; }

      virtual void finish(
        Source_Ref source
        , Error error
	, std::string reason
      ) {}
    };

    // DEPRECATED
    struct CreateSourceReport : public callback::ReportBase
    {
      enum Action {
        ABORT,  // abort and return error
        RETRY	// retry
      };

      enum Error {
        NO_ERROR,
        NOT_FOUND, 	// the requested Url was not found
        IO,		// IO error
        INVALID		// th source is invalid
      };

      virtual void startData(
        Url source_url
      ) {}

      virtual void startProbe(Url url) {}

      virtual void endProbe(Url url) {}

      virtual bool progressData(int value, Url url)
      { return true; }

      virtual Action problem(
        Url url
          , Error error
          , std::string description
      ) { return ABORT; }

      virtual void finishData(
        Url url
        , Error error
        , std::string reason
      ) {}
    };

    // progress for probing a source
    struct ProbeSourceReport : public callback::ReportBase
    {
      enum Action {
        ABORT,  // abort and return error
        RETRY	// retry
      };

      enum Error {
        NO_ERROR,
        NOT_FOUND, 	// the requested Url was not found
        IO,		// IO error
        INVALID		// th source is invalid
      };

      virtual void start(const Url &url) {}
      virtual void failedProbe( const Url &url, const std::string &type ) {}
      virtual void successProbe( const Url &url, const std::string &type ) {}
      virtual void finish(const Url &url, Error error, std::string reason ) {}

      virtual bool progress(const Url &url, int value)
      { return true; }

      virtual Action problem( const Url &url, Error error, std::string description ) { return ABORT; }
    };
    
    // progress for refreshing a source data
    struct SourceProcessReport : public callback::ReportBase
    {
      enum Action {
        ABORT,  // abort and return error
        RETRY,	// retry
        IGNORE  // skip refresh, ignore failed refresh
      };

      enum Error {
        NO_ERROR,
        NOT_FOUND, 	// the requested Url was not found
        IO,		// IO error
        INVALID		// th source is invalid
      };
      
      virtual void start( Source_Ref source ) {}
      virtual bool progress(int value, Source_Ref source)
      { return true; }

      virtual Action problem(
          Source_Ref source
          , Error error
          , std::string description )
      { return ABORT; }

      virtual void finish(
          Source_Ref source
          , Error error
          , std::string reason )
      {}
    };
    
    
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace media
  {
    // media change request callback
    struct MediaChangeReport : public callback::ReportBase
    {
      enum Action {
        ABORT,  // abort and return error
        RETRY,	// retry
        IGNORE, // ignore this media in future, not available anymore
        IGNORE_ID,	// ignore wrong medium id
        CHANGE_URL,	// change media URL
        EJECT		// eject the medium
      };

      enum Error {
        NO_ERROR,
        NOT_FOUND,  // the medie not found at all
        IO,	// error accessing the media
        INVALID, // media is broken
        WRONG	// wrong media, need a different one
      };

      virtual Action requestMedia(
        const Source_Ref source
        , unsigned mediumNr
        , Error error
        , std::string description
      ) { return ABORT; }
    };

    // progress for downloading a file
    struct DownloadProgressReport : public callback::ReportBase
    {
        enum Action {
          ABORT,  // abort and return error
          RETRY,	// retry
          IGNORE	// ignore the failure
        };

        enum Error {
          NO_ERROR,
          NOT_FOUND, 	// the requested Url was not found
          IO		// IO error
        };

        virtual void start( Url file, Pathname localfile ) {}

        virtual bool progress(int value, Url file)
        { return true; }

        virtual Action problem(
          Url file
  	  , Error error
  	  , std::string description
        ) { return ABORT; }

        virtual void finish(
          Url file
          , Error error
	  , std::string reason
        ) {}
    };

    /////////////////////////////////////////////////////////////////
  } // namespace media
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace target
  {

    // resolvable Message
    struct MessageResolvableReport : public callback::ReportBase
    {
        virtual void show(
	  Message::constPtr message
        ) {}
    };

    ///////////////////////////////////////////////////////////////////
    namespace rpm
    {

      // progress for installing a resolvable
      struct InstallResolvableReport : public callback::ReportBase
      {
        enum Action {
          ABORT,  // abort and return error
          RETRY,	// retry
	  IGNORE	// ignore the failure
        };

        enum Error {
	  NO_ERROR,
          NOT_FOUND, 	// the requested Url was not found
	  IO,		// IO error
	  INVALID		// th resolvable is invalid
        };

        // the level of RPM pushing
        enum RpmLevel {
            RPM,
            RPM_NODEPS,
            RPM_NODEPS_FORCE
        };

        virtual void start(
	  Resolvable::constPtr resolvable
        ) {}

        virtual bool progress(int value, Resolvable::constPtr resolvable)
        { return true; }

        virtual Action problem(
          Resolvable::constPtr resolvable
  	  , Error error
  	  , std::string description
	  , RpmLevel level
        ) { return ABORT; }

        virtual void finish(
          Resolvable::constPtr resolvable
          , Error error
	  , std::string reason
	  , RpmLevel level
        ) {}
      };

      // progress for removing a resolvable
      struct RemoveResolvableReport : public callback::ReportBase
      {
        enum Action {
          ABORT,  // abort and return error
          RETRY,	// retry
	  IGNORE	// ignore the failure
        };

        enum Error {
	  NO_ERROR,
          NOT_FOUND, 	// the requested Url was not found
	  IO,		// IO error
	  INVALID		// th resolvable is invalid
        };

        virtual void start(
	  Resolvable::constPtr resolvable
        ) {}

        virtual bool progress(int value, Resolvable::constPtr resolvable)
        { return true; }

        virtual Action problem(
          Resolvable::constPtr resolvable
  	  , Error error
  	  , std::string description
        ) { return ABORT; }

        virtual void finish(
          Resolvable::constPtr resolvable
          , Error error
	  , std::string reason
        ) {}
      };

      // progress for rebuilding the database
      struct RebuildDBReport : public callback::ReportBase
      {
        enum Action {
          ABORT,  // abort and return error
          RETRY,	// retry
	  IGNORE	// ignore the failure
        };

        enum Error {
	  NO_ERROR,
	  FAILED		// failed to rebuild
        };

        virtual void start(Pathname path) {}

        virtual bool progress(int value, Pathname path)
        { return true; }

        virtual Action problem(
	  Pathname path
  	 , Error error
  	 , std::string description
        ) { return ABORT; }

        virtual void finish(
	  Pathname path
          , Error error
	  , std::string reason
        ) {}
      };

      // progress for converting the database
      struct ConvertDBReport : public callback::ReportBase
      {
        enum Action {
          ABORT,  // abort and return error
          RETRY,	// retry
	  IGNORE	// ignore the failure
        };

        enum Error {
	  NO_ERROR,
	  FAILED		// conversion failed
        };

        virtual void start(
	  Pathname path
        ) {}

        virtual bool progress(int value, Pathname path)
        { return true; }

        virtual Action problem(
	  Pathname path
  	  , Error error
  	 , std::string description
        ) { return ABORT; }

        virtual void finish(
	  Pathname path
          , Error error
	  , std::string reason
        ) {}
      };

       // progress for scanning the database
      struct ScanDBReport : public callback::ReportBase
      {
        enum Action {
          ABORT,  // abort and return error
          RETRY,	// retry
	  IGNORE	// ignore the failure
        };

        enum Error {
	  NO_ERROR,
	  FAILED		// conversion failed
        };

        virtual void start(
        ) {}

        virtual bool progress(int value)
        { return true; }

        virtual Action problem(
  	  Error error
  	 , std::string description
        ) { return ABORT; }

        virtual void finish(
          Error error
	  , std::string reason
        ) {}
      };

      /////////////////////////////////////////////////////////////////
    } // namespace rpm
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

#endif // ZYPP_ZYPPCALLBACKS_H
