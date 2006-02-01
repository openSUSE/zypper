/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Source.h
 *
*/
#ifndef ZYPP_ZYPPCALLBACKS_H
#define ZYPP_ZYPPCALLBACKS_H

#include <iosfwd>
#include <string>

#include "zypp/Callback.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  namespace source
  {
    // progress for downloading a resolvable
    struct DownloadResolvableReport : public callback::ReportBase
    {
      virtual void start() {}
      virtual void progress(int value) {]
      virtual void finish() {}
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
