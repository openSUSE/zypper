/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/CapFactory.cc
 *
*/
#include <iostream>
#include <functional>
#include <set>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/CapFactory.h"

#include "zypp/capability/Capabilities.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace
{ /////////////////////////////////////////////////////////////////

  typedef zypp::capability::CapabilityImpl_Ptr CapabilityImpl_Ptr;

  /** \todo check set ordering to assert no dups */
  struct USetOrder : public std::binary_function<CapabilityImpl_Ptr, CapabilityImpl_Ptr, bool>
  {
    bool operator()( const CapabilityImpl_Ptr & lhs,
                     const CapabilityImpl_Ptr & rhs ) const
    { return lhs->asString() < rhs->asString(); }
  };

  typedef std::set<CapabilityImpl_Ptr,USetOrder> USet;

  USet _uset;

  /////////////////////////////////////////////////////////////////
} // namespace
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : CapFactoryImpl
  //
  /** CapFactory implementation (UNUSED) */
  struct CapFactory::Impl
  {};
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : CapFactory
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::CapFactory
  //	METHOD TYPE : Ctor
  //
  CapFactory::CapFactory()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::~CapFactory
  //	METHOD TYPE : Dtor
  //
  CapFactory::~CapFactory()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::parse
  //	METHOD TYPE : Capability
  //
  Capability CapFactory::parse( const std::string & strval_r ) const
  {
    return parse( strval_r, Resolvable::Kind() );
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::parse
  //	METHOD TYPE : Capability
  //
  /** \todo fix it */
  Capability CapFactory::parse( const std::string & strval_r,
                                const Resolvable::Kind & defaultRefers_r ) const
  {
    // (defaultRefers_r==Resolvable::Kind()) ==> throw on
    // missing Resolvable::Kind in strval
    // fix it!
    if ( strval_r.empty() )
      throw "no Resolvable::Kind";
    CapabilityImpl_Ptr newcap( new capability::NamedCap( defaultRefers_r, strval_r ) );
    USet::iterator in( _uset.insert( newcap ).first );
    return Capability( *in );
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::parse
  //	METHOD TYPE : Capability
  //
  /** \todo fix it */
  Capability CapFactory::parse( const Resolvable::Kind & refers_r,
                                const std::string & name_r,
                                Rel op_r,
                                const Edition & edition_r ) const
  {
    CapabilityImpl_Ptr newcap( new capability::NamedCap( refers_r, name_r ) );
    USet::iterator in( _uset.insert( newcap ).first );
    return Capability( *in );
  }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const CapFactory & obj )
  {
    return str << "No CapFactory stats implemented";
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
