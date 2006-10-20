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

#include "zypper-callbacks.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace ZmartRecipients
{

// resolvable Message
struct MessageResolvableReportReceiver : public zypp::callback::ReceiveReport<zypp::target::MessageResolvableReport>
{
  virtual void show( zypp::Message::constPtr message )
  {
      std::cerr << message;
  }
};

///////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////
struct ScanRpmDbReceive : public zypp::callback::ReceiveReport<zypp::target::rpm::ScanDBReport>
{
  int & _step;				// step counter for install & receive steps
  int last_reported;
  
  ScanRpmDbReceive( int & step )
  : _step( step )
  {
  }

  virtual void start()
  {
    last_reported = 0;
    progress (0);
  }

  virtual bool progress(int value)
  {
    display_progress ("RPM database", value);
    return true;
  }

  virtual Action problem( zypp::target::rpm::ScanDBReport::Error error, const std::string& description )
  {
    return zypp::target::rpm::ScanDBReport::problem( error, description );
  }

  virtual void finish( Error error, const std::string& reason )
  {
    display_done ();
    display_error (error, reason);
  }
};

 // progress for removing a resolvable
struct RemoveResolvableReportReceiver : public zypp::callback::ReceiveReport<zypp::target::rpm::RemoveResolvableReport>
{
  virtual void start( zypp::Resolvable::constPtr resolvable )
  {
    std::cerr << "Removing: " << *resolvable << std::endl;
  }

  virtual bool progress(int value, zypp::Resolvable::constPtr resolvable)
  {
    display_progress ("Removing " + to_string (resolvable), value);
    return true;
  }

  virtual Action problem( zypp::Resolvable::constPtr resolvable, Error error, const std::string& description )
  {
    cerr << resolvable << error << description << endl;
    return (Action) read_action_ari ();
  }

  virtual void finish( zypp::Resolvable::constPtr resolvable, Error error, const std::string& reason )
  {}
};

// progress for installing a resolvable
struct InstallResolvableReportReceiver : public zypp::callback::ReceiveReport<zypp::target::rpm::InstallResolvableReport>
{
  zypp::Resolvable::constPtr _resolvable;
  
  void display_step( zypp::Resolvable::constPtr resolvable, int value )
  {
    display_progress ("Installing " /* + to_string (resolvable) */,  value);
  }
  
  virtual void start( zypp::Resolvable::constPtr resolvable )
  {
    _resolvable = resolvable;
    cerr << "Installing: " + to_string (resolvable) << endl;
  }

  virtual bool progress(int value, zypp::Resolvable::constPtr resolvable)
  {
    display_step( resolvable, value );
    return true;
  }

  virtual Action problem( zypp::Resolvable::constPtr resolvable, Error error, const std::string& description, RpmLevel level )
  {
    cerr << resolvable << " " << description << std::endl;
    cerr << error << ", " << level << endl;
    return (Action) read_action_ari ();
  }

  virtual void finish( zypp::Resolvable::constPtr resolvable, Error error, const std::string& reason, RpmLevel level )
  {
    display_done ();
    if (error != NO_ERROR) {
      const char * level_s[] = {
	"", "(with nodeps)", "(with nodeps+force)"
      };
      cerr << level_s[level];
    }
    display_error (error, reason);
  }
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
// Local Variables:
// mode: c++
// c-basic-offset: 2
// End:
