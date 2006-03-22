/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/MediaSet.h
 *
*/
#ifndef ZYPP_SOURCE_MEDIASET_H
#define ZYPP_SOURCE_MEDIASET_H

#include <iosfwd>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/Source.h"

#include "zypp/media/MediaManager.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////

    DEFINE_PTR_TYPE(MediaSet);

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : MediaSet
    //
    class MediaSet : public base::ReferenceCounted, private base::NonCopyable
    {
      friend std::ostream & operator<<( std::ostream & str, const MediaSet & obj );

    public:
      MediaSet(const Source_Ref & source_r);

      ~MediaSet();

      /** Get the media access ID to specified media */
      media::MediaAccessId getMediaAccessId (media::MediaNr medianr, bool no_attach = false);
      /** Redirect specified media to a new MediaId */
      void redirect (media::MediaNr medianr, media::MediaAccessId media_id);
      /**
       * Reattach the source if it is not mounted, but downloaded,
       * to different directory
       *
       * \throws Exception
       */
      void reattach(const Pathname &attach_point);
      /** Reset the handles to the medias */
      void reset();
      /**
       * Release all medias in the set
       */
      void release();

    protected:

      /** Overload to realize stream output. */
      virtual std::ostream & dumpOn( std::ostream & str ) const;

      typedef std::map<media::MediaNr, media::MediaAccessId> MediaMap;
      /** Mapping between each CD and Media Access ID */
      MediaMap medias;
      /** Refference to the source */
      Source_Ref _source;

      /** Rewrite the URL according to media number */
      Url rewriteUrl (const Url & url_r, const media::MediaNr medianr);

    };
    ///////////////////////////////////////////////////////////////////

    /** \relates MediaSet Stream output */
    inline std::ostream & operator<<( std::ostream & str, const MediaSet & obj )
    { return obj.dumpOn( str ); }


    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_MEDIASET_H
