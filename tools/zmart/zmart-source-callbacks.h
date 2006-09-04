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
  public:
    SourceCallbacks()
    {
      _sourceProbeReport.connect();
      _SourceReport.connect();
    }

    ~SourceCallbacks()
    {
      _sourceProbeReport.disconnect();
      _SourceReport.disconnect();
    }

};

#endif 
