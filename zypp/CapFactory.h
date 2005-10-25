/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/CapFactory.h
 *
*/
#ifndef ZYPP_CAPFACTORY_H
#define ZYPP_CAPFACTORY_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"

#include "zypp/Capability.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////
    DEFINE_PTR_TYPE(CapFactoryImpl)
    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : CapFactory
  //
  /** */
  class CapFactory
  {
  public:
    /** Default ctor */
    CapFactory();
    /** Factory ctor */
    explicit
    CapFactory( detail::CapFactoryImplPtr impl_r );
    /** Dtor */
    ~CapFactory();

  public:
    /**  */
    Capability parse( const std::string & strval_r ) const;

  private:
    /** Pointer to implementation */
    detail::CapFactoryImplPtr _pimpl;
  public:
    /** Avoid a bunch of friend decl. */
    detail::constCapFactoryImplPtr sayFriend() const;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates CapFactory Stream output */
  extern std::ostream & operator<<( std::ostream & str, const CapFactory & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPFACTORY_H
