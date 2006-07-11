/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SourceCache.h
 *
*/
#ifndef ZYPP_SourceCache_H
#define ZYPP_SourceCache_H

#include <iosfwd>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/source/SourceInfo.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/Pathname.h"
#include "zypp/cache/sqlite3x/sqlite3x.hpp"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace cache
  { /////////////////////////////////////////////////////////////////

    DEFINE_PTR_TYPE(SourceCache);

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : SourceCache
    //
    class SourceCache : public base::ReferenceCounted, private base::NonCopyable
    {
      friend std::ostream & operator<<( std::ostream & str, const SourceCache & obj );

    public:
      /** Creates a source cache */
      SourceCache( const Pathname &root_r, const std::string alias );
      ~SourceCache();

      void cachePattern( const data::Pattern pattern );
      void cachePackage( const data::Package package );
    protected:
      void cacheResolvable( const data::ResObject );
      /** Overload to realize stream output. */
      virtual std::ostream & dumpOn( std::ostream & str ) const;
      //typedef std::map<media::MediaNr, media::MediaAccessId> MediaMap
      shared_ptr<sqlite3x::sqlite3_connection> _con;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates SourceCache Stream output */
    inline std::ostream & operator<<( std::ostream & str, const SourceCache & obj )
    { return obj.dumpOn( str ); }


    /////////////////////////////////////////////////////////////////
  } // namespace cache
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_SourceCache_H
