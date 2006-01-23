/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ResPool.h
 *
*/
#ifndef ZYPP_RESPOOL_H
#define ZYPP_RESPOOL_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ResPool
  //
  /** */
  class ResPool
  {
    friend std::ostream & operator<<( std::ostream & str, const ResPool & obj );

  public:
    /** Implementation  */
    class Impl;

  public:
    /** Default ctor */
    ResPool();
    /** Dtor */
    ~ResPool();

  public:

  private:
    /** Pointer to implementation */
    RWCOW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates ResPool Stream output */
  std::ostream & operator<<( std::ostream & str, const ResPool & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESPOOL_H
