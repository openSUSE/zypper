/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZMART_SOURCE_CALLBACKS_H
#define ZMART_SOURCE_CALLBACKS_H

#include <stdlib.h>
#include <iostream>

#include <zypp/base/Logger.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/Pathname.h>
#include <zypp/KeyRing.h>
#include <zypp/Digest.h>
#include <zypp/Url.h>
#include <zypp/Source.h>

#include "AliveCursor.h"

///////////////////////////////////////////////////////////////////
namespace ZmartRecipients
{
///////////////////////////////////////////////////////////////////    
    // progress for probing a source
    struct ProbeSourceReceive : public zypp::callback::ReceiveReport<zypp::source::ProbeSourceReport>
    {
      virtual void start(const zypp::Url &url)
      {
        cout << "Determining " << url << " source type..." << endl;
      }
      
      virtual void failedProbe( const zypp::Url &url, const std::string &type )
      {
        cout << ".. not " << type << endl;
      }
      
      virtual void successProbe( const zypp::Url &url, const std::string &type )
      {
        cout << url << " is type " << type << endl;
      }
      
      virtual void finish(const zypp::Url &url, Error error, std::string reason )
      {
        if ( error == INVALID )
        {
          cout << reason << endl;
          exit(-1);
        }
      }

      virtual bool progress(const zypp::Url &url, int value)
      { return true; }

      virtual Action problem( const zypp::Url &url, Error error, std::string description )
      {
        cout << error << endl;
        exit(-1);
        return ABORT;
      }
    };
    
    
// progress for downloading a resolvable
struct DownloadResolvableReportReceiver : public zypp::callback::ReceiveReport<zypp::source::DownloadResolvableReport>
{
  zypp::Resolvable::constPtr _resolvable_ptr;
  zypp::Url _url;
  AliveCursor _cursor;
  zypp::Pathname _delta;
  zypp::ByteCount _delta_size;
  zypp::Pathname _patch;
  zypp::ByteCount _patch_size;
  
  virtual void start( zypp::Resolvable::constPtr resolvable_ptr, const zypp::Url &url )
  {
    _resolvable_ptr =  resolvable_ptr;
    _url = url;
  }
   
  void display_step( std::string what, int value )
  {
    cout << "\x1B 2K\r" << _cursor << " " <<  what << " [" << value << " %]  ";
    ++_cursor;
  }
  
  // Dowmload delta rpm:
  // - path below url reported on start()
  // - expected download size (0 if unknown)
  // - download is interruptable
  // - problems are just informal
  virtual void startDeltaDownload( const zypp::Pathname & filename, const zypp::ByteCount & downloadsize )
  {
    
  }
  
  virtual bool progressDeltaDownload( int value )
  {
    display_step( "Downloading delta " + _delta.asString(), value );
  }
  
  virtual void problemDeltaDownload( std::string description )
  {
    std::cout << description << std::endl;
  }
  
  virtual void finishDeltaDownload()
  {
  }
  
  // Apply delta rpm:
  // - local path of downloaded delta
  // - aplpy is not interruptable
  // - problems are just informal
  virtual void startDeltaApply( const zypp::Pathname & filename )
  {
    _delta = filename;
  }
  
  virtual void progressDeltaApply( int value )
  {
    display_step( "Applying delta " + _delta.asString(), value );
  }
  
  virtual void problemDeltaApply( std::string description )
  {
    std::cout << description << std::endl;
  }
  
  virtual void finishDeltaApply()
  {}
  
  // Dowmload patch rpm:
  // - path below url reported on start()
  // - expected download size (0 if unknown)
  // - download is interruptable
  virtual void startPatchDownload( const zypp::Pathname & filename, const zypp::ByteCount & downloadsize )
  {
    _patch = filename;
    _patch_size = downloadsize;
  }
  
  virtual bool progressPatchDownload( int value )
  {
    display_step( "Applying patchrpm " + _patch.asString(), value );
    return true;
  }
  
  virtual void problemPatchDownload( std::string description )
  {
    std::cout << description << std::endl;
  }
  
  virtual void finishPatchDownload()
  {}
  
  
  // return false if the download should be aborted right now
  virtual bool progress(int value, zypp::Resolvable::constPtr resolvable_ptr)
  {
    display_step( "Downloading " + resolvable_ptr->name(), value );
    return true;
  }
  
  virtual Action problem( zypp::Resolvable::constPtr resolvable_ptr, Error error, std::string description )
  {
    std::cout << resolvable_ptr << " " << description << std::endl;
    return ABORT;
  }
  
  virtual void finish( zypp::Resolvable::constPtr resolvable_ptr, Error error, std::string reason )
  {}
};

struct SourceReportReceiver  : public zypp::callback::ReceiveReport<zypp::source::SourceReport>
{     
  virtual void start( zypp::Source_Ref source, const std::string &task )
  {
    if ( source != _source )
      cout << endl;

    _task = task;
    _source = source;
    
    display_step(0);
  }
  
  void display_step( int value )
  {
    cout << "\x1B 2K\r" << _cursor << " " <<  _task << " [" << value << " %]  ";
    ++_cursor;
  }
  
  virtual bool progress( int value )
  {
    display_step(value);
    return true;
  }
  
  virtual Action problem( zypp::Source_Ref source, Error error, std::string description )
  { return ABORT; }

  virtual void finish( zypp::Source_Ref source, const std::string task, Error error, std::string reason )
  {
    if ( error == SourceReportReceiver::NO_ERROR )
      display_step(100);
  }
  
  AliveCursor _cursor;
  std::string _task;
  zypp::Source_Ref _source;
};

    ///////////////////////////////////////////////////////////////////
}; // namespace ZmartRecipients
///////////////////////////////////////////////////////////////////

class SourceCallbacks {

  private:
    ZmartRecipients::ProbeSourceReceive _sourceProbeReport;
    ZmartRecipients::SourceReportReceiver _SourceReport;
    ZmartRecipients::DownloadResolvableReportReceiver _downloadReport;
  public:
    SourceCallbacks()
    {
      _sourceProbeReport.connect();
      _SourceReport.connect();
      _downloadReport.connect();
    }

    ~SourceCallbacks()
    {
      _sourceProbeReport.disconnect();
      _SourceReport.disconnect();
      _downloadReport.disconnect();
    }

};

#endif 
