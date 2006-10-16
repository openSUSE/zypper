/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZMART_RPM_CALLBACKS_H
#define ZMART_RPM_CALLBACKS_H

#include <iostream>
#include <string>

#include <zypp/base/Logger.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/Package.h>
//#include <zypp/target/rpm/RpmCallbacks.h>

#include "AliveCursor.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace ZmartRecipients
{

// resolvable Message
struct MessageResolvableReportReceiver : public zypp::callback::ReceiveReport<zypp::target::MessageResolvableReport>
{
  virtual void show( zypp::Message::constPtr message )
  {
   
  }
};

///////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////
struct ScanRpmDbReceive : public zypp::callback::ReceiveReport<zypp::target::rpm::ScanDBReport>
{
  int & _step;				// step counter for install & receive steps
  int last_reported;
  AliveCursor _cursor;
  
  ScanRpmDbReceive( int & step )
  : _step( step )
  {
  }

  ~ScanRpmDbReceive( )
  {
  }
  
  virtual void reportbegin()
  {
    cout << _cursor << " 0 %"; 
  }

  virtual void reportend()
  {
    cout << "\r done..." << endl; 
  }

  virtual void start()
  {
    last_reported = 0;

  }

  virtual bool progress(int value)
  {
    cout << "\r" << _cursor << " " << value << "%";
    ++_cursor;
    return true;
  }

  virtual Action problem( zypp::target::rpm::ScanDBReport::Error error, const std::string& description )
  {
    return zypp::target::rpm::ScanDBReport::problem( error, description );
  }

  virtual void finish( Error error, const std::string& reason )
  {
    string errmsg;
    switch (error) {
      case NO_ERROR:
        return;
        break;
      case FAILED:
        errmsg = "FAILED";
        break;
    }
    return;
  }
};

 // progress for removing a resolvable
struct RemoveResolvableReportReceiver : public zypp::callback::ReceiveReport<zypp::target::rpm::RemoveResolvableReport>
{
  virtual void start( zypp::Resolvable::constPtr resolvable )
  {}

  virtual bool progress(int value, zypp::Resolvable::constPtr resolvable)
  { return true; }

  virtual Action problem( zypp::Resolvable::constPtr resolvable, Error error, const std::string& description )
  { return ABORT; }

  virtual void finish( zypp::Resolvable::constPtr resolvable, Error error, const std::string& reason )
  {}
};

// progress for installing a resolvable
struct InstallResolvableReportReceiver : public zypp::callback::ReceiveReport<zypp::target::rpm::InstallResolvableReport>
{
  AliveCursor _cursor;
  zypp::Resolvable::constPtr _resolvable;
  
  void display_step( zypp::Resolvable::constPtr resolvable, int value )
  {
    cout << CLEARLN << _cursor << " Installing " <<  resolvable << " [" << value << " %]  " << flush;
    ++_cursor;
  }
  
  virtual void start( zypp::Resolvable::constPtr resolvable )
  {
    _resolvable = resolvable;
  }

  virtual bool progress(int value, zypp::Resolvable::constPtr resolvable)
  {
    display_step( resolvable, value );
    return true;
  }

  virtual Action problem( zypp::Resolvable::constPtr resolvable, Error error, const std::string& description, RpmLevel level )
  {
    std::cout << resolvable << " " << description << std::endl;
    return ABORT;
  }

  virtual void finish( zypp::Resolvable::constPtr resolvable, Error error, const std::string& reason, RpmLevel level )
  {}
};


///////////////////////////////////////////////////////////////////
}; // namespace ZyppRecipients
///////////////////////////////////////////////////////////////////

class RpmCallbacks {

  private:
    ZmartRecipients::MessageResolvableReportReceiver _messageReceiver;
    ZmartRecipients::ScanRpmDbReceive _readReceiver;
    ZmartRecipients::RemoveResolvableReportReceiver _installReceiver;
    ZmartRecipients::InstallResolvableReportReceiver _removeReceiver;
    int _step_counter;

  public:
    RpmCallbacks()
	: _readReceiver( _step_counter )
	//, _removeReceiver( _step_counter )
	, _step_counter( 0 )
    {
      _messageReceiver.connect();
      _installReceiver.connect();
      _removeReceiver.connect();
      _readReceiver.connect();
    }

    ~RpmCallbacks()
    {
      _messageReceiver.disconnect();
      _installReceiver.disconnect();
      _removeReceiver.disconnect();
      _readReceiver.connect();
    }
};

#endif // ZMD_BACKEND_RPMCALLBACKS_H
