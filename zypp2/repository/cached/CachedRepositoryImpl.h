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
#include <map>
#include "zypp/Arch.h"
#include "zypp/Rel.h"
#include "zypp/Pathname.h"
#include "zypp/data/RecordId.h"
#include "zypp2/repository/RepositoryImpl.h"
#include "zypp/ResStore.h"

#include <sqlite3.h>
#include "zypp2/cache/sqlite3x/sqlite3x.hpp"

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

        const ResStore & resolvables() const
        { return _store; }
        
        virtual void createResolvables();
      private:
        /** Ctor substitute.
         * Actually get the metadata.
         * \throw EXCEPTION on fail
        */
        virtual void factoryInit();
        
        
        zypp::Pathname _dbdir;
        ResStore _store;
        
        Rel relationFor( const data::RecordId &id );
        Resolvable::Kind kindFor( const data::RecordId &id );
        Dep deptypeFor( const data::RecordId &id );
        Arch archFor( const data::RecordId &id );
        
        void read_capabilities( sqlite3x::sqlite3_connection &con, std::map<data::RecordId, NVRAD> &nvras );
        
        std::map<data::RecordId, Rel> _rel_cache;
        std::map<data::RecordId, Resolvable::Kind> _kind_cache;
        std::map<data::RecordId, std::string> _deptype_cache;
        std::map<data::RecordId, Arch> _arch_cache;
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

