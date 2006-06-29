/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/KnownSourcesCache.h
 *
*/
#ifndef ZYPP_KnownSourcesCache_H
#define ZYPP_KnownSourcesCache_H

#include <iosfwd>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/source/SourceInfo.h"
#include "zypp/Pathname.h"
#include "zypp/cache/sqlite3x/sqlite3x.hpp"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace cache
  { /////////////////////////////////////////////////////////////////

    DEFINE_PTR_TYPE(KnownSourcesCache);

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : KnownSourcesCache
    //
    class KnownSourcesCache : public base::ReferenceCounted, private base::NonCopyable
    {
      friend std::ostream & operator<<( std::ostream & str, const KnownSourcesCache & obj );

    public:
      /** root path */
      KnownSourcesCache( const Pathname &root_r );
      ~KnownSourcesCache();
      source::SourceInfoList knownSources() const;
      void storeSource( const source::SourceInfo &info );    
    protected:
			bool tablesCreated() const;
			void createTables();
			void importOldSources();
      /** Overload to realize stream output. */
      virtual std::ostream & dumpOn( std::ostream & str ) const;
      //typedef std::map<media::MediaNr, media::MediaAccessId> MediaMap
			shared_ptr<sqlite3x::sqlite3_connection> _con;
			Pathname _root;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates KnownSourcesCache Stream output */
    inline std::ostream & operator<<( std::ostream & str, const KnownSourcesCache & obj )
    { return obj.dumpOn( str ); }


    /////////////////////////////////////////////////////////////////
  } // namespace cache
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_KnownSourcesCache_H
