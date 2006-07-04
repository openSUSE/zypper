/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_YUMSourceCacher_H
#define ZYPP_YUMSourceCacher_H

#include <iosfwd>
#include <string>

#include "zypp/cache/SourceCacher.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////

    namespace yum
    { /////////////////////////////////////////////////////////////////

    DEFINE_PTR_TYPE(YUMSourceCacher);

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : SourceCacher
    //
    class YUMSourceCacher : public cache::SourceCacher
    {
      friend std::ostream & operator<<( std::ostream & str, const YUMSourceCacher & obj );

    public:
      /** root path */
      YUMSourceCacher( const Pathname &root_r );
      ~YUMSourceCacher();
    protected:

      /** Overload to realize stream output. */
      virtual std::ostream & dumpOn( std::ostream & str ) const;
      //typedef std::map<media::MediaNr, media::MediaAccessId> MediaMap
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates SourceCacher Stream output */
    inline std::ostream & operator<<( std::ostream & str, const YUMSourceCacher & obj )
    { return obj.dumpOn( str ); }


    /////////////////////////////////////////////////////////////////
  } // namespace cache
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
}
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_SourceCacher_H
