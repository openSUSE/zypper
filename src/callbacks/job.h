/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZMART_JOB_CALLBACKS_H
#define ZMART_JOB_CALLBACKS_H

#include <iostream>

#include <zypp/base/Logger.h>
#include <zypp/base/String.h>
#include <zypp/ZYppCallbacks.h>

#include "Zypper.h"
#include "output/prompt.h"

///////////////////////////////////////////////////////////////////
namespace ZmartRecipients
{
  ///////////////////////////////////////////////////////////////////
  /// \class JobReportReceiver
  /// \brief Receive generic notification callbacks
  ///////////////////////////////////////////////////////////////////
  struct JobReportReceiver : public callback::ReceiveReport<JobReport>
  {
    virtual bool message( MsgType type_r, const std::string & msg_r, const UserData & userData_r ) const
    {
      Out & out( Zypper::instance()->out() );
      switch ( type_r.asEnum() )
      {
	case MsgType::debug:
	  out.info( "[zypp] "+msg_r, Out::DEBUG, Out::TYPE_NORMAL );
	  break;

	case MsgType::info:
	  out.info( msg_r );
	  break;

	case MsgType::warning:
	  out.warning( msg_r );
	  break;

	case MsgType::error:
	  out.error( msg_r );
	  break;


	case MsgType::important:
	case MsgType::data:
	default:
	  INT << "Unhandled MsgType(" << type_r.asEnum() << "): " << msg_r << endl;
	  break;
      }
      return true;
    }
  };
} // namespace ZmartRecipients
///////////////////////////////////////////////////////////////////

class JobCallbacks
{
  public:
    JobCallbacks()
    {
      _jobReport.connect();
    }

    ~JobCallbacks()
    {
      _jobReport.disconnect();
    }

  private:
    ZmartRecipients::JobReportReceiver _jobReport;
};

#endif // ZMART_JOB_CALLBACKS_H
