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
  //
  //	CLASS NAME : CapFactory
  //
  /**
   * \todo define EXCEPTIONS
  */
  class CapFactory
  {
    friend std::ostream & operator<<( std::ostream & str, const CapFactory & obj );

  public:
    /** Default ctor */
    CapFactory();
    /** Dtor */
    ~CapFactory();

  public:
    /** Parse Capability from string (incl. Resolvable::Kind).
     * \a strval_r is expected to define a valid Capability \em including
     * the Resolvable::Kind.
     * \throw EXCEPTION on parse error.
    */
    Capability parse( const std::string & strval_r ) const;
    /** Parse Capability from string (providing default Resolvable::Kind).
     * \a strval_r is expected to define a valid Capability. If it does
     * not define the Resolvable::Kind, \a defaultRefers_r is used instead.
     * \throw EXCEPTION on parse error.
    */
    Capability parse( const std::string & strval_r, const Resolvable::Kind & defaultRefers_r ) const;

    /** Parse Capability providing Resolvable::Kind, name, RelOp and Edition.
     * \throw EXCEPTION on parse error.
    */
    Capability parse( const Resolvable::Kind & refers_r,
                      const std::string & name_r,
                      RelOp op_r,
                      const Edition & edition_r ) const;

  private:
    /** Implementation */
    struct Impl;
    /** Pointer to implementation */
    base::ImplPtr<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates CapFactory Stream output */
  extern std::ostream & operator<<( std::ostream & str, const CapFactory & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPFACTORY_H
