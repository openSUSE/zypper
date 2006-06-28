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
#include "zypp/Pathname.h"

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
      /** root path */
      SourceCache( const Pathname &root_r );
      ~SourceCache();
      source::SourceInfoList knownSources() const;
      void storeSource( const source::SourceInfo &info );    
    protected:

      /** Overload to realize stream output. */
      virtual std::ostream & dumpOn( std::ostream & str ) const;
      //typedef std::map<media::MediaNr, media::MediaAccessId> MediaMap
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
