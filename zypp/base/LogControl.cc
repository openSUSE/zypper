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
#include <fstream>
#include <string>

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

      void logFormat( const std::string & group_r, LogLevel level_r,
                      const char * file_r, const char * func_r, int line_r,
                      const std::string & buffer_r );

      ///////////////////////////////////////////////////////////////////
      struct StdErrWriter : public LogControl::LineWriter
      {
        virtual void writeOut( const std::string & formated_r )
        {
          std::cerr << formated_r << endl;
        }
      };
      ///////////////////////////////////////////////////////////////////
      struct FileWriter : public LogControl::LineWriter
      {
        FileWriter( const Pathname & logfile_r )
        : _logfile( logfile_r )
        {}
        Pathname _logfile;

        virtual void writeOut( const std::string & formated_r )
        {
          std::ofstream outs( _logfile.asString().c_str(), std::ios_base::app );
          outs << formated_r << endl;
        }
      };

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : Loglinebuf
      //
      class Loglinebuf : public std::streambuf {

      public:
        /** */
        Loglinebuf( const std::string & group_r, LogLevel level_r )
        : _group( group_r )
        , _level( level_r )
        , _file( "" )
        , _func( "" )
        , _line( -1 )
        {}
        /** */
        ~Loglinebuf()
        {
          if ( !_buffer.empty() )
            writeout( "\n", 1 );
        }

        /** */
        void tagSet( const char * fil_r, const char * fnc_r, int lne_r )
        {
          _file = fil_r;
          _func = fnc_r;
          _line = lne_r;
        }

      private:
        /** */
        virtual std::streamsize xsputn( const char * s, std::streamsize n )
        { return writeout( s, n ); }
        /** */
        virtual int overflow( int ch = EOF )
        {
          if ( ch != EOF )
            {
              char tmp = ch;
              writeout( &tmp, 1 );
            }
          return 0;
        }
        /** */
        virtual int writeout( const char* s, std::streamsize n )
        {
          if ( s && n )
            {
              const char * c = s;
              for ( int i = 0; i < n; ++i, ++c )
                {
                  if ( *c == '\n' ) {
                    _buffer += std::string( s, c-s );
                    logger::logFormat( _group, _level, _file, _func, _line, _buffer );
                    _buffer = std::string();
                    s = c+1;
                  }
                }
              if ( s < c )
                {
                  _buffer += std::string( s, c-s );
                }
            }
          return n;
        }

      private:
        std::string  _group;
        LogLevel     _level;
        const char * _file;
        const char * _func;
        int          _line;
        std::string  _buffer;
      };

      ///////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : Loglinestream
      //
      class Loglinestream {

      public:
        /** */
        Loglinestream( const std::string & group_r, LogLevel level_r )
        : _mybuf( group_r, level_r )
        , _mystream( &_mybuf )
        {}
        /** */
        ~Loglinestream()
        { _mystream.flush(); }

      public:
        /** */
        std::ostream & getStream( const char * fil_r, const char * fnc_r, int lne_r )
        {
          _mybuf.tagSet( fil_r, fnc_r, lne_r );
          return _mystream;
        }

      private:
        Loglinebuf   _mybuf;
        std::ostream _mystream;
      };
      ///////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : LogControlImpl
      //
      /** LogControl implementation (Singleton). */
      struct LogControlImpl
      {
      public:
        void excessive( bool onOff_r )
        { _excessive = onOff_r; }

        void setLineWriter( const shared_ptr<LogControl::LineWriter> & writer_r )
        { _lineWriter = writer_r; }

      private:
        std::ostream _no_stream;
        bool         _excessive;

        shared_ptr<LogControl::LineWriter> _lineWriter;

      public:
        /** Provide the stream to write (logger interface) */
        std::ostream & getStream( const std::string & group_r,
                                  LogLevel            level_r,
                                  const char *        file_r,
                                  const char *        func_r,
                                  const int           line_r )
        {
          if ( level_r == E_XXX && !_excessive )
            {
              return _no_stream;
            }

          if ( !_streamtable[group_r][level_r] )
            {
              _streamtable[group_r][level_r].reset( new Loglinestream( group_r, level_r ) );
            }
          return _streamtable[group_r][level_r]->getStream( file_r, func_r, line_r );
        }

        /** Write out formated line from Loglinebuf. */
        void writeLine( const std::string & formated_r )
        {
          if ( _lineWriter )
            _lineWriter->writeOut( formated_r );
        }

      private:
        typedef shared_ptr<Loglinestream>        StreamPtr;
        typedef std::map<LogLevel,StreamPtr>     StreamSet;
        typedef std::map<std::string,StreamSet>  StreamTable;
        /** one streambuffer per group and level */
        StreamTable _streamtable;

      private:
        /** Singleton */
        LogControlImpl()
        : _no_stream( 0 )
        , _excessive( getenv("ZYPP_FULLLOG") )
        , _lineWriter( getenv("ZYPP_NOLOG") ? NULL : new StdErrWriter )
        {}

      public:
        /** The LogControlImpl singleton
         * \note As most dtors log, it is inportant that the
         * LogControlImpl instance is the last static variable
         * destructed. At least destucted after all statics
         * which log from their dtor.
        */
        static LogControlImpl instance;
      };
      ///////////////////////////////////////////////////////////////////

      // 'THE' LogControlImpl singleton
      LogControlImpl LogControlImpl::instance;

      ///////////////////////////////////////////////////////////////////

      /** \relates LogControl::Impl Stream output */
      inline std::ostream & operator<<( std::ostream & str, const LogControlImpl & obj )
      {
        return str << "LogControlImpl";
      }

      ///////////////////////////////////////////////////////////////////
      //
      // Access from logger::
      //
      ///////////////////////////////////////////////////////////////////

      /** That's what logger:: calls.  */
      std::ostream & getStream( const char * group_r,
                                LogLevel     level_r,
                                const char * file_r,
                                const char * func_r,
                                const int    line_r )
      {
        return LogControlImpl::instance.getStream( group_r,
                                                   level_r,
                                                   file_r,
                                                   func_r,
                                                   line_r );
      }

      /** That's what Loglinebuf calls.  */
      inline void logFormat( const std::string & group_r, LogLevel level_r,
                             const char * file_r, const char * func_r, int line_r,
                             const std::string & buffer_r )
      {
        LogControlImpl::instance.writeLine( str::form( "<%d> [%s] %s(%s):%d %s",
                                                       level_r, group_r.c_str(),
                                                       file_r, func_r, line_r,
                                                       buffer_r.c_str() ) );
      }

      /////////////////////////////////////////////////////////////////
    } // namespace logger
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : LogControl
    //  Forward to LogControlImpl singleton.
    //
    ///////////////////////////////////////////////////////////////////

    using logger::LogControlImpl;

    void LogControl::logfile( const Pathname & logfile_r )
    { LogControlImpl::instance.setLineWriter( shared_ptr<LineWriter>( new logger::FileWriter(logfile_r) ) ); }

    void LogControl::setLineWriter( const shared_ptr<LineWriter> & writer_r )
    { LogControlImpl::instance.setLineWriter( writer_r ); }

    void LogControl::logNothing()
    { LogControlImpl::instance.setLineWriter( shared_ptr<LineWriter>() ); }

    void LogControl::logToStdErr()
    { LogControlImpl::instance.setLineWriter( shared_ptr<LineWriter>( new logger::StdErrWriter ) ); }

    ///////////////////////////////////////////////////////////////////
    //
    // LogControl::TmpExcessive
    //
    ///////////////////////////////////////////////////////////////////
    LogControl::TmpExcessive::TmpExcessive()
    { LogControlImpl::instance.excessive( true ); }
    LogControl::TmpExcessive::~TmpExcessive()
    { LogControlImpl::instance.excessive( false );  }

    /******************************************************************
     **
     **	FUNCTION NAME : operator<<
     **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const LogControl & obj )
    {
      return str << LogControlImpl::instance;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
