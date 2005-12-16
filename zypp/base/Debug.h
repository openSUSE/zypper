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
#include <sstream>
#include "zypp/base/Logger.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/ResObject.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace debug
  { /////////////////////////////////////////////////////////////////

    /** \defgroup DEBUG Debug tools
    */

#define TAG INT << __PRETTY_FUNCTION__ << std::endl

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
    };

    /** \relates TraceCADBase Stream output. */
    inline std::ostream & operator<<( std::ostream & str, const TraceCADBase & obj )
    { return str << "TraceCAD[" << &obj << "] "; }

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

    /** A simple tracer. To trace class Foo, derive public from
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
    //@}
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    /** \defgroup DBG_FAKED_RESOLVABLES Faked Resolvables
     * \ingroup DEBUG
     * \code
     *   // parse or fill in the values:
     *   std::string  _name;
     *   Edition      _edition;
     *   Arch         _arch;
     *   Dependencies _deps;
     *   // Create a faked ResObject claiming to be a Package:
     *   ResObject::Ptr ptr( debug::fakeResObject<Package>( _name, _edition, _arch, Dependencies() ) );
     * \endcode
    */
    //@{
    /** Implementation of faked Resolvable. */
    class ResObjectFakeImpl : public detail::ResObjectImplIf
    {
      virtual Label summary() const
      {
          std::ostringstream str;
          str << "FAKED " << *self();
          return str.str();
      }
    };

    /** Faked Resolvable.
     * Template argument defines the kind of Resolvable.
    */
    template<class _Res>
      class ResObjectFake : public ResObject
      {
      public:
        ResObjectFake( const std::string & name_r,
                       const Edition & edition_r,
                       const Arch & arch_r )
        : ResObject( ResTraits<_Res>::kind, name_r, edition_r, arch_r )
        {}
      };

    /** Faked Resolvable factory function.
     * Provide ready to use NVRA and Dependencies.
    */
    template<class _Res>
      ResObject::Ptr fakeResObject( const std::string & name_r,
                                    const Edition & edition_r,
                                    const Arch & arch_r,
                                    const Dependencies & deps_r )
      {
        using detail::_resobjectfactory_detail::ResImplConnect;

        shared_ptr<ResObjectFakeImpl> impl( new ResObjectFakeImpl );
        ResObject::Ptr ret( new ResImplConnect<ResObjectFake<_Res> >
                            ( name_r, edition_r, arch_r, impl ) );
        ret->setDeps( deps_r );
        return ret;
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
