/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/NVR.h
 *
*/
#ifndef ZYPP_NVR_H
#define ZYPP_NVR_H

#include <iosfwd>
#include <string>

#include "zypp/Edition.h"
#include "zypp/ResTraits.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : NVR
  //
  /** Helper storing Name and Edition. */
  struct NVR
  {
    /** Default ctor */
    NVR()
    {}

    /** Ctor */
    explicit
    NVR( const std::string & name_r,
         const Edition & edition_r = Edition() )
    : name( name_r )
    , edition( edition_r )
    {}

    /** Ctor from Resolvable::constPtr */
    explicit
    NVR( ResTraits<Resolvable>::constPtrType res_r );

    /**  */
    std::string name;
    /**  */
    Edition edition;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates NVR Stream output */
  std::ostream & operator<<( std::ostream & str, const NVR & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_NVR_H
