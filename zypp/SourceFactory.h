/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SourceFactory.h
 *
*/
#ifndef ZYPP_SOURCEFACTORY_H
#define ZYPP_SOURCEFACTORY_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"

#include "zypp/Source.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : SourceFactory
  //
  /** Factory to create a \ref Source.
   * Actually a Singleton
   *
  */
  class SourceFactory
  {
    friend std::ostream & operator<<( std::ostream & str, const SourceFactory & obj );

  public:
    /** Default ctor */
    SourceFactory();
    /** Dtor */
    ~SourceFactory();

  public:
    /** Construct source from an implementation.
     * \throw EXCEPTION on NULL \a impl_r
    */
    Source createFrom( const Source::Impl_Ptr & impl_r );

  private:
    /** Implementation  */
    class Impl;
    /** Pointer to implementation */
    RW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates SourceFactory Stream output */
  extern std::ostream & operator<<( std::ostream & str, const SourceFactory & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCEFACTORY_H
