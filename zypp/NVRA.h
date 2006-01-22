/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/NVRA.h
 *
*/
#ifndef ZYPP_NVRA_H
#define ZYPP_NVRA_H

#include <iosfwd>

#include "zypp/NVR.h"
#include "zypp/Arch.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : NVRA
  //
  /**  Helper storing Name, Edition and Arch. */
  struct NVRA : public NVR
  {
    /** Default ctor */
    NVRA()
    {}

    /** Ctor */
    explicit
    NVRA( const std::string & name_r,
          const Edition & edition_r = Edition(),
          const Arch & arch_r = Arch() )
    : NVR( name_r, edition_r )
    , arch( arch_r )
    {}

    /** Ctor */
    explicit
    NVRA( const NVR & nvr_r,
          const Arch & arch_r = Arch() )
    : NVR( nvr_r )
    , arch( arch_r )
    {}

    /** Ctor from Resolvable::constPtr */
    explicit
    NVRA( ResTraits<Resolvable>::constPtrType res_r );

    /**  */
    Arch arch;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates NVRA Stream output */
  std::ostream & operator<<( std::ostream & str, const NVRA & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_NVRA_H
