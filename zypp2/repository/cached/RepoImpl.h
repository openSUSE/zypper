/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_RepoImpl_H
#define ZYPP_RepoImpl_H

#include <iosfwd>
#include <map>
#include "zypp/Arch.h"
#include "zypp/Rel.h"
#include "zypp/Pathname.h"
#include "zypp/data/RecordId.h"
#include "zypp2/repository/RepositoryImpl.h"
#include "zypp/ResStore.h"
#include "zypp2/cache/sqlite3x/sqlite3x.hpp"
#include "zypp2/cache/CacheTypes.h"
#include "zypp2/cache/ResolvableQuery.h"

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
      //	CLASS NAME : RepoImpl
      //
      /** */
      class RepoImpl : public repository::RepositoryImpl
      {
      public:
        typedef intrusive_ptr<RepoImpl>       Ptr;
        typedef intrusive_ptr<const RepoImpl> constPtr;

      public:
        /** Default ctor */
        RepoImpl( const zypp::Pathname &dbdir, const data::RecordId &repository_id );
        /** Dtor */
        ~RepoImpl();

      public:
        
        cache::ResolvableQuery resolvableQuery();
        
        /** String identifying the type of the source. */
	static std::string typeString()
	{ return "CachedSource"; }

        /** String identifying the type of the source. */
        virtual std::string type() const
        { return typeString(); }

        const ResStore & resolvables() const
        { return _store; }
        
        virtual void createResolvables();
      public:
        
        
        
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
        
        cache::CacheTypes _type_cache;
        data::RecordId _repository_id;
        
        cache::ResolvableQuery _rquery;
      };
      ///////////////////////////////////////////////////////////////////

      /////////////////////////////////////////////////////////////////
    } // namespace cached
    ///////////////////////////////////////////////////////////////////

    using cached::RepoImpl;

    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_PLAINDIR_PLAINDIRIMPL_H

