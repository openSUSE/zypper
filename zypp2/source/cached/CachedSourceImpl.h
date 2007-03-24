/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_CachedSourceImpl_H
#define ZYPP_CachedSourceImpl_H

#include <iosfwd>

#include "zypp/source/SourceImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace cached
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : CachedSourceImpl
      //
      /** */
      class CachedSourceImpl : public source::SourceImpl
      {
      public:
        typedef intrusive_ptr<CachedSourceImpl>       Ptr;
        typedef intrusive_ptr<const CachedSourceImpl> constPtr;

      public:
        /** Default ctor */
        CachedSourceImpl();
        /** Dtor */
        ~CachedSourceImpl();

      public:
        /** String identifying the type of the source. */
	static std::string typeString()
	{ return "CachedSource"; }

        /** String identifying the type of the source. */
        virtual std::string type() const
        { return typeString(); }

      private:
        /** Ctor substitute.
         * Actually get the metadata.
         * \throw EXCEPTION on fail
        */
        virtual void factoryInit();
        virtual void createResolvables(Source_Ref source_r);
        
      };
      ///////////////////////////////////////////////////////////////////

      /////////////////////////////////////////////////////////////////
    } // namespace cached
    ///////////////////////////////////////////////////////////////////

    using cached::CachedSourceImpl;

    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_PLAINDIR_PLAINDIRIMPL_H
