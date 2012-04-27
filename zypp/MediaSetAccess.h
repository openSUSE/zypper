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
#include "zypp/base/Function.h"

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/Flags.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/media/MediaManager.h"
#include "zypp/Pathname.h"
#include "zypp/CheckSum.h"
#include "zypp/OnMediaLocation.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

    DEFINE_PTR_TYPE(MediaSetAccess);

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : MediaSetAccess
    //
    /**
     * Media access layer responsible for handling files distributed on a set
     * of media with media change and abort/retry/ingore user callback handling.
     *
     * This is provided as a means to handle CD or DVD sets accessible through
     * dir, iso, nfs or other URL schemes other than cd/dvd (see
     * \ref MediaManager for info on different implemented media backends).
     * Currently it handles URLs containing cdN, CDN, dvdN, and DVDN strings,
     * where N is the number of particular media in the set.
     *
     * Examples:
     * \code
     * "iso:/?iso=/path/to/iso/images/openSUSE-10.3-Alpha2plus-DVD-x86_64-DVD1.iso"
     * "dir:/path/to/cdset/sources/openSUSE-10.3/Alpha2plus/CD1"
     * \endcode
     *
     * MediaSetAccess accesses files on desired media by rewriting
     * the original URL, replacing the digit (usually) 1 with requested media
     * number and uses \ref MediaManager to get the files from the new URL.
     *
     * Additionaly, each media number can be assined a media verifier which
     * checks if the media we are trying to access is the desired one. See
     * \ref MediaVerifierBase for more info.
     *
     * Code example:
     * \code
     * Url url("dir:/path/to/cdset/sources/openSUSE-10.3/Alpha2plus/CD1");
     *
     * MediaSetAccess access(url);
     *
     * access.setVerifier(1, media1VerifierRef);
     * access.setVerifier(2, media2VerifierRef);
     *
     * Pathname file1 = "/some/file/on/media1";
     * Pathname providedfile1 = access.provideFile(file1, 1);
     * Pathname file2 = "/some/file/on/media2";
     * Pathname providedfile2 = access.provideFile(file1, 2);
     *
     * \endcode
     */
    class MediaSetAccess : public base::ReferenceCounted, private base::NonCopyable
    {
      friend std::ostream & operator<<( std::ostream & str, const MediaSetAccess & obj );

    public:
      /**
       * Creates a callback enabled media access for specified \a url.
       *
       * \param url
       * \param prefered_attach_point Prefered attach (mount) point. Use, if
       *        you want to mount the media to a specific directory.
       */
      MediaSetAccess( const Url &url, const Pathname & prefered_attach_point = "" );
      /** \overload Also taking a \ref label. */
      MediaSetAccess( const std::string & label_r, const Url &url, const Pathname & prefered_attach_point = "" );
      ~MediaSetAccess();

      /**
       * Sets a \ref MediaVerifier verifier for given media number.
       */
      void setVerifier( unsigned media_nr, media::MediaVerifierRef verifier );

      /**
       * The label identifing this media set and to be sent in a media change request.
       */
      const std::string & label() const
      { return _label; }

      /**
       * Set the label identifing this media set and to be sent in a media change request.
       */
      void setLabel( const std::string & label_r )
      { _label = label_r; }

      enum ProvideFileOption
      {
        /**
         * The user is not asked anything, and the error
         * exception is just propagated */
        PROVIDE_DEFAULT = 0x0,
        PROVIDE_NON_INTERACTIVE = 0x1
      };
      ZYPP_DECLARE_FLAGS(ProvideFileOptions,ProvideFileOption);

      /**
       * Provides a file from a media location.
       *
       * \param resource location of the file on media
       * \return local pathname of the requested file
       *
       * \throws MediaException if a problem occured and user has chosen to
       *         abort the operation. The calling code should take care
       *         to quit the current operation.
       * \throws SkipRequestException if a problem occured and user has chosen
       *         to skip the current operation. The calling code should continue
       *         with the next one, if possible.
       *
       *
       * If the resource is marked as optional, no Exception is thrown
       * and Pathname() is returned
       *
       * the optional deltafile argument describes a file that can
       * be used for delta download algorithms.
       *
       * \note interaction with the user does not ocurr if
       * \ref ProvideFileOptions::NON_INTERACTIVE is set.
       *
       * \note OnMediaLocation::optional() hint has no effect on the transfer.
       *
       * \see zypp::media::MediaManager::provideFile()
       */
      Pathname provideFile( const OnMediaLocation & resource, ProvideFileOptions options = PROVIDE_DEFAULT, const Pathname &deltafile = Pathname() );

      /**
       * Provides \a file from media \a media_nr.
       *
       * \param file path to the file relative to media URL
       * \param media_nr the media number in the media set
       * \return local pathname of the requested file
       *
       * \note interaction with the user does not ocurr if
       * \ref ProvideFileOptions::NON_INTERACTIVE is set.
       *
       * \note OnMediaLocation::optional() hint has no effect on the transfer.
       *
       * \throws MediaException if a problem occured and user has chosen to
       *         abort the operation. The calling code should take care
       *         to quit the current operation.
       * \throws SkipRequestException if a problem occured and user has chosen
       *         to skip the current operation. The calling code should continue
       *         with the next one, if possible.
       * \see zypp::media::MediaManager::provideFile()
       */
      Pathname provideFile(const Pathname & file, unsigned media_nr = 1, ProvideFileOptions options = PROVIDE_DEFAULT );

      /**
       * Release file from media.
       * This signal that file is not needed anymore.
       *
       * \param resource location of the file on media
       */
      void releaseFile( const OnMediaLocation &resource );


      /**
       * Release file from media.
       * This signal that file is not needed anymore.
       *
       * \param file path to the file relative to media URL
       * \param media_nr the media number in the media set
       */
      void releaseFile(const Pathname & file, unsigned media_nr = 1 );

      /**
       * Provides direcotry \a dir from media number \a media_nr.
       *
       * \param dir path to the directory relative to media URL
       * \param recursive whether to provide the whole directory subtree
       * \param media_nr the media number in the media set
       * \return local pathname of the requested directory
       *
       * \throws MediaException if a problem occured and user has chosen to
       *         abort the operation. The calling code should take care
       *         to quit the current operation.
       * \todo throw SkipRequestException if a problem occured and user has chosen
       *         to skip the current operation. The calling code should continue
       *         with the next one, if possible.
       * \see zypp::media::MediaManager::provideDir()
       * \see zypp::media::MediaManager::provideDirTree()
       */
      Pathname provideDir(const Pathname & dir, bool recursive, unsigned media_nr = 1, ProvideFileOptions options = PROVIDE_DEFAULT );

      /**
       * Checks if a file exists on the specified media, with user callbacks.
       *
       * \param file file to check
       * \param media_nr Media number
       *
       * \throws MediaException if a problem occured and user has chosen to
       *         abort the operation. The calling code should take care
       *         to quit the current operation.
       * \throws SkipRequestException if a problem occured and user has chosen
       *         to skip the current operation. The calling code should continue
       *         with the next one, if possible.
       * \see zypp::media::MediaManager::doesFileExist(MediaAccessId,const Pathname&)
       */
      bool doesFileExist(const Pathname & file, unsigned media_nr = 1 );

      /**
       * Fills \ref retlist with directory information.
       */
      void dirInfo( filesystem::DirContent &retlist, const Pathname &dirname,
                    bool dots = true, unsigned media_nr = 1 );

      /**
       * Release all attached media of this set.
       *
       * \throws MediaNotOpenException for invalid access IDs.
       */
      void release();

      /**
       * Replaces media number in specified url with given \a medianr.
       *
       * Media number in the URL is searched for with regex
       * <tt> "^(.*(cd|dvd))([0-9]+)(\\.iso)$" </tt> for iso scheme and
       * with <tt> "^(.*(cd|dvd))([0-9]+)(/?)$" </tt> for other schemes.
       *
       * For cd and dvd scheme it returns the original URL, as well as for
       * URL which do not match the above regexes.
       *
       * \param url_r   original URL
       * \param medianr requested media number
       * \return        rewritten URL if applicable, the original URL otherwise
       */
      static Url rewriteUrl (const Url & url_r, const media::MediaNr medianr);

    protected:
      /**
       * Provides the \a file from medium number \a media_nr and returns its
       * local path.
       *
       * \note   The method must not throw if \a checkonly is <tt>true</tt>.
       *
       * \throws MediaException \a checkonly is <tt>false</tt> and
       *         a problem occured and user has chosen to
       *         abort the operation. The calling code should take care
       *         to quit the current operation.
       * \throws SkipRequestException \a checkonly is <tt>false</tt> and
       *         a problem occured and user has chosen
       *         to skip the current operation. The calling code should continue
       *         with the next one, if possible.
       */
      Pathname provideFileInternal( const OnMediaLocation &resource, ProvideFileOptions options );

      typedef function<void( media::MediaAccessId, const Pathname & )> ProvideOperation;

      void provide( ProvideOperation op, const OnMediaLocation &resource, ProvideFileOptions options, const Pathname &deltafile );

      media::MediaAccessId getMediaAccessId (media::MediaNr medianr);
      virtual std::ostream & dumpOn( std::ostream & str ) const;

    private:
      /** Media or media set URL */
      Url _url;

      /**
       * Prefered mount point.
       *
       * \see MediaManager::open(Url,Pathname)
       * \see MediaHandler::_attachPoint
       */
      Pathname _prefAttachPoint;

      std::string _label;

      typedef std::map<media::MediaNr, media::MediaAccessId> MediaMap;
      typedef std::map<media::MediaNr, media::MediaVerifierRef > VerifierMap;

      /** Mapping between media number and Media Access ID */
      MediaMap _medias;
      /** Mapping between media number and corespondent verifier */
      VerifierMap _verifiers;
    };
    ///////////////////////////////////////////////////////////////////
    ZYPP_DECLARE_OPERATORS_FOR_FLAGS(MediaSetAccess::ProvideFileOptions);

    /** \relates MediaSetAccess Stream output */
    inline std::ostream & operator<<( std::ostream & str, const MediaSetAccess & obj )
    { return obj.dumpOn( str ); }


} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_MediaSetAccess_H
