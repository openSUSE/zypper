/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/LogControl.cc
 *
*/
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/base/LogControl.h"
#include "zypp/base/String.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace base
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace logger
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : LogControlImpl
      //
      /** LogControl implementation. */
      struct LogControlImpl
      {
      public:
        const Pathname & logfile() const
        { return _logfile; }

        /** \todo IMPEMENT it */
        void logfile( const Pathname & logfile_r )
        {}

        void excessive( bool onOff_r )
        { _excessive = onOff_r; }

      public:
        /** Provide the stream to write (logger interface) */
        std::ostream & getStream( const char * group_r,
                                  LogLevel     level_r,
                                  const char * file_r,
                                  const char * func_r,
                                  const int    line_r );
      private:
        /** Current output stream. */
        std::ostream & outStr()
        { return *_outStrPtr; }

        /** Output stream for level XXX */
        std::ostream & fullStr()
        { return _excessive ? outStr() : _no_stream; }

      private:
        std::ostream _no_stream;

        /** must pont to the current outpot stream or _no_stream! */
        std::ostream *_outStrPtr;

        Pathname     _logfile;
        bool         _excessive;

      private:
        /** Singleton */
        LogControlImpl()
        : _no_stream( 0 )
        , _outStrPtr( getenv("ZYPP_NOLOG") ? &_no_stream : &std::cerr )
        , _excessive( getenv("ZYPP_FULLLOG") )
        {}

      public:
        /** Offer default Impl. */
        static shared_ptr<LogControlImpl> instance()
        {
          static shared_ptr<LogControlImpl> _instance( new LogControlImpl );
          return _instance;
        }
      };
      ///////////////////////////////////////////////////////////////////

      /** \relates LogControl::Impl Stream output */
      inline std::ostream & operator<<( std::ostream & str, const LogControlImpl & obj )
      {
        return str << "LogControl::Impl";
      }

      /** That's what logger:: calls.  */
      std::ostream & getStream( const char * group_r,
                                LogLevel     level_r,
                                const char * file_r,
                                const char * func_r,
                                const int    line_r )
      {
        // forward to LogControlImpl singleton
        static shared_ptr<LogControlImpl> _logControl( LogControlImpl::instance() );
        return _logControl->getStream( group_r, level_r, file_r, func_r, line_r );
      }

      ///////////////////////////////////////////////////////////////////
      //
      // CLASS NAME : LogControlImpl
      //
      ///////////////////////////////////////////////////////////////////

      std::ostream & LogControlImpl::getStream( const char * group_r,
                                                LogLevel     level_r,
                                                const char * file_r,
                                                const char * func_r,
                                                const int    line_r )
      {
        return (level_r != E_XXX ? outStr() : fullStr() )
        << str::form( "<%d> [%s] %s(%s):%d ",
                      level_r, group_r,
                      file_r, func_r, line_r );
      }

      /////////////////////////////////////////////////////////////////
    } // namespace logger
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : LogControl
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : LogControl::instance
    //	METHOD TYPE : LogControl
    //
    LogControl LogControl::instance()
    {
      return LogControl();
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : LogControl::LogControl
    //	METHOD TYPE : Ctor
    //
    LogControl::LogControl()
    : _pimpl( Impl::instance() )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : LogControl::~LogControl
    //	METHOD TYPE : Dtor
    //
    LogControl::~LogControl()
    {}

    ///////////////////////////////////////////////////////////////////
    //
    // Forward to implementation.
    //
    ///////////////////////////////////////////////////////////////////

    const Pathname & LogControl::logfile() const
    { return _pimpl->logfile(); }

    void LogControl::logfile( const Pathname & logfile_r )
    { _pimpl->logfile( logfile_r ); }

    ///////////////////////////////////////////////////////////////////
    //
    // LogControl::TmpExcessive
    //
    ///////////////////////////////////////////////////////////////////
    LogControl::TmpExcessive::TmpExcessive()
    { logger::LogControlImpl::instance()->excessive( true ); }
    LogControl::TmpExcessive::~TmpExcessive()
    { logger::LogControlImpl::instance()->excessive( false );  }

    /******************************************************************
     **
     **	FUNCTION NAME : operator<<
     **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const LogControl & obj )
    {
      return str << *obj._pimpl;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
