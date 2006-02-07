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
        RETRY	// retry
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

      // return false if the download should be aborted right now
      virtual bool progress(int value, Resolvable::constPtr resolvable_ptr) 
      { return true; }

      virtual Action problem(
        Resolvable::Ptr resolvable_ptr
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

    // progress for creating a source (download and parsing)
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
	  Resolvable::Ptr resolvable
        ) {}

        virtual bool progress(int value, Resolvable::Ptr resolvable) 
        { return true; }

        virtual Action problem(
          Resolvable::Ptr resolvable
  	  , Error error
  	  , std::string description
        ) { return ABORT; }

        virtual void finish(
          Resolvable::Ptr resolvable
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
          NOT_FOUND, 	// the requested Url was not found
	  IO,		// IO error
	  INVALID		// th resolvable is invalid
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
          NOT_FOUND, 	// the requested Url was not found
	  IO,		// IO error
	  INVALID		// th resolvable is invalid
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
