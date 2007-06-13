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
#include <utility>
#include "zypp/Arch.h"
#include "zypp/Rel.h"
#include "zypp/Pathname.h"
#include "zypp/data/RecordId.h"
#include "zypp/repo/RepositoryImpl.h"
#include "zypp/ResStore.h"
#include "zypp/cache/sqlite3x/sqlite3x.hpp"
#include "zypp/cache/CacheTypes.h"
#include "zypp/cache/ResolvableQuery.h"
#include "zypp/RepoInfo.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace cached
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : RepoImpl
      //
      /** */
      class RepoImpl : public repo::RepositoryImpl
      {
      public:
        typedef intrusive_ptr<RepoImpl>       Ptr;
        typedef intrusive_ptr<const RepoImpl> constPtr;

      public:
        /** Default ctor */
        RepoImpl( const RepoInfo &repoinfo, const Pathname &dbdir, const data::RecordId &repository_id );
        /** Dtor */
        ~RepoImpl();
        void factoryInit();
      public:
        
        cache::ResolvableQuery resolvableQuery();
        void createResolvables();
        void createPatchAndDeltas();
      protected:
        void read_capabilities( sqlite3x::sqlite3_connection &con, std::map<data::RecordId, std::pair<Resolvable::Kind, NVRAD> > &nvras );
        Pathname _dbdir;
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

