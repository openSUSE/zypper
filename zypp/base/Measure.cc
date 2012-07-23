/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/Measure.cc
 *
*/
extern "C"
{
#include <sys/times.h>
#include <unistd.h>
}
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/base/Measure.h"
#include "zypp/base/String.h"

using std::endl;

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "Measure"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace debug
  { /////////////////////////////////////////////////////////////////

      /** Times measured by \ref Measure. */
      struct Tm
      {
        Tm()
        : _real( 0 )
        , _proc( tmsEmpty )
        {}

        void get()
        {
          _real = ::time(NULL);
          ::times( &_proc );
        }

        Tm operator-( const Tm & rhs ) const
        {
          Tm ret( *this );
          ret._real -= rhs._real;
          ret._proc.tms_utime -= rhs._proc.tms_utime;
          ret._proc.tms_stime -= rhs._proc.tms_stime;
          ret._proc.tms_cutime -= rhs._proc.tms_cutime;
          ret._proc.tms_cstime -= rhs._proc.tms_cstime;
          return ret;
        }

        std::string asString() const
        {
          std::string ret( timeStr( _real ) );
          ret += " (u ";
          ret += timeStr( asSec( _proc.tms_utime ) );
          ret += " s ";
          ret += timeStr( asSec( _proc.tms_stime ) );
          ret += " c ";
          ret += timeStr( asSec( _proc.tms_cutime + _proc.tms_cstime ) );
          ret += ")";
          return ret;
        }

        std::string stringIf( clock_t ticks_r, const std::string & tag_r ) const
        {
          std::string ret;
          if ( ticks_r )
            {
              ret += tag_r;
              ret += timeStr( asSec( ticks_r ) );
            }
          return ret;
        }

        double asSec( clock_t ticks_r ) const
        { return double(ticks_r) / ticks; }

        std::string timeStr( time_t sec_r ) const
        {
          time_t h = sec_r/3600;
          sec_r -= h*3600;
          time_t m = sec_r/60;
          sec_r -= m*60;
          if ( h )
            return str::form( "%lu:%02lu:%02lu", h, m, sec_r );
          if ( m )
            return str::form( "%lu:%02lu", m, sec_r );
          return str::form( "%lu", sec_r );
        }

        std::string timeStr( double sec_r ) const
        {
          time_t h = time_t(sec_r)/3600;
          sec_r -= h*3600;
          time_t m = time_t(sec_r)/60;
          sec_r -= m*60;
          if ( h )
            return str::form( "%lu:%02lu:%05.2lf", h, m, sec_r );
          if ( m )
            return str::form( "%lu:%05.2lf", m, sec_r );
          return str::form( "%.2lf", sec_r );
        }

        /** Systems ticks per second. */
        static const long ticks;
        /** Empty struct tms. */
        static const struct tms tmsEmpty;
        /** Real time via \c ::time. */
        time_t      _real;
        /** Process times via \c ::times. */
        struct tms  _proc;
      };

      const struct tms Tm::tmsEmpty = { 0, 0, 0, 0 };
      const long Tm::ticks = sysconf(_SC_CLK_TCK);

      /** \refers Tm Stream output. */
      std::ostream & operator<<( std::ostream & str, const Tm & obj )
      {
        return str << obj.asString();
      }


    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Measure::Impl
    //
    /** Measure implementation. */
    class Measure::Impl
    {
    public:
      Impl( const std::string & ident_r )
      : _ident  ( ident_r )
      , _level  ( _glevel )
      , _seq    ( 0 )
      {
	_glevel += "..";
        INT << _level << "START MEASURE(" << _ident << ")" << endl;
        _start.get();
      }

      ~Impl()
      {
        _stop.get();
        ++_seq;
        std::ostream & str( INT << _level << "MEASURE(" << _ident << ") " );
        dumpMeasure( str );
	_glevel.erase( 0, 2 );
      }

      void restart()
      {
        INT << _level << "RESTART MEASURE(" << _ident << ")" << endl;
        _start = _stop;
      }

      void elapsed() const
      {
        _stop.get();
        ++_seq;
        std::ostream & str( INT << _level << "ELAPSED(" << _ident << ") " );
        dumpMeasure( str );
        _elapsed = _stop;
      }

    private:
      std::ostream & dumpMeasure( std::ostream & str_r ) const
      {
        str_r << ( _stop - _start );
        if ( _seq > 1 ) // diff to previous _elapsed
          {
            str_r << " [" << ( _stop - _elapsed ) << "]";
          }
        return str_r << endl;
      }

    private:
      static std::string _glevel;

      std::string       _ident;
      std::string       _level;
      Tm               _start;
      mutable unsigned _seq;
      mutable Tm       _elapsed;
      mutable Tm       _stop;
    };

    std::string Measure::Impl::_glevel;

    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Measure
    //
    ///////////////////////////////////////////////////////////////////

    Measure::Measure()
    {}

    Measure::Measure( const std::string & ident_r )
    : _pimpl( new Impl( ident_r ) )
    {}

    Measure::~Measure()
    {}

    void Measure::start( const std::string & ident_r )
    {
      stop();
      _pimpl.reset( new Impl( ident_r ) );
    }

    void Measure::restart()
    {
      _pimpl->restart();
    }

    void Measure::elapsed() const
    {
      if ( _pimpl )
        _pimpl->elapsed();
    }

    void Measure::stop()
    {
      _pimpl.reset();
    }

    /////////////////////////////////////////////////////////////////
  } // namespace debug
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
