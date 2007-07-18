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
    namespace memory
    { /////////////////////////////////////////////////////////////////

      class RepoImpl : public repo::RepositoryImpl
      {
      public:
        typedef intrusive_ptr<RepoImpl>       Ptr;
        typedef intrusive_ptr<const RepoImpl> constPtr;

      public:
        /** Default ctor */
        RepoImpl( const RepoInfo &repoinfo);
        /** Dtor */
        ~RepoImpl();

      public:

        void createResolvables();
        void createPatchAndDeltas();
      protected:
      };
      ///////////////////////////////////////////////////////////////////

      /////////////////////////////////////////////////////////////////
    } // namespace memory
    ///////////////////////////////////////////////////////////////////

    using memory::RepoImpl;

    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif

