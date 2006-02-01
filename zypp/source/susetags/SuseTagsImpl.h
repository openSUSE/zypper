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

        typedef intrusive_ptr<SuseTagsImpl>      Ptr;

        /** \deprecated Interim ctor for testing
         * \throw EXCEPTION on parse error
        */
        SuseTagsImpl( const Pathname & localDir_r );
        /** Factory ctor */
        SuseTagsImpl( media::MediaAccess::Ptr & media_r, const Pathname & path_r = "/", const std::string & alias_r = "");
        /** Dtor */
        ~SuseTagsImpl();

	virtual void createResolvables(Source_Ref source_r);
	
	Pathname sourceDir( const NVRAD& nvrad );

      public:
        /** Stream output. */
        virtual std::ostream & dumpOn( std::ostream & str ) const;

      public:

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
