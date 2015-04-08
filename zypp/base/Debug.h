/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/Debug.h
 *
 * Debuging tools which should not be used in released code.
*/
#ifndef ZYPP_NDEBUG
#warning ZYPP_BASE_DEBUG_H included
#ifndef ZYPP_BASE_DEBUG_H
#define ZYPP_BASE_DEBUG_H

#include <iosfwd>
//#include <sstream>
//#include <string>
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/ExternalProgram.h"
#include "zypp/base/ProvideNumericId.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace debug
  { /////////////////////////////////////////////////////////////////

    /** \defgroup DEBUG Debug tools
    */

#define TAG INT << __PRETTY_FUNCTION__ << std::endl

    /** 'ps v' */
    inline std::ostream & dumpMemOn( std::ostream & str, const std::string & msg = std::string() )
    {
      static std::string mypid( str::numstring( getpid() ) );
      const char* argv[] =
      {
        "ps",
        "v",
        mypid.c_str(),
        NULL
      };
      ExternalProgram prog(argv,ExternalProgram::Discard_Stderr, false, -1, true);

      str << "MEMUSAGE " << msg << std::endl;
      for( std::string line = prog.receiveLine(); ! line.empty(); line = prog.receiveLine() )
          str << line;

      prog.close();
      return str;
    }

    ///////////////////////////////////////////////////////////////////
    /** \defgroup DBG_TRACER Tracer
     * \ingroup DEBUG
    */
    //@{
    /** Base for a simple tracer. Provides an enum indicating which
     * traced functions were called.
    */
    struct TraceCADBase
    {
      enum What { CTOR, COPYCTOR, ASSIGN, DTOR, PING };
      std::string _ident;
    };

    /** \relates TraceCADBase Stream output of TraceCADBase::What. */
    inline std::ostream & operator<<( std::ostream & str, TraceCADBase::What obj )
    {
      switch( obj )
        {
        case TraceCADBase::CTOR:     return str << "CTOR";
        case TraceCADBase::COPYCTOR: return str << "COPYCTOR";
        case TraceCADBase::ASSIGN:   return str << "ASSIGN";
        case TraceCADBase::DTOR:     return str << "DTOR";
        case TraceCADBase::PING:     return str << "PING";
        }
      return str;
    }

    /** A simple tracer for (copy) Construction, Assignment, and
     * Destruction. To trace class Foo, derive public from
     * TraceCAD<Foo>. This tracer simply calls traceCAD in each
     * traced method, and traceCAD simply drops a line in the log.
     *
     * This tracer logs construction, copy construction, assignment,
     * destruction and _PING.
     *
     * assignment: In case the traced class defines an operator=
     * it must be altered to call TraceCAD::operator=, otherwise it
     * won't be triggered.
     *
     * _PING: Completely up to you. Call _PING somewhere in the traced
     * class to indicate something. In case you overload traceCAD, do
     * whatever is appropriate on _PING. It's just an offer to perform
     * logging or actions here, and not in the traced code.
     *
     * But traceCAD may be overloaded to produce more stats.
     *
     * \see \c Example.COW_debug.cc.
     */
    template<class _Tp>
      struct TraceCAD : public base::ProvideNumericId<TraceCAD<_Tp>, unsigned long>
                      , public TraceCADBase
      {
        static unsigned long & _totalTraceCAD()
        { static unsigned long _val = 0;
          return _val; }

        TraceCAD()
        { _ident = __PRETTY_FUNCTION__;
          ++_totalTraceCAD();
          traceCAD( CTOR, *this, *this ); }

        TraceCAD( const TraceCAD & rhs )
        { ++_totalTraceCAD();
          traceCAD( COPYCTOR, *this, rhs ); }

        TraceCAD & operator=( const TraceCAD & rhs )
        { traceCAD( ASSIGN, *this, rhs ); return *this; }

        virtual ~TraceCAD()
        { --_totalTraceCAD();
          traceCAD( DTOR, *this, *this ); }

        void _PING() const
        { traceCAD( PING, *this, *this ); }
      };

    /** \relates TraceCAD Stream output. */
    template<class _Tp>
      inline std::ostream & operator<<( std::ostream & str, const TraceCAD<_Tp> & obj )
      { return str << "(ID " << obj.numericId() << ", TOTAL " << obj._totalTraceCAD()
                   << ") [" << &obj << "] "; }

    /** Drop a log line about the traced method. Overload to
     * fit your needs.
    */
    template<class _Tp>
      void traceCAD( TraceCADBase::What what_r,
                     const TraceCAD<_Tp> & self_r,
                     const TraceCAD<_Tp> & rhs_r )
      {
        switch( what_r )
          {
          case TraceCADBase::CTOR:
          case TraceCADBase::PING:
          case TraceCADBase::DTOR:
            _DBG("DEBUG") << what_r << self_r << " (" << self_r._ident << ")" << std::endl;
            break;

          case TraceCADBase::COPYCTOR:
          case TraceCADBase::ASSIGN:
            _DBG("DEBUG") << what_r << self_r << "( " << rhs_r << ")" << " (" << self_r._ident << ")" << std::endl;
            break;
          }
      }
    //@}
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace debug
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_DEBUG_H
#endif // ZYPP_NDEBUG
