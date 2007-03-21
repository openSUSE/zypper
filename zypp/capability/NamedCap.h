/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/capability/NamedCap.h
 *
*/
#ifndef ZYPP_CAPABILITY_NAMEDCAP_H
#define ZYPP_CAPABILITY_NAMEDCAP_H

#include "zypp/capability/CapabilityImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

    DEFINE_PTR_TYPE(NamedCap)
    
    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : NamedCap
    //
    /** A \c name and optional Edition::MatchRange.
     * To provide an Edition::MatchRange create a \ref VersionedCap.
    */
    class NamedCap : public CapabilityImpl
    {
    public:
      typedef NamedCap Self;
      typedef NamedCap_Ptr       Ptr;
      typedef NamedCap_constPtr  constPtr;

      /** Ctor */
      NamedCap( const Resolvable::Kind & refers_r, const std::string & name_r )
      : CapabilityImpl( refers_r )
      , _name( name_r )
      {}

    public:
      /**  */
      virtual const Kind & kind() const;

      /** Return whether the Capabilities match. */
      virtual CapMatch matches( const CapabilityImpl::constPtr & rhs ) const;

      /** Name. */
      virtual std::string encode() const;
      
      /**  */
      const std::string & name() const
      { return _name; }
      
    protected:
      
      /**  Rel::ANY. */
      virtual const Edition::MatchRange & range() const;

    private:
      /**  */
      std::string _name;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPABILITY_NAMEDCAP_H
