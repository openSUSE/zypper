/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/susetags/SuseTagsImpl.h
 *
*/
#ifndef ZYPP_SOURCE_SUSETAGS_SUSETAGSIMPL_H
#define ZYPP_SOURCE_SUSETAGS_SUSETAGSIMPL_H

#include <iosfwd>
#include <string>

#include "zypp/Pathname.h"
#include "zypp/source/SourceImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : SuseTagsImpl
      //
      /** */
      class SuseTagsImpl : public SourceImpl
      {
      public:
        typedef intrusive_ptr<SuseTagsImpl>       Ptr;
        typedef intrusive_ptr<const SuseTagsImpl> constPtr;

      public:
        /** Factory ctor */
        SuseTagsImpl( media::MediaId & media_r, const Pathname & path_r = "/", const std::string & alias_r = "");
        /** Dtor */
        ~SuseTagsImpl();

	virtual void createResolvables(Source_Ref source_r);

	Pathname sourceDir( const NVRAD& nvrad );

      protected:
        /** Stream output. */
        virtual std::ostream & dumpOn( std::ostream & str ) const;
      };
      ///////////////////////////////////////////////////////////////////

      /////////////////////////////////////////////////////////////////
    } // namespace susetags
    ///////////////////////////////////////////////////////////////////

    using susetags::SuseTagsImpl;

    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_SUSETAGS_SUSETAGSIMPL_H
