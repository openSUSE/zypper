/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/SrcPackageImplIf.h
 *
*/
#ifndef ZYPP_DETAIL_SRCPACKAGEIMPLIF_H
#define ZYPP_DETAIL_SRCPACKAGEIMPLIF_H

#include <set>

#include "zypp/detail/ResObjectImplIf.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class SrcPackage;

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : SrcPackageImplIf
    //
    /** Abstact SrcPackage implementation interface.
    */
    class SrcPackageImplIf : public ResObjectImplIf
    {
    public:
      typedef SrcPackage ResType;

    public:
      /** */
      virtual Pathname location() const PURE_VIRTUAL;
      /** */
      virtual unsigned mediaId() const PURE_VIRTUAL;

    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_SRCPACKAGEIMPLIF_H
