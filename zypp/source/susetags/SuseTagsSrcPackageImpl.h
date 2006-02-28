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

        /** \name Rpm Package Attributes. */
        //@{
        /** */
        virtual Pathname location() const;
        /** */
        virtual ByteCount archivesize() const;
        /** */
        virtual DiskUsage diskusage() const;
        /** */
	virtual unsigned mediaId() const;

      private:
        Source_Ref _source;
        ByteCount _archivesize;
        unsigned int _media_number;
        Pathname _location;
        DiskUsage _diskusage;
      public:
        Source_Ref source() const;

/*
=Grp: System/Base
=Lic: GPL
=Src: 3ddiag 0.724 3 src
=Tim: 1111489970
=Loc: 1 3ddiag-0.724-3.i586.rpm

*/

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
