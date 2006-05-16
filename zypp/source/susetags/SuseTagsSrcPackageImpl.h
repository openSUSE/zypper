/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/susetags/SrcPackageImpl.h
 *
*/
#ifndef ZYPP_SOURCE_SUSETAGSSRCPackageIMPL_H
#define ZYPP_SOURCE_SUSETAGSSRCPackageIMPL_H

#include "zypp/detail/SrcPackageImplIf.h"
#include "zypp/Source.h"
#include "zypp/DiskUsage.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : SrcPackageImpl
      //
      /**
      */
      struct SuseTagsSrcPackageImpl : public zypp::detail::SrcPackageImplIf
      {
        SuseTagsSrcPackageImpl(Source_Ref source_r);
        virtual ~SuseTagsSrcPackageImpl();

        /** */
        virtual Pathname location() const;
        /** */
        virtual ByteCount archivesize() const;
        /** */
        virtual DiskUsage diskusage() const;
        /** */
	virtual unsigned sourceMediaNr() const;

      private:
        Source_Ref _source;
        ByteCount _archivesize;
        unsigned _media_number;
        Pathname _location;
        DiskUsage _diskusage;
      public:
        Source_Ref source() const;
      };
      ///////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////
    } // namespace susetags
      /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_SUSETAGS_SRCPACKAGEIMPL_H
