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
#include "zypp/base/Exception.h"
#include "zypp/base/String.h"

#include "zypp/CapFactory.h"

#include "zypp/capability/Capabilities.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace
{ /////////////////////////////////////////////////////////////////

  typedef zypp::capability::CapabilityImpl_Ptr CapabilityImpl_Ptr;

  /** \ref USet order.
   * \todo check set ordering to assert no dups
  */
  struct USetOrder : public std::binary_function<CapabilityImpl_Ptr, CapabilityImpl_Ptr, bool>
  {
    /** */
    bool operator()( const CapabilityImpl_Ptr & lhs,
                     const CapabilityImpl_Ptr & rhs ) const
    { return lhs->asString() < rhs->asString(); }
  };

  /** */
  typedef std::set<CapabilityImpl_Ptr,USetOrder> USet;

  /** Set to unify created capabilities. */
  USet _uset;

  /*
   NoCap: empty string

   NamedCap: name; string not starting with '/'

   FileCap: filename; string starting with '/'

   VersionedCap: name op edition;



   filename
   name op edition
  */


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
  {
    template<typename _Tp>
      static bool tryMake( const std::string & str_r, _Tp & ret_r )
      {
        try
          {
            ret_r = _Tp( str_r );
          }
        catch (...)
          {
            return false;
          }
        return true;
      }

  };
  ///////////////////////////////////////////////////////////////////

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

#if 0
  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::parse
  //	METHOD TYPE : Capability
  //
  Capability CapFactory::parse( const std::string & strval_r ) const
  {
    return parse( strval_r, Resolvable::Kind() );
  }
#endif

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::parse
  //	METHOD TYPE : Capability
  //
  /** \todo fix it */
  Capability CapFactory::parse( const Resolvable::Kind & refers_r,
                                const std::string & strval_r ) const
  {
    // (defaultRefers_r==Resolvable::Kind()) ==> throw on
    // missing Resolvable::Kind in strval
    // fix it!

    if ( strval_r.empty() )
      throw "no Resolvable::Kind";
    CapabilityImpl_Ptr newcap( new capability::NamedCap( refers_r, strval_r ) );
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
    string fake = name_r + " " + op_r.asString() + " " + edition_r.asString();
    CapabilityImpl_Ptr newcap( new capability::NamedCap( refers_r, fake ) );
    USet::iterator in( _uset.insert( newcap ).first );
    return Capability( *in );
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::parse
  //	METHOD TYPE : Capability
  //
  Capability CapFactory::parse( const Resolvable::Kind & refers_r,
                                const std::string & name_r,
                                const std::string & op_r,
                                const std::string & edition_r )
  {
    string fake = name_r + " " + op_r + " " + edition_r;
    CapabilityImpl_Ptr newcap( new capability::NamedCap( refers_r, fake ) );
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
