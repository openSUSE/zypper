/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaAccess.h
 *
*/
#ifndef ZYPP_MEDIA_MEDIAACCESS_H
#define ZYPP_MEDIA_MEDIAACCESS_H

#include <iosfwd>
#include <map>
#include <list>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/APIConfig.h"

#include "zypp/Pathname.h"
#include "zypp/PathInfo.h"

#include "zypp/media/MediaException.h"
#include "zypp/media/MediaSource.h"

#include "zypp/Url.h"

namespace zypp {
  namespace media {

    class MediaHandler;

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : MediaAccess
    /**
     * @short Handle access to a medium
     *
     * The concrete @ref MediaHandler for a certain url is created
     * on @ref open and deleted on @close.
     *
     * The inteface here basically checks whether the handler exists,
     * then forwards the request to @ref MediaHandler.
     **/
    class MediaAccess : public base::ReferenceCounted, private base::NonCopyable
    {
    public:
	typedef intrusive_ptr<MediaAccess> Ptr;
	typedef intrusive_ptr<const MediaAccess> constPtr;

    private:

	static const Pathname _noPath;

	/**
	 * handler for 'physical' media
	 * == 0 if not open
	 **/
	MediaHandler * _handler;

    	friend class MediaManager;
    	friend class MediaManager_Impl;

	AttachedMedia        attachedMedia() const;

	bool                 isSharedMedia() const;

	void                 resetParentId();
	bool                 dependsOnParent() const;

	bool                 dependsOnParent(MediaAccessId parentId,
	                                     bool exactIdMatch) const;
    public:

       /**
        * constructor
        **/
	MediaAccess();

	/**
	 * open url. If preferred_attach_point is given,
	 * try to use it as attach point.
	 *
	 * <b>Caution:</b> The medium can choose a different attach point.
	 * Only getAttachPoint() knows the real attach point.
	 *
	 * \throws MediaException
	 *
	 **/
	void open( const Url& url, const Pathname & preferred_attach_point = "" );

	/**
	 * True if media is open.
	 **/
	bool isOpen() const { return( _handler != 0 ); }

	/**
	 * Hint if files are downloaded or not.
	 * @return True, if the files are downloaded.
	 */
	bool        downloads() const;

	/**
	 * Used Protocol if media is opened, otherwise 'unknown'.
	 **/
        std::string protocol() const;

	/**
	 * Url if media is opened, otherwise empty.
	 **/
        Url url() const;

	/**
	 * close url
	 *
	 * \throws MediaException
	 *
	 **/
	void close();

    public:

	/**
	 * Use concrete handler to attach the media.
	 *
	 * @param next try next available device in turn until end of device
	 * list is reached (for media which are accessible through multiple
	 * devices like cdroms).
	 *
	 * \throws MediaException
	 *
	 **/
	void attach(bool next = false);

	/**
	 * True if media is attached.
	 *
	 * \throws MediaException
	 *
	 **/
	bool isAttached() const;

        bool hasMoreDevices() const;

        /**
         * Fill in a vector of detected ejectable devices and the index of the
         * currently attached device within the vector. The contents of the vector
         * are the device names (/dev/cdrom and such).
         *
         * \param devices  vector to load with the device names
         * \param index    index of the currently used device in the devices vector
         */
        virtual void
        getDetectedDevices(std::vector<std::string> & devices,
                           unsigned int & index) const;


	/**
	 * Return the local directory that corresponds to medias url,
	 * no matter if media isAttached or not. Files requested will
	 * be available at 'localRoot() + filename' or better
	 * 'localPath( filename )'.
	 *
	 * If media is not open an empty pathname is returned.
	 **/
	Pathname localRoot() const;

	/**
	 * Short for 'localRoot() + pathname', but returns an empty
	 * pathname if media is not open.
	 *
	 * Files provided will be available at 'localPath(filename)'.
	 **/
	Pathname localPath( const Pathname & pathname ) const;

        /**
          Use concrete handler to disconnect the media.

          This is useful for media which e.g. holds open a connection to a
          server like FTP. After calling disconnect() the media object still is
          valid and files are present.

          After calling disconnect() it's not possible to call provideFile() or
          provideDir() anymore.
	 *
	 * \throws MediaException
	 *
        */
        void disconnect();

        /**
         * Use concrete handler to release the media.
         * @param ejectDev Device to eject. None if empty.
         *
         * \throws MediaException
         *
         **/
        void release( const std::string & ejectDev = "" );

	/**
	 * Use concrete handler to provide file denoted by path below
	 * 'attach point'. Filename is interpreted relative to the
	 * attached url and a path prefix is preserved.
         *
         * @param cached  If cached is set to true, the function checks, if
         *                the file already exists and doesn't download it again
         *                if it does. Currently only the existence is checked,
         *                no other file attributes.
	 * @param checkonly If this and 'cached' are set to true only the
	 *                  existence of the file is checked but it's not
	 *                  downloaded. If 'cached' is unset an errer is
	 *                  returned always.
	 *
	 * \throws MediaException
	 *
	 **/
	void provideFile( const Pathname & filename ) const;

	/**
	 * Remove filename below attach point IFF handler downloads files
	 * to the local filesystem. Never remove anything from media.
	 *
	 * \throws MediaException
	 *
	 **/
	void releaseFile( const Pathname & filename ) const;

	/**
	 * Use concrete handler to provide directory denoted
	 * by path below 'attach point' (not recursive!).
	 * 'dirname' is interpreted relative to the
	 * attached url and a path prefix is preserved.
	 *
	 * \throws MediaException
	 *
	 **/
	void provideDir( const Pathname & dirname ) const;

	/**
	 * Use concrete handler to provide directory tree denoted
	 * by path below 'attach point' (recursive!!).
	 * 'dirname' is interpreted relative to the
	 * attached url and a path prefix is preserved.
	 *
	 * \throws MediaException
	 *
	 **/
	void provideDirTree( const Pathname & dirname ) const;

	/**
	 * Remove directory tree below attach point IFF handler downloads files
	 * to the local filesystem. Never remove anything from media.
	 *
	 * \throws MediaException
	 *
	 **/
	void releaseDir( const Pathname & dirname ) const;

	/**
	 * Remove pathname below attach point IFF handler downloads files
	 * to the local filesystem. Never remove anything from media.
	 *
	 * If pathname denotes a directory it is recursively removed.
	 * If pathname is empty or '/' everything below the attachpoint
	 * is recursively removed.
	 *
	 * \throws MediaException
	 *
	 **/
	void releasePath( const Pathname & pathname ) const;

	/**
	 * set a deltafile to be used in the next download
	 */
	void setDeltafile( const Pathname & filename ) const;

    public:

	/**
	 * Return content of directory on media via retlist. If dots is false
	 * entries starting with '.' are not reported.
	 *
	 * The request is forwarded to the concrete handler,
	 * which may atempt to retieve the content e.g. via 'readdir'
	 *
	 * <B>Caution:</B> This is not supported by all media types.
	 * Be prepared to handle E_not_supported_by_media.
	 *
	 * \throws MediaException
	 *
	 **/
        void dirInfo( std::list<std::string> & retlist,
			 const Pathname & dirname, bool dots = true ) const;

	/**
	 * Basically the same as dirInfo above. The content is returned as
	 * filesystem::DirContent, which includes name and filetype of each directory
	 * entry. Retrieving the filetype usg. requires an additional ::stat call for
	 * each entry, thus it's more expensive than a simple readdir.
	 *
	 * <B>Caution:</B> This is not supported by all media types.
	 * Be prepared to handle E_not_supported_by_media.
	 *
	 * \throws MediaException
	 *
	 **/
	void dirInfo( filesystem::DirContent & retlist,
                      const Pathname & dirname, bool dots = true ) const;

        /**
         * check if a file exists
         *
         * Asserted that url is a file and not a dir.
         *
         * \throws MediaException
         *
         **/
        bool doesFileExist( const Pathname & filename ) const;

	/**
	 * Destructor
	 **/
	virtual ~MediaAccess();

    public:

	virtual std::ostream & dumpOn( std::ostream & str ) const;

    public:
        /**
         * Get file from location at specified by URL and copy it to
         * destination.
         *
         * @param from Source URL
         * @param to   Destination file name
	 *
	 * \throws MediaException
	 *
         **/
        void getFile( const Url &from, const Pathname &to );

    public:

      /**
       * Helper class that provides file on construction
       * and cleans up on destruction.
       *
       * <b>Caution:</b> There's no synchronisation between multiple
       * FileProvider instances, that provide the same file from the
       * same media. If the first one goes out of scope, the file is
       * cleaned. It's just a convenience for 'access and forgett'.
       *
       * <b>Caution:</b> We should either store the reference MediaAccess'
       * MediaHandler here (for this MediaHandler must become a
       * ref counting pointer class), or we need more info from MediaHandler
       * (whether he's downloading to the local fs. If not, no releasefile
       * is necessary).
       * Currently we can not releaseFile after the media was closed
       * (it's passed to the handler, which is deleted on close).
       *
       * \throws MediaBadFilenameException
       * \throws MediaException
       **/
      class FileProvider {
	FileProvider( const FileProvider & );             // no copy
	FileProvider & operator=( const FileProvider & ); // no assign
	private:
	  MediaAccess::constPtr _media;
	  Pathname              _file;
	  Pathname		_local_file;
	public:
	  /**
           * \throws MediaException
           */
	  FileProvider( MediaAccess::constPtr media_r, const Pathname & file_r )
	    : _media( media_r )
	    , _file( file_r )
	    , _local_file( "" )
	  {
	    if ( _file.empty() ) {
	      ZYPP_THROW(MediaBadFilenameException(_file.asString()));
	    } else if ( _media ) {
	      try {
		_media->provideFile( _file );
		_local_file = _media->localPath( _file );
	      }
	      catch (const MediaException & excpt_r)
              {
		ZYPP_CAUGHT(excpt_r);
		_media = NULL;
		ZYPP_RETHROW(excpt_r);
	      }
	    }
	  }

	  ~FileProvider() {
	    if ( _media )
	    {
	      try {
		_media->releaseFile( _file );
	      }
	      catch (const MediaException &excpt_r)
	      {
		ZYPP_CAUGHT(excpt_r);
	      }
              catch(...) {} // No exception from dtor!
	    }
	  }

	public:

	  /**
	   * If no error, expect operator() to return the local
	   * Pathname of the provided file.
	   **/
	  Pathname localFile() const { return _local_file; }

	  /**
	   * Return the local Pathname of the provided file or
	   * an empty Pathname on error.
	   **/
	  Pathname operator()() const {
	    if ( _media )
	      return _media->localPath( _file );
	    return Pathname();
	  }
      };
    };

    std::ostream & operator<<( std::ostream & str, const MediaAccess & obj );

///////////////////////////////////////////////////////////////////

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_MEDIAACCESS_H

