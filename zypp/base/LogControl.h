/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/LogControl.h
 *
*/
#ifndef ZYPP_BASE_LOGCONTROL_H
#define ZYPP_BASE_LOGCONTROL_H

#include <iosfwd>

#include "zypp/base/Logger.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace base
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : LogControl
    //
    /** Maintain logfile related options.
     * \note A Singleton using a Singleton implementation class,
     * that's why there is no _pimpl like in other classes.
    */
    class LogControl
    {
      friend std::ostream & operator<<( std::ostream & str, const LogControl & obj );

    public:
      /** Singleton access. */
      static LogControl instance()
      { return LogControl(); }


      /** If you want to log the (formated) loglines by yourself,
       *  derive from this, and overload \c writeOut.
       * Expect \a formated_r to be a formated log line without trailing \c NL.
       * Ready to be written to the log.
      */
      struct LineWriter
      {
        virtual void writeOut( const std::string & /*formated_r*/ )
        {}
        virtual ~LineWriter() {}
      };

      /** If you want to format loglines by yourself,
       *  derive from this, and overload \c format.
       * Return a formated logline without trailing \c NL.
       * Ready to be written to the log.
      */
      struct LineFormater
      {
        virtual std::string format( const std::string & /*group_r*/,
                                    logger::LogLevel    /*level_r*/,
                                    const char *        /*file_r*/,
                                    const char *        /*func_r*/,
                                    int                 /*line_r*/,
                                    const std::string & /*message_r*/ );
        virtual ~LineFormater() {}
      };
            
    public:
      /** Assign a LineFormater.
       * If you want to format loglines by yourself. NULL installs the
       * default formater.
      */
      void setLineFormater( const shared_ptr<LineFormater> & formater_r );

      /** Set path for the logfile.
       * Permission for logfiles is set to 0640 unless an explicit mode_t
       * value is given. An empty pathname turns off logging. <tt>"-"</tt>
       * logs to std::err.
       * \throw if \a logfile_r is not usable.
      */
      void logfile( const Pathname & logfile_r );
      void logfile( const Pathname & logfile_r, mode_t mode_r );

      /** Turn off logging. */
      void logNothing();

      /** Log to std::err. */
      void logToStdErr();

      /** Assign a LineWriter.
       * If you want to log the (formated) loglines by yourself.
       * NULL turns off logging (same as logNothing)
      */
      void setLineWriter( const shared_ptr<LineWriter> & writer_r );

    public:
      /** Turn on excessive logging for the lifetime of this object.*/
      struct TmpExcessive
      {
        TmpExcessive();
        ~TmpExcessive();
      };

      /** Exchange LineWriter for the lifetime of this object. */
      struct TmpLineWriter
      {
        TmpLineWriter( const shared_ptr<LineWriter> & writer_r = shared_ptr<LineWriter>() );
        ~TmpLineWriter();
      private:
        shared_ptr<LineWriter> _writer;
      };

    private:
      /** Default ctor: Singleton */
      LogControl()
      {}
      bool _log_microseconds;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates LogControl Stream output */
    std::ostream & operator<<( std::ostream & str, const LogControl & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_LOGCONTROL_H
