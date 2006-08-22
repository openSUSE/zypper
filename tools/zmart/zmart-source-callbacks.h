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
    
    
    ///////////////////////////////////////////////////////////////////
}; // namespace zypp
///////////////////////////////////////////////////////////////////

class SourceCallbacks {

  private:
    ZmartRecipients::ProbeSourceReceive _sourceReport;

  public:
    SourceCallbacks()
    {
      _sourceReport.connect();
    }

    ~SourceCallbacks()
    {
      _sourceReport.disconnect();
    }

};

#endif 
