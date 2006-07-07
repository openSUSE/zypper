/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_MediaSetAccess_H
#define ZYPP_MediaSetAccess_H

#include <iosfwd>
#include <string>
#include <vector>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/media/MediaManager.h"
#include "zypp/Pathname.h"
#include "zypp/CheckSum.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

    DEFINE_PTR_TYPE(MediaSetAccess);


    class MediaVerifier : public zypp::media::MediaVerifierBase
    {
      public:
      /** ctor */
      MediaVerifier(const std::string & vendor_r, const std::string & id_r, const media::MediaNr media_nr = 1);
      /**
       * Check if the specified attached media contains
       * the desired media number (e.g. SLES10 CD1).
       */
      virtual bool isDesiredMedia(const media::MediaAccessRef &ref);
      private:
        std::string _media_vendor;
        std::string _media_id;
        media::MediaNr _media_nr;
    };

    class OnMediaLocation
    {
      public:
      OnMediaLocation()
      {};
      private:
      
    };

    typedef boost::function<bool ( const Pathname &file )> FileChecker;

    class NullFileChecker
    {
      public:
      bool operator()( const Pathname &file )
      { return true; }
    };

    class ChecksumFileChecker
    {
      public:
      ChecksumFileChecker( const CheckSum &checksum );
      bool operator()( const Pathname &file );
      private:
      CheckSum _checksum;
    };

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : SourceCache
    //
    class MediaSetAccess : public base::ReferenceCounted, private base::NonCopyable
    {
      friend std::ostream & operator<<( std::ostream & str, const MediaSetAccess & obj );

    public:
      /**
       * creates a callback enabled media access  for \param url and \param path.
       * with only  media no verified
       */
      MediaSetAccess( const Url &url, const Pathname &path );
      ~MediaSetAccess();
      /**
       * the media change callbacks depend on the verifiers given for each media.
       */
      void setVerifiers( const std::vector<media::MediaVerifierRef> &verifiers );
      const Pathname provideFile(const Pathname & file, const unsigned media_nr = 1 );
      const Pathname provideFile(const Pathname & file, const unsigned media_nr, const FileChecker checker );
    protected:
      const Pathname provideFileInternal(const Pathname & file, const unsigned media_nr, bool checkonly, bool cached);
      Url rewriteUrl (const Url & url_r, const media::MediaNr medianr);
      media::MediaAccessId getMediaAccessId (media::MediaNr medianr);
      virtual std::ostream & dumpOn( std::ostream & str ) const;
    private:
      Url _url;
      Pathname _path;
      std::vector<media::MediaVerifierRef> _verifiers;
      typedef std::map<media::MediaNr, media::MediaAccessId> MediaMap;
      /** Mapping between each CD and Media Access ID */
      MediaMap medias;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates MediaSetAccess Stream output */
    inline std::ostream & operator<<( std::ostream & str, const MediaSetAccess & obj )
    { return obj.dumpOn( str ); }


} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_MediaSetAccess_H
