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
#include <zypp/base/Regex.h>
#include <zypp/ZYppCallbacks.h>

#include "Zypper.h"
#include "output/prompt.h"

///////////////////////////////////////////////////////////////////
namespace env
{
  bool ZYPPER_ON_CODE12_RETURN_107(); // in callbacks/rpm.h
} // namespace env
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace ZmartRecipients
{
  /** bsc#1198139: return 107 if %posttrans fails.
   * %posttrans are executed by libzypp directly, that's why the failure is
   * reported here and not via the rpm callbacks.
   * Returning 107 is not backported to Code-12,but we make it available if
   * $ZYPPER_ON_CODE12_RETURN_107 is set in the environment.
   **/
  inline void checkFailedPosttrans( const std::string & msg_r )
  {
    static const bool envZYPPER_ON_CODE12_RETURN_107 = env::ZYPPER_ON_CODE12_RETURN_107();
    if ( envZYPPER_ON_CODE12_RETURN_107 ) {
      static str::regex  rx("%posttrans script failed \\(returned");  // sent from libzypp(RpmPostTransCollector)
      static str::smatch what;
      if ( str::regex_match( msg_r, what, rx ) )
        Zypper::instance().setExitInfoCode( ZYPPER_EXIT_INF_RPM_SCRIPT_FAILED );
    }
  }

  ///////////////////////////////////////////////////////////////////
  /// \class JobReportReceiver
  /// \brief Receive generic notification callbacks
  ///////////////////////////////////////////////////////////////////
  struct JobReportReceiver : public callback::ReceiveReport<JobReport>
  {
    virtual bool message( MsgType type_r, const std::string & msg_r, const UserData & userData_r ) const
    {
      Out & out( Zypper::instance().out() );
      switch ( type_r.asEnum() )
      {
	case MsgType::debug:
	  out.info( "[zypp] "+msg_r, Out::DEBUG, Out::TYPE_NORMAL );
	  break;

	case MsgType::info:
	  out.info( msg_r );
	  break;

	case MsgType::warning:
          checkFailedPosttrans( msg_r );  // bsc#1198139: return 107 if %posttrans fails
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
