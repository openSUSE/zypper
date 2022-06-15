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
      static const JobReport::UserData::ContentType rpmPosttrans { "cmdout", "%posttrans" };
      static const JobReport::UserData::ContentType cmdMonitor   { "cmdout", "monitor" };

      if ( userData_r.type() == cmdMonitor ) {
        return doCmdMonitor( type_r, msg_r, userData_r );
      }

      Out & out( Zypper::instance().out() );
      switch ( type_r.asEnum() )
      {
        case MsgType::debug:
          out.info( "[zypp] "+msg_r, Out::DEBUG, Out::TYPE_NORMAL );
          break;

        case MsgType::info:
          if ( userData_r.type() == rpmPosttrans )
          {
            processAdditionalRpmOutput( msg_r );
          }
          else if ( userData_r.type().type() == "cmdout" )
          {
            // Render command output highlighted
            out.info( HIGHLIGHTString(msg_r).str() );
          }
          else
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

    bool doCmdMonitor( MsgType type_r, const std::string & msg_r, const UserData & userData_r ) const
    {
      static unsigned lastid = 0;
      static std::string tag;

      if ( type_r == MsgType::debug ) {
        if ( msg_r == "?" ) {
          userData_r.reset( "!" );  // we are listening!
        }
        else if ( msg_r == "ping" ) {
          // still alive
        }
      }

      if ( type_r == MsgType::data ) {
        Out & out( Zypper::instance().out() );
        unsigned id { userData_r.get( "CmdId", unsigned(0) ) };
        if ( id != lastid ) {
          tag = "#"+userData_r.get( "CmdTag", std::string() )+"> ";
          out.info() << ( ColorContext::HIGHLIGHT << tag << "Output from " << userData_r.get( "CmdName", std::string() ) << ":" ); // info() writes NL
          tag[0] = '[';
          lastid = id;
        }
        out.info() << ( ColorContext::HIGHLIGHT << tag ) << msg_r; // info() writes NL
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
