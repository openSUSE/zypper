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
#include "zypp/RelCompare.h"

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

  public:
    /** Comparison mostly for std::container */
    static int compare( const NVR & lhs, const NVR & rhs )
    {
      int res = lhs.name.compare( rhs.name );
      if ( res )
        return res;
      return lhs.edition.compare( rhs.edition );
    }
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates NVR Stream output */
  std::ostream & operator<<( std::ostream & str, const NVR & obj );

  /** \relates NVR */
  inline bool operator==( const NVR & lhs, const NVR & rhs )
  { return compareByRel( Rel::EQ, lhs, rhs ); }

  /** \relates NVR */
  inline bool operator!=( const NVR & lhs, const NVR & rhs )
  { return compareByRel( Rel::NE, lhs, rhs ); }

  /** \relates NVR Order in std::container */
  inline bool operator<( const NVR & lhs, const NVR & rhs )
  { return compareByRel( Rel::LT, lhs, rhs ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_NVR_H
