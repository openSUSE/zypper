/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp2/repository/memory/DSrcPackageImpl.h
 *
*/
#ifndef ZYPP_SOURCE_MEMORYSRCPackageIMPL_H
#define ZYPP_SOURCE_MEMORYSRCPackageIMPL_H

#include "zypp/detail/SrcPackageImplIf.h"
#include "zypp/Source.h"
#include "zypp/DiskUsage.h"
#include "zypp/data/ResolvableData.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repository
  { /////////////////////////////////////////////////////////////////
    namespace memory
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : SrcPackageImpl
      //
      /**
      */
      struct DSrcPackageImpl : public zypp::detail::SrcPackageImplIf
      {
        DSrcPackageImpl(data::SrcPackage_Ptr ptr);
        virtual ~DSrcPackageImpl();

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
public:
        Source_Ref source() const;
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
