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
#include "zypp/DiskUsage.h"

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
    /** Abstract SrcPackage implementation interface.
    */
    class SrcPackageImplIf : public ResObjectImplIf
    {
    public:
      typedef SrcPackage ResType;

    public:
      /** Overloaded ResObjectImpl attribute.
       * \return The \ref location media number.
       */
      virtual unsigned mediaNr() const;

      /** Overloaded ResObjectImpl attribute.
       * \return The \ref location downloadSize.
       */
      virtual ByteCount downloadSize() const;

    public:
      /** */
      virtual OnMediaLocation location() const PURE_VIRTUAL;
      /** */
      virtual DiskUsage diskusage() const PURE_VIRTUAL;

    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_SRCPACKAGEIMPLIF_H
