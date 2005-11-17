/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/capability/CapabilityImpl.h
 *
*/
#ifndef ZYPP_CAPABILITY_CAPABILITYIMPL_H
#define ZYPP_CAPABILITY_CAPABILITYIMPL_H

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"

#include "zypp/Resolvable.h"
#include "zypp/SolverContextFwd.h"
#include "zypp/ResKind.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////
    DEFINE_PTR_TYPE(CapabilityImpl)

    /** \todo Check implementation */
    typedef std::string CapKind;

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : CapabilityImpl
    //
    /** */
    class CapabilityImpl : public base::ReferenceCounted, private base::NonCopyable
    {
    public:
      /** Ctor */
      CapabilityImpl( const ResKind & refers_r );

    public:
      /**  */
      virtual const CapKind & kind() const = 0;
      /**  */
      const ResKind & refers() const
      { return _refers; }
      /**  */
      virtual std::string asString() const = 0;
      /**  */
      virtual bool matches( Resolvable::constPtr resolvable_r,
                            const SolverContext & colverContext_r ) const = 0;

    private:
      /**  */
      ResKind _refers;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates CapabilityImpl Stream output */
    extern std::ostream & operator<<( std::ostream & str, const CapabilityImpl & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPABILITY_CAPABILITYIMPL_H
