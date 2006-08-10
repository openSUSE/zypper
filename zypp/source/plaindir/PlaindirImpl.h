/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/plaindir/PlaindirImpl.h
 *
*/
#ifndef ZYPP_SOURCE_PLAINDIR_PLAINDIRIMPL_H
#define ZYPP_SOURCE_PLAINDIR_PLAINDIRIMPL_H

#include <iosfwd>

#include <zypp/target/rpm/RpmHeader.h>
#include <zypp/target/rpm/RpmDb.h>

#include "zypp/source/SourceImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace plaindir
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : PlaindirImpl
      //
      /** */
      class PlaindirImpl : public SourceImpl
      {
      public:
        typedef intrusive_ptr<PlaindirImpl>       Ptr;
        typedef intrusive_ptr<const PlaindirImpl> constPtr;

      public:
        /** Default ctor */
        PlaindirImpl();
        /** Dtor */
        ~PlaindirImpl();

      public:
        /** String identifying the type of the source. */
	static std::string typeString()
	{ return "Plaindir"; }

        /** String identifying the type of the source. */
        virtual std::string type() const
        { return typeString(); }

        virtual void createResolvables(Source_Ref source_r);
        
        
      private:
        /** Ctor substitute.
         * Actually get the metadata.
         * \throw EXCEPTION on fail
        */
        virtual void factoryInit();
        int extract_packages_from_directory (ResStore & store, const Pathname & path, Source_Ref source, bool recursive);

      };
      ///////////////////////////////////////////////////////////////////

      /////////////////////////////////////////////////////////////////
    } // namespace plaindir
    ///////////////////////////////////////////////////////////////////

    using plaindir::PlaindirImpl;

    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_PLAINDIR_PLAINDIRIMPL_H
