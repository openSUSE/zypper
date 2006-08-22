/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zmd/backend/RpmCallbacks.cc
 *
*/

#ifndef ZMD_BACKEND_RPMCALLBACKS_H
#define ZMD_BACKEND_RPMCALLBACKS_H

#include <iostream>
#include <string>

#include <zypp/base/Logger.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/Package.h>
#include <zypp/target/rpm/RpmCallbacks.h>

#include "AliveCursor.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace ZmartRecipients
{

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

  virtual Action problem( zypp::target::rpm::ScanDBReport::Error error, std::string description )
  {
    return zypp::target::rpm::ScanDBReport::problem( error, description );
  }

  virtual void finish( Error error, std::string reason )
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

///////////////////////////////////////////////////////////////////
}; // namespace ZyppRecipients
///////////////////////////////////////////////////////////////////

class RpmCallbacks {

  private:
    //ZyppRecipients::InstallPkgReceive _installReceiver;
    //ZyppRecipients::RemovePkgReceive _removeReceiver;
    ZmartRecipients::ScanRpmDbReceive _readReceiver;
    int _step_counter;

  public:
    RpmCallbacks()
	: _readReceiver( _step_counter )
	//, _removeReceiver( _step_counter )
	, _step_counter( 0 )
    {
      //_installReceiver.connect();
      //_removeReceiver.connect();
      _readReceiver.connect();
    }

    ~RpmCallbacks()
    {
      //_installReceiver.disconnect();
      //_removeReceiver.disconnect();
      _readReceiver.connect();
    }

};

#endif // ZMD_BACKEND_RPMCALLBACKS_H
