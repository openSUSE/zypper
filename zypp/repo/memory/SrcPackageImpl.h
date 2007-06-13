/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repository/memory/SrcPackageImpl.h
 *
*/
#ifndef ZYPP_SOURCE_MEMORYSRCPackageIMPL_H
#define ZYPP_SOURCE_MEMORYSRCPackageIMPL_H

#include "zypp/detail/SrcPackageImplIf.h"
#include "zypp/DiskUsage.h"
#include "zypp/data/ResolvableData.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////
    namespace memory
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : SrcPackageImpl
      //
      /**
      */
      struct SrcPackageImpl : public zypp::detail::SrcPackageImplIf
      {
        SrcPackageImpl(data::SrcPackage_Ptr ptr);
        virtual ~SrcPackageImpl();

        /** */
        virtual Pathname location() const;
        /** */
        virtual ByteCount archivesize() const;
        /** */
        virtual DiskUsage diskusage() const;
        /** */
        virtual unsigned sourceMediaNr() const;

private:
        ByteCount _archivesize;
        unsigned _media_number;
        Pathname _location;
        DiskUsage _diskusage;      
      };
      ///////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////
    } // namespace memory
    /////////////////////////////////////////////////////////////////
  } // namespace repository
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_MEMORY_SRCPACKAGEIMPL_H
