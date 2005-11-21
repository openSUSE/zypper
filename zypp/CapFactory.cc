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

  typedef zypp::capability::CapabilityImplPtr CapabilityImplPtr;

  /** \todo check set ordering to assert no dups */
  struct USetOrder : public std::binary_function<CapabilityImplPtr, CapabilityImplPtr, bool>
  {
    bool operator()( const CapabilityImplPtr & lhs,
                     const CapabilityImplPtr & rhs ) const
    { return lhs->asString() < rhs->asString(); }
  };

  typedef std::set<CapabilityImplPtr,USetOrder> USet;

  USet _uset;

  /////////////////////////////////////////////////////////////////
} // namespace
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : CapFactoryImpl
    //
    /** CapFactory implementation */
    struct CapFactoryImpl : public base::ReferenceCounted, private base::NonCopyable
    {
      /** Default ctor*/
      CapFactoryImpl();
      /** Dtor */
      ~CapFactoryImpl();
    };
    ///////////////////////////////////////////////////////////////////
    IMPL_PTR_TYPE(CapFactoryImpl)
    ///////////////////////////////////////////////////////////////////

    /** \relates CapFactoryImpl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const CapFactoryImpl & obj )
    {
      return str << "CapFactoryImpl";
    }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
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
  //	METHOD NAME : CapFactory::CapFactory
  //	METHOD TYPE : Ctor
  //
  CapFactory::CapFactory( detail::CapFactoryImplPtr impl_r )
  : _pimpl( impl_r )
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
  //	METHOD NAME : CapFactory::sayFriend
  //	METHOD TYPE : detail::constCapFactoryImplPtr
  //
  detail::constCapFactoryImplPtr CapFactory::sayFriend() const
  { return _pimpl; }

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
    CapabilityImplPtr newcap( new capability::NamedCap( defaultRefers_r, strval_r ) );
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
    return str << *obj.sayFriend();
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
