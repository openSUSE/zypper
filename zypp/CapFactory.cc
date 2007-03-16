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
#include <map>

#include <ext/hash_set>
#include <ext/hash_fun.h>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/base/String.h"
#include "zypp/base/Counter.h"

#include "zypp/CapFactory.h"
#include "zypp/capability/Capabilities.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace
{ /////////////////////////////////////////////////////////////////
  using ::zypp::Resolvable;
  using ::zypp::capability::CapabilityImpl;
  using ::zypp::capability::CapImplOrder;

  struct CapImplHashFun
  {
    size_t operator() ( const CapabilityImpl::Ptr & p ) const
    {
      return __gnu_cxx::hash<const char*>()( p->encode().c_str() );
    }
  };

  struct CapImplHashEqual
  {
    bool operator() ( const CapabilityImpl::Ptr & lhs, const CapabilityImpl::Ptr & rhs ) const
    {
      return (    lhs->encode() == rhs->encode()
               && lhs->kind()   == rhs->kind()
               && lhs->refers() == rhs->refers() );
    }
  };

  /** Set of unique CapabilityImpl. */
  //typedef std::set<CapabilityImpl::Ptr,CapImplOrder> USet;
  typedef __gnu_cxx::hash_set<CapabilityImpl::Ptr, CapImplHashFun, CapImplHashEqual> USet;


  /** Set to unify created capabilities.
   *
   * This is to unify capabilities. Each CapabilityImpl created
   * by CapFactory, must be inserted into _uset, and the returned
   * CapabilityImpl::Ptr has to be uset to create the Capability.
  */
  USet _uset;

  /** Each CapabilityImpl created in CapFactory \b must be wrapped.
   *
   * Immediately wrap \a allocated_r, and unified by inserting it into
   * \c _uset. Each CapabilityImpl created by CapFactory, \b must be
   * inserted into _uset, by calling usetInsert.
   *
   * \return CapabilityImpl_Ptr referencing \a allocated_r (or an
   * eqal representation, allocated is deleted then).
  */
  CapabilityImpl::Ptr usetInsert( CapabilityImpl::Ptr cap_ptr )
  {
    return *(_uset.insert( cap_ptr ).first);
  }

  /** Collect USet statistics.
   * \ingroup DEBUG
  */
  struct USetStatsCollect : public std::unary_function<CapabilityImpl::constPtr, void>
  {
    typedef ::zypp::Counter<unsigned> Counter;

    Counter _caps;
    std::map<CapabilityImpl::Kind,Counter> _capKind;
    std::map<Resolvable::Kind,Counter>     _capRefers;

    void operator()( const CapabilityImpl::constPtr & cap_r )
    {
      //DBG << *cap_r << endl;
      ++_caps;
      ++(_capKind[cap_r->kind()]);
      ++(_capRefers[cap_r->refers()]);
    }

    std::ostream & dumpOn( std::ostream & str ) const
    {
      str << "  Capabilities total: " << _caps << endl;
      str << "  Capability kinds:" << endl;
      for ( std::map<CapabilityImpl::Kind,Counter>::const_iterator it = _capKind.begin();
	    it != _capKind.end(); ++it )
	{
	  str << "    " << it->first << '\t' << it->second << endl;
	}
      str << "  Capability refers:" << endl;
      for ( std::map<Resolvable::Kind,Counter>::const_iterator it = _capRefers.begin();
	    it != _capRefers.end(); ++it )
	{
	  str << "    " << it->first << '\t' << it->second << endl;
	}
      return str;
    }
  };

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
  /** CapFactory implementation.
   *
   * Provides various functions doing checks and log and \c throw.
   * CapFactory::parse usually combines them, and if nothing fails,
   * finaly builds the Capability.
   *
   * \attention Each CapabilityImpl created by CapFactory, \b must
   * be inserted into ::_uset, by calling ::usetInsert, \b before
   * the Capability is created.
   *
   * \li \c file:     /absolute/path
   * \li \c split:    name:/absolute/path
   * \li \c name:     name
   * \li \c vers:     name op edition
   * \li \c hal:      hal(string)
   * \li \c modalias: modalias(string)
  */
  struct CapFactory::Impl
  {
 
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

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::parse
  //	METHOD TYPE : Capability
  //
  Capability CapFactory::parse( const Resolvable::Kind & refers_r,
				const std::string & strval_r ) const
  try
  {
    return Capability( usetInsert( ::zypp::capability::parse( refers_r, strval_r ) ) );
  }
  catch ( Exception & excpt )
  {
    ZYPP_RETHROW( excpt );
    return Capability(); // not reached
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::parse
  //	METHOD TYPE : Capability
  //
  Capability CapFactory::parse( const Resolvable::Kind & refers_r,
				const std::string & name_r,
				const std::string & op_r,
				const std::string & edition_r ) const
  try
  {
    return Capability( usetInsert(::zypp::capability::parse( refers_r, name_r, op_r, edition_r ) ) );
  }
  catch ( Exception & excpt )
  {
    ZYPP_RETHROW( excpt );
    return Capability(); // not reached
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::parse
  //	METHOD TYPE : Capability
  //
  Capability CapFactory::parse( const Resolvable::Kind & refers_r,
				const std::string & name_r,
				Rel op_r,
				const Edition & edition_r ) const
  try
    {
      return Capability( usetInsert(::zypp::capability::parse( refers_r, name_r, op_r, edition_r )) );
    }
  catch ( Exception & excpt )
    {
      ZYPP_RETHROW( excpt );
      return Capability(); // not reached
    }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::halEvalCap
  //	METHOD TYPE : Capability
  //
  Capability CapFactory::halEvalCap() const
  try
    {
      return Capability( usetInsert( ::zypp::capability::buildHal( Resolvable::Kind(), "hal()" ) ) );
    }
  catch ( Exception & excpt )
    {
      ZYPP_RETHROW( excpt );
      return Capability(); // not reached
    }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::modaliasEvalCap
  //	METHOD TYPE : Capability
  //
  Capability CapFactory::modaliasEvalCap() const
  try
    {
      return Capability( usetInsert( ::zypp::capability::buildModalias( Resolvable::Kind(), "modalias()" ) ) );
    }
  catch ( Exception & excpt )
    {
      ZYPP_RETHROW( excpt );
      return Capability(); // not reached
    }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::encode
  //	METHOD TYPE : std::string
  //
  std::string CapFactory::encode( const Capability & cap_r ) const
  {
    return cap_r._pimpl->encode();
  }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const CapFactory & obj )
  {
    str << "CapFactory stats:" << endl;

    return for_each( _uset.begin(), _uset.end(), USetStatsCollect() ).dumpOn( str );
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
