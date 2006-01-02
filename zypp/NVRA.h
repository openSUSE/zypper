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
#include <string>

#include "zypp/base/PtrTypes.h"
#include "zypp/Edition.h"
#include "zypp/Arch.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : NVRA
  //
  /** */
  struct NVRA
  {
    /** Default ctor */
    NVRA()
    {}

    /** Ctor */
    explicit
    NVRA( const std::string & name_r,
          const Edition & edition_r = Edition(),
          const Arch & arch_r = Arch() )
    : name( name_r )
    , edition( edition_r )
    , arch( arch_r )
    {}

    /**  */
    std::string name;
    /**  */
    Edition edition;
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
