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
       * Sets a \ref MediaVerifierRef verifier for given media number
       */
      void setVerifier( unsigned media_nr, media::MediaVerifierRef verifier );
      
      /**
       * provide a file from a media location.
       */
      Pathname provideFile( const OnMediaLocation & on_media_file );

      Pathname provideFile(const Pathname & file, unsigned media_nr = 1 );
      Pathname provideFile(const Pathname & file, unsigned media_nr, const FileChecker checker );
      
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
