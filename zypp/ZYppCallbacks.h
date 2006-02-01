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
#include "zypp/TranslatedText.h"

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
        NOT_FOUND, 	// the requested URL was not found
	IO,		// IO error
	INVALID		// the downloaded file is invalid
      }

      virtual void start(
        Resolvable::Ptr resolvable_ptr
	, Source_Ref source
	, Url url
      ) {}

      virtual void progress(int value, Resolvable::Ptr resolvable_ptr) {]

      virtual Action problem(
        Resolvable::Ptr resolvable_ptr
	, Error error
	, TranslatedText description
      ) { return ABORT; ]

      virtual void finish(Resolvable::Ptr resolvable_ptr
        , Error error
	, TranslatedText reason
      ) {}
    };
    

    // progress for downloading a specific file
    struct DownloadFileReport : public callback::ReportBase
    {
      virtual void start() {}
      virtual void progress(int value) {]
      virtual void finish() {}
    };

    // progress for refreshing a source data
    struct RefreshSourceReport : public callback::ReportBase
    {
      virtual void start() {}
      virtual void progress(int value) {]
      virtual void finish() {}
    };

    // progress for downloading a resolvable
    struct CreateSourceReport : public callback::ReportBase
    {
      virtual void start() {}
      virtual void progress(int value) {]
      virtual void finish() {}
    };
    
    // progress for downloading a resolvable
    struct RefreshSourceReport : public callback::ReportBase
    {
      virtual void start() {}
      virtual void progress(int value) {]
      virtual void finish() {}
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
        RETRY	// retry
      }; 

      virtual Action userAction() { return ABORT; }
    };

    /////////////////////////////////////////////////////////////////
  } // namespace media
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace target 
  { ///////////////////////////////////////////////////////////////////
    namespace rpm 
    { 

      // progress for installing a resolvable
      struct InstallResolvableReport : public callback::ReportBase
      {
        enum Action { 
          ABORT,  // abort and return error
          RETRY,  // retry
	  IGNORE  // ignore
        }; 

        virtual void start() {}
        virtual void progress(int value) {]
	virtual Action error () { return ABORT; }
        virtual void finish() {}
      };
    
      // progress for removing a resolvable
      struct RemoveResolvableReport : public callback::ReportBase
      {
        enum Action { 
          ABORT,  // abort and return error
          RETRY,  // retry
	  IGNORE  // ignore
        }; 

        virtual void start() {}
        virtual void progress(int value) {]
	virtual Action error () { return ABORT; }
        virtual void finish() {}
      };
    
      // progress for rebuilding the database
      struct RebuildDBReport : public callback::ReportBase
      {
        virtual void start() {}
        virtual void progress(int value) {]
        virtual void finish() {}
      };

      // progress for converting the database
      struct ConvertDBReport : public callback::ReportBase
      {
        virtual void start() {}
        virtual void progress(int value) {]
        virtual void finish() {}
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
