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
#include <boost/function.hpp>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/media/MediaManager.h"
#include "zypp/Pathname.h"
#include "zypp/CheckSum.h"
#include "zypp/OnMediaLocation.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

    DEFINE_PTR_TYPE(MediaSetAccess);

    typedef boost::function<bool ( const Pathname &file )> FileChecker;

    class NullFileChecker
    {
      public:
      bool operator()( const Pathname &file );
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
    /**
     * The
     */
    class MediaSetAccess : public base::ReferenceCounted, private base::NonCopyable
    {
      friend std::ostream & operator<<( std::ostream & str, const MediaSetAccess & obj );

    public:
      /**
       * creates a callback enabled media access  for \param url and \param path.
       * with only 1 media no verified
       */
      MediaSetAccess( const Url &url, const Pathname &path );
      ~MediaSetAccess();

      /**
       * Sets a verifier for given media number
       */
      void setVerifier( unsigned media_nr, media::MediaVerifierRef verifier );
      
      /**
       * provide a file fom a multiple media
       */
      Pathname provideFile( const OnMediaLocation & on_media_file );

      /**
       * provides a file on multiple media which is possibly cached
       * The cached_file is provided and the Checksums are compared.
       * if they match, the cached one is copied to the destination directory
       * if not the file is provided and copied to the destination directory.
       */
      void providePossiblyCachedMetadataFile( const OnMediaLocation &file_on_media, const Pathname &destination, const Pathname &cached_file);

      Pathname provideFile(const Pathname & file, unsigned media_nr = 1 );
      Pathname provideFile(const Pathname & file, unsigned media_nr, const FileChecker checker );
      void providePossiblyCachedMetadataFile( const Pathname &file_to_download, unsigned medianr, const Pathname &destination, const Pathname &cached_file, const CheckSum &checksum );

    protected:
      Pathname provideFileInternal(const Pathname & file, unsigned media_nr, bool checkonly, bool cached);
      Url rewriteUrl (const Url & url_r, const media::MediaNr medianr);
      media::MediaAccessId getMediaAccessId (media::MediaNr medianr);
      virtual std::ostream & dumpOn( std::ostream & str ) const;
    private:
      Url _url;
      Pathname _path;
      typedef std::map<media::MediaNr, media::MediaAccessId> MediaMap;
      typedef std::map<media::MediaNr, media::MediaVerifierRef > VerifierMap;
      /** Mapping between each CD and Media Access ID */
      MediaMap _medias;
      VerifierMap _verifiers;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates MediaSetAccess Stream output */
    inline std::ostream & operator<<( std::ostream & str, const MediaSetAccess & obj )
    { return obj.dumpOn( str ); }


} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_MediaSetAccess_H
