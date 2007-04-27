/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_CachedRepositoryImpl_H
#define ZYPP_CachedRepositoryImpl_H

#include <iosfwd>
#include "zypp/Pathname.h"
#include "zypp2/repository/RepositoryImpl.h"
#include "zypp/ResStore.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repository
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace cached
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : CachedRepositoryImpl
      //
      /** */
      class CachedRepositoryImpl : public repository::RepositoryImpl
      {
      public:
        typedef intrusive_ptr<CachedRepositoryImpl>       Ptr;
        typedef intrusive_ptr<const CachedRepositoryImpl> constPtr;

      public:
        /** Default ctor */
        CachedRepositoryImpl( const zypp::Pathname &dbdir );
        /** Dtor */
        ~CachedRepositoryImpl();

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
        virtual void createResolvables();
        
        zypp::Pathname _dbdir;
        ResStore _store;
      };
      ///////////////////////////////////////////////////////////////////

      /////////////////////////////////////////////////////////////////
    } // namespace cached
    ///////////////////////////////////////////////////////////////////

    using cached::CachedRepositoryImpl;

    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_PLAINDIR_PLAINDIRIMPL_H

