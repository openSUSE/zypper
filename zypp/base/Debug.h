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
#warning ZYPP_BASE_DEBUG_H included
#ifndef ZYPP_BASE_DEBUG_H
#define ZYPP_BASE_DEBUG_H

#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/base/PtrTypes.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace debug
  { /////////////////////////////////////////////////////////////////

#define TAG INT << __PRETTY_FUNCTION__ << std::endl

    ///////////////////////////////////////////////////////////////////
    struct TraceCADBase
    {
      enum What { CTOR, COPYCTOR, ASSIGN, DTOR, PING };
    };

    inline std::ostream & operator<<( std::ostream & str, const TraceCADBase & obj )
    { return str << "TraceCAD[" << &obj << "] "; }

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

    template<class _Tp>
      struct TraceCAD : public TraceCADBase
      {
        TraceCAD()
        { traceCAD( CTOR, *this, *this ); }

        TraceCAD( const TraceCAD & rhs )
        { traceCAD( COPYCTOR, *this, rhs ); }

        TraceCAD & operator=( const TraceCAD & rhs )
        { traceCAD( ASSIGN, *this, rhs ); return *this; }

        virtual ~TraceCAD()
        { traceCAD( DTOR, *this, *this ); }

        void _PING() const
        { traceCAD( PING, *this, *this ); }
      };

    template<class _Tp>
      void traceCAD( TraceCADBase::What what_r,
                     const TraceCAD<_Tp> & self_r,
                     const TraceCAD<_Tp> & rhs_r )
      {
        switch( what_r )
          {
          case TraceCADBase::CTOR:
          case TraceCADBase::DTOR:
          case TraceCADBase::PING:
            INT << self_r << what_r << std::endl;
            break;
          case TraceCADBase::COPYCTOR:
          case TraceCADBase::ASSIGN:
            INT << self_r << what_r << "( " << rhs_r << ")" << std::endl;
            break;
          }
      }

    /////////////////////////////////////////////////////////////////
  } // namespace debug
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_DEBUG_H
