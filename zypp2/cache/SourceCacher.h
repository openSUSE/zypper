/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SourceCacher.h
 *
*/
#ifndef ZYPP_SourceCacher_H
#define ZYPP_SourceCacher_H

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

    DEFINE_PTR_TYPE(SourceCacher);

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : SourceCacher
    //
    class SourceCacher : public base::ReferenceCounted, private base::NonCopyable
    {
      friend std::ostream & operator<<( std::ostream & str, const SourceCacher & obj );

    public:
      /** root path */
      SourceCacher( const Pathname &root_r );
      ~SourceCacher();
    protected:

      /** Overload to realize stream output. */
      virtual std::ostream & dumpOn( std::ostream & str ) const;
      //typedef std::map<media::MediaNr, media::MediaAccessId> MediaMap
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates SourceCacher Stream output */
    inline std::ostream & operator<<( std::ostream & str, const SourceCacher & obj )
    { return obj.dumpOn( str ); }


    /////////////////////////////////////////////////////////////////
  } // namespace cache
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_SourceCacher_H
