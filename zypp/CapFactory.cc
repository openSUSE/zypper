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

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/base/String.h"

#include "zypp/CapFactory.h"
#include "zypp/capability/Capabilities.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace
{ /////////////////////////////////////////////////////////////////

  using ::zypp::capability::CapabilityImpl;

  /** \ref USet order.
   * \todo check set ordering to assert no dups
  */
  struct USetOrder : public std::binary_function<CapabilityImpl::Ptr, CapabilityImpl::Ptr, bool>
  {
    /** */
    bool operator()( const CapabilityImpl::Ptr & lhs,
                     const CapabilityImpl::Ptr & rhs ) const
    { return lhs->asString() < rhs->asString(); }
  };

  /** */
  typedef std::set<CapabilityImpl::Ptr,USetOrder> USet;

  /** Set to unify created capabilities. */
  USet _uset;

  /** Immediately wrap \a allocated_r and unify by inserting
   * it into \c _uset.
   * \return CapabilityImpl_Ptr referencing \a allocated_r (or an
   * eqal representation, allocated is deleted then).
  */
  CapabilityImpl::Ptr usetInsert( CapabilityImpl * allocated_r )
  {
    return *(_uset.insert( CapabilityImpl::Ptr(allocated_r) ).first);
  }

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
   * \li \c file:    /absolute/path
   * \li \c split:   name:/absolute/path
   * \li \c name:    name
   * \li \c vers:    name op edition
  */
  struct CapFactory::Impl
  {
    /** Assert a valid Resolvable::Kind. */
    static void assertResKind( const Resolvable::Kind & refers_r )
    {
      if ( refers_r == Resolvable::Kind() )
        ZYPP_THROW( "Missing or empty  Resolvable::Kind in Capability." );
    }

    /** Test for a FileCap. \a name_r starts with \c /. */
    static bool isFileSpec( const std::string & name_r )
    {
      return *name_r.c_str() == '/';
    }

    /** Check whether \a op_r and \a edition_r are valid.
     *
     * \return Whether to build a VersionedCap (i.e. \a op_r
     * is not Rel::ANY.
    */
    static bool isEditionSpec( Rel op_r, const Edition & edition_r )
    {
      switch ( op_r.inSwitch() )
        {
        case Rel::ANY_e:
          if ( edition_r != Edition::noedition )
            WAR << "Operator " << op_r << " causes Edition "
            << edition_r << " to be ignored." << endl;
          return false;
          break;

        case Rel::NONE_e:
          ZYPP_THROW( "Operator NONE is not allowed in Capability " );
          break;

        case Rel::EQ_e:
        case Rel::NE_e:
        case Rel::LT_e:
        case Rel::LE_e:
        case Rel::GT_e:
        case Rel::GE_e:
          return true;
          break;
        }
      // SHOULD NOT GET HERE
      ZYPP_THROW( "Unknow Operator NONE is not allowed in Capability " );
      return false; // not reached
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
  Capability CapFactory::parse( const Resolvable::Kind & refers_r,
                                const std::string & strval_r ) const

  try
    {
      str::regex  rx( "(.*[^ \t])([ \t]+)([^ \t]+)([ \t]+)([^ \t]+)" );
      str::smatch what;
      if( str::regex_match( strval_r.begin(), strval_r.end(),what, rx ) )
        {
          // name op edition
          return parse( refers_r,
                        what[1].str(), what[3].str(), what[5].str() );
        }
      //else
      // not VersionedCap

      return parse( refers_r,
                    strval_r, Rel::ANY, Edition::noedition );
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
      // Try creating Rel and Edition, then parse
      return parse( refers_r, name_r, Rel(op_r), Edition(edition_r) );
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
      Impl::assertResKind( refers_r );

      if ( Impl::isEditionSpec( op_r, edition_r ) )
        {
          // build a VersionedCap
          return Capability
          ( usetInsert
            ( new capability::VersionedCap( refers_r, name_r, op_r, edition_r ) ) );
        }
      //else
      // op_r is Rel::ANY so build a NamedCap or FileCap

      if ( Impl::isFileSpec( name_r ) )
        {
          return Capability
          ( usetInsert
            ( new capability::FileCap( refers_r, name_r ) ) );
        }
      //else
      // build a NamedCap

      return Capability
      ( usetInsert
        ( new capability::NamedCap( refers_r, name_r ) ) );
    }
  catch ( Exception & excpt )
    {
      ZYPP_RETHROW( excpt );
      return Capability(); // not reached
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
