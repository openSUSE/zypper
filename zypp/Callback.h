/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Callback.h
 *
*/
#ifndef ZYPP_CALLBACK_H
#define ZYPP_CALLBACK_H

#include "zypp/base/NonCopyable.h"
#include "zypp/UserData.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /** \todo Eliminate this! */
  namespace HACK {
    class Callback
    {
    };
  } // namespace HACK

  ///////////////////////////////////////////////////////////////////
  /** Callbacks light.
   *
   * \par The task report structure (SENDER SIDE).
   *
   * A default constructible struct derived from callback::ReportBase.
   * It \b must \b not contain any data, just virtual methods.
   *
   * These are the functions the sender invokes, and which will be forwarded
   * to some receiver. If no receiver is present, the defined default
   * implementations are invoked.
   *
   * For methods returning non-void, define a reasonable return value,
   * because this is what you get back in case no receiver is listening.
   *
   * That way the sending side does not need to know whether some receiver
   * is listening. And it enables the receiver to return a reasonable value,
   * in case he's got no idea, what else to return.
   *
   * \code
   *   struct Foo : public callback::ReportBase
   *   {
   *     virtual void ping( int i )
   *     {}
   *     virtual int pong()
   *     { return -1; }
   *
   *   };
   * \endcode
   *
   * \par Sending a Task report (SENDER SIDE).
   *
   * Simply create a callback::SendReport<_Report>, where _Report
   * is your task report structure. Invoke the callback functions
   * as needed. That's it.
   *
   * \note Even creation and destruction of a callback::SendReport
   * are indicated to a receiver. So even in case of an Exception,
   * the receiver is able to recognize, that the task ended.
   * So don't create it without need.
   *
   * \code
   * {
   *   callback::SendReport<Foo> report;
   *   report->ping( 13 );
   *   int response = report->pong();
   * }
   * \endcode
   *
   * \par Receiving Task reports (RECEIVER SIDE).
   *
   * To receive task reports of type \c Foo the recipient class
   * derives from callback::ReceiveReport\<Foo\>. callback::ReceiveReport
   * inherits \c Foo and provides two additional virtual methods:
   *
   * \code
   *   virtual void reportbegin() {}
   *   virtual void reportend() {}
   * \endcode
   *
   * These two are automatically invoked, whenever the sender
   * creates a callback::SendReport instance, and when it gets
   * destructed. So even if the sending task is aborted without
   * sending an explicit notification, the reciever may notice it,
   * by overloading \c reportend.
   *
   * Overload the methods you're interested in.
   *
   * \note In case you must return some value and don't know which,
   * return the task structures default. The author of the task
   * structure had to provide this value, so it's probabely better
   * than anything you \e invent.
   * \code
   *   int somefunction()
   *   {
   *     ...// don't know what to return?
   *     return Foo::somefunction();
   *   }
   * \endcode
   *
   * \par Connecting the Receiver
   *
   * For this callback::ReceiveReport provides 4 methods:
   * \code
   *  void connect();
   *  void disconnect();
   *  bool connected() const;
   *  ReceiveReport * whoIsConnected() const;
   * \endcode
   *
   * \li \c connect Connect this ReceiveReport (in case some other
   * ReceiveReport is connected, it get disconnected. Remember its
   * a Callback light).
   * \li \c disconnect Disconnect this ReceiveReport in case it is
   * connected. If not connected nothing happens.
   * \li \c connected Test whether this ReceiveReport is currently
   * connected.
   * \li \c whoIsConnected Return a 'ReceiveReport*' to the currently
   * connected ReceiveReport, or \c NULL if none is connected.
   *
   * \par Passing Userdata via Callbacks
   *
   * For typesafe passing of user data via callbacks \see \ref UserData.
   *
   * ReportBase provides a generic \ref callback::ReportBase:report method
   * which can be used to communicate by encoding everything in its \a UserData
   * argument.
   *
   * Convenient sending can be achieved by installing non-virtual methods
   * in the _Report class, which encode the arguments in UserData and send
   * them via ReportBase::report().
   *
   * Convenient receiving can be achieved by installing virtual methods in
   * the _Report class, which can be simply overloaded by the receiver. Downside
   * of this is that adding virtual methods breaks binary compatibility.
   */
  namespace callback
  { /////////////////////////////////////////////////////////////////

    /**  */
    struct ReportBase
    {
      typedef callback::UserData UserData;
      typedef UserData::ContentType ContentType;

      /** The most generic way of sending/receiving data. */
      virtual void report( const UserData & userData_r = UserData() )
      {}

      virtual ~ReportBase()
      {}
    };

    /**  */
    template<class _Report>
      class DistributeReport;

    /**  */
    template<class _Report>
      struct ReceiveReport : public _Report
      {
	typedef _Report                   ReportType;
	typedef ReceiveReport<_Report>    Receiver;
        typedef DistributeReport<_Report> Distributor;

        virtual ~ReceiveReport()
        { disconnect(); }

        ReceiveReport * whoIsConnected() const
        { return Distributor::instance().getReceiver(); }

        bool connected() const
        { return whoIsConnected() == this; }

        void connect()
        { Distributor::instance().setReceiver( *this ); }

        void disconnect()
        { Distributor::instance().unsetReceiver( *this ); }

        virtual void reportbegin()
        {}
        virtual void reportend()
        {}
      };

    /**  */
    template<class _Report>
      struct DistributeReport
      {
       public:
	typedef _Report                   ReportType;
	typedef ReceiveReport<_Report>    Receiver;
	typedef DistributeReport<_Report> Distributor;

         static DistributeReport & instance()
         {
           static DistributeReport _self;
           return _self;
         }

         Receiver * getReceiver() const
         { return _receiver == &_noReceiver ? 0 : _receiver; }

         void setReceiver( Receiver & rec_r )
         { _receiver = &rec_r; }

         void unsetReceiver( Receiver & rec_r )
         { if ( _receiver == &rec_r ) noReceiver(); }

         void noReceiver()
         { _receiver = &_noReceiver; }

      public:
         Receiver * operator->()
         { return _receiver; }

      private:
        DistributeReport()
        : _receiver( &_noReceiver )
        {}
        Receiver _noReceiver;
        Receiver * _receiver;
      };

    /**  */
    template<class _Report>
      struct SendReport : private zypp::base::NonCopyable
      {
	typedef _Report                   ReportType;
        typedef ReceiveReport<_Report>    Receiver;
        typedef DistributeReport<_Report> Distributor;

        SendReport()
        { Distributor::instance()->reportbegin(); }

        ~SendReport()
        { Distributor::instance()->reportend(); }

        static Receiver * whoIsConnected()
        { return Distributor::instance().getReceiver(); }

        static bool connected()
        { return whoIsConnected(); }

        Distributor & operator->()
        { return Distributor::instance(); }
      };

    /** Temporarily connect a ReceiveReport then restore the previous one.
     *
     * Pass the ReceiveReport you want to connect temporarily
     * to the ctor. The ReceiveReport is connected, a previously
     * connected ReceiveReport is remembered and re-connected in
     * the dtor.
     * Use the default ctpr to temporarily disconnect any connected report.
     * \code
     *  struct FooReceive : public callback::ReceiveReport<Foo>
     *  {..};
     *  struct FooReceive2 : public callback::ReceiveReport<Foo>
     *  {..};
     *
     *  FooReceive  r;
     *  FooReceive2 r2;
     *
     *  r.connect();
     *  ... // r receiving the report
     *  {
     *    callback::TempConnect<Foo> temp( r2 );
     *    ...// r2 receiving the report
     *  }
     *  ...// r receiving the report
     * \endcode
    */
    template<class _Report>
      struct TempConnect
      {
	typedef _Report                   ReportType;
        typedef ReceiveReport<_Report>    Receiver;
        typedef DistributeReport<_Report> Distributor;

        TempConnect()
        : _oldRec( Distributor::instance().getReceiver() )
        {
          Distributor::instance().noReceiver();
        }

        TempConnect( Receiver & rec_r )
        : _oldRec( Distributor::instance().getReceiver() )
        {
          rec_r.connect();
        }

        ~TempConnect()
        {
          if ( _oldRec )
            Distributor::instance().setReceiver( *_oldRec );
          else
            Distributor::instance().noReceiver();
        }
      private:
        Receiver * _oldRec;
      };

    /////////////////////////////////////////////////////////////////
  } // namespace callback
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CALLBACK_H
