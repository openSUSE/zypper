
/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaHandler.h
 *
*/
#ifndef ZYPP_MEDIA_MEDIAHANDLERL_H
#define ZYPP_MEDIA_MEDIAHANDLERL_H

#include <iosfwd>
#include <string>
#include <list>

#include "zypp/Pathname.h"
#include "zypp/PathInfo.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/Url.h"

#include "zypp/media/MediaSource.h"
#include "zypp/media/MediaException.h"
#include "zypp/APIConfig.h"

namespace zypp {
  namespace media {


///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : MediaHandler
/**
 * @short Abstract base class for 'physical' MediaHandler like MediaCD, etc.
 *
 * Handles the requests forwarded by @ref MediaAccess. The public interface
 * contains nonvirtual methods, which should do common sanitychecks and
 * logging. For the real action they call virtual methods overloaded by the
 * concrete handler.
 **/
class MediaHandler {
    friend std::ostream & operator<<( std::ostream & str, const MediaHandler & obj );

    public:
	typedef shared_ptr<MediaHandler> Ptr;
	typedef shared_ptr<const MediaHandler> constPtr;

	static bool setAttachPrefix(const Pathname &attach_prefix);

	static std::string getRealPath(const std::string &path);
	static Pathname    getRealPath(const Pathname    &path);

    private:
        /**
	 * User defined default attach point prefix.
	 */
    	static Pathname _attachPrefix;

	/**
	 * The attached media source description reference.
	 */
	mutable
	MediaSourceRef  _mediaSource;

	/**
	 * This is where the media will be actually attached ("mounted").
	 * All files are provided bellow this + _relativeRoot directory.
	 **/
	AttachPointRef  _attachPoint;

	/**
	 * The user provided attach preferred point. It may contain
	 * following values:
	 *
	 *      "",  true  => create temporary attach point bellow of
	 *                    _attachPrefix or a built-in default and
	 *                    remove it if not needed any more.
	 *
	 *      dir, false => user specified attach point (not removed)
	 */
	AttachPoint     _AttachPointHint;

	/**
	 * The relative root directory of the data on the media.
	 * See also localRoot() and urlpath_below_attachpoint_r
	 * constructor argument.
	 */
	Pathname        _relativeRoot;

	/**
	 * True if concrete handler downloads files to the local
	 * filesystem. If true releaseFile/Dir will delete them.
	 **/
	bool            _does_download;

        /** timestamp of the the last attach verification */
        mutable time_t  _attach_mtime;

	/** file usable for delta downloads */
	mutable Pathname _deltafile;

    protected:
        /**
	 * Url to handle
	 **/
	const Url        _url;

	/**
	 * Access Id of media handler we depend on.
	 */
	MediaAccessId    _parentId;

        /**
	 * MediaAccess (MediaManager) needs access to the attachedMedia()
	 * function to deliver a shared media source and its attach point
	 * to the media manager and then to other media handler instances.
	 * Further, is needs to be able to forward the dependsOnParent()
	 * and resetParentId() functions to the media manager.
	 */
	friend class MediaAccess;

	/**
	 * Check if the current media handler depends on an
	 * another handler specified by media access id.
	 * \param parentId The id of the parent handler to check against.
	 * \return true if it depends, false if not.
	 */
	bool             dependsOnParent(MediaAccessId parentId,
	                                 bool exactIdMatch);
	bool             dependsOnParent();

	/**
	 * Called in case, where the media manager takes over the
	 * destruction of the parent id (e.g. while destruction
	 * of the media manager).
	 */
	void             resetParentId();

        /**
	 * Return the currently used attach point.
	 **/
	Pathname         attachPoint() const;

	/**
	 * Set a new attach point.
	 * \param path  The attach point directory path.
	 * \param temp  If to remove the attach point while cleanup.
	 */
	void             setAttachPoint(const Pathname &path, bool temp);

	/**
	 * Set a (shared) attach point.
	 * \param ref New attach point reference.
	 */
	void             setAttachPoint(const AttachPointRef &ref);

	/**
	 * Get the actual attach point hint.
	 */
	AttachPoint      attachPointHint() const;

	/**
	 * Set the attach point hint as specified by the user.
	 * \param path  The attach point directory path.
	 * \param temp  If to remove the attach point while cleanup.
	 */
	void             attachPointHint(const Pathname &path, bool temp);

	/**
	 * Try to create a default / temporary attach point.
	 * It trys to create it in attachPrefix if avaliable,
	 * then in built-in directories.
	 * \return The name of the new attach point or empty path name.
	 */
	Pathname         createAttachPoint() const;
	/**
	 * Try to create a temporary attach point in specified root.
	 * \param attach_root The attach root dir where to create the
	 *                    attach point in.
	 * \return The name of the new attach point or empty path name.
	 */
        Pathname         createAttachPoint(const Pathname &attach_root) const;

	/**
	 * Remove unused attach point. If the attach point is temporary,
	 * the attach point directory and all it content will be removed.
	 */
	void             removeAttachPoint();

        /**
	 * Verify if the specified directory as attach point (root)
	 * as requires by the particular media handler implementation.
	 * \param apoint The directory to check.
	 * \return True, if the directory checks succeeded.
	 */
	virtual bool     checkAttachPoint(const Pathname &apoint) const;

	/**
	 * Verify if the specified directory as attach point (root)
	 * using requested checks.
	 * \param apoint The directory to check.
	 * \param empty_dir Check if the directory is empty.
	 * \param writeable Check if the directory is writeable.
	 * \return True, if the directory checks succeeded.
	 */
	static bool      checkAttachPoint(const Pathname &apoint,
					  bool            empty_dir,
	                                  bool            writeable);

	/**
	 * Ask media manager, if the specified path is already used
	 * as attach point or if there are another attach points
	 * bellow of it.
	 * \param path The attach point path to check.
	 * \param mtab Whether to check against the mtab, too.
	 * \return True, if the path can be used as attach point.
	 */
        bool             isUseableAttachPoint(const Pathname &path,
	                                      bool            mtab=true) const;

	/**
	 * Get the media source name or an empty string.
	 * \return media source name or empty string.
	 */
	std::string      mediaSourceName() const
	{
	  return _mediaSource ? _mediaSource->name : "";
	}

	/**
	 * Set new media source reference.
	 * \param ref The new reference.
	 */
	void             setMediaSource(const MediaSourceRef &ref);

	/**
	 * Ask the media manager if specified media source
	 * is already attached.
	 */
	AttachedMedia
	findAttachedMedia(const MediaSourceRef &media) const;

	/**
	 * Returns the attached media. Used by MediaManager
	 * to find other handlers using the same source.
	 * \note This function increments reference counters
	 *       on the mediaSource and attachPoint references
	 *       it contains, for the life time of the returned
	 *       object. That is, it enables a (temporary) sharing
	 *       of them.
	 * \return The AttachedMedia struct containing (shared)
	 *         references to media source and attach point.
	 */
	AttachedMedia    attachedMedia() const;

	/**
	 * Returns a hint if the media is shared or not.
	 * \return true, if media is shared.
	 */
	bool             isSharedMedia() const;

	/**
	 * Check actual mediaSource attachment against the current
	 * mount table of the system. Used to implement isAttached().
	 * \param matchMountFs If to use the filesystem type from the
	 *        mount table (nfs, smb and cifs) or from mediaSource
	 *        while compare of a mount entry with mediaSource.
	 * \return true, if the media appears in the mount table.
	 */
	bool             checkAttached(bool matchMountFs) const;

	/**
	 * Call to this function will try to release all media matching
	 * the currenlty attached media source, that it is able to find
	 * in the mount table. This means also foreign (user) mounts!
	 * \param matchMountFs If to use the filesystem type from the
	 *        mount table (nfs, smb and cifs) or from mediaSource
	 *        while compare of a mount entry with mediaSource.
	 */
	void             forceRelaseAllMedia(bool matchMountFs);
	void             forceRelaseAllMedia(const MediaSourceRef &ref,
	                                     bool matchMountFs);

    protected:

        ///////////////////////////////////////////////////////////////////
        //
        // Real action interface to be overloaded by concrete handler.
        //
        ///////////////////////////////////////////////////////////////////

	/**
	 * Call concrete handler to attach the media.
	 *
	 * Asserted that not already attached, and attachPoint is a directory.
	 *
	 * @param next try next available device in turn until end of device
	 * list is reached (for media which are accessible through multiple
	 * devices like cdroms).
	 *
	 * \throws MediaException
	 *
	 **/
	virtual void attachTo(bool next = false) = 0;

        /**
         * Call concrete handler to disconnect media.
	 *
	 * Asserted that media is attached.
	 *
         * This is useful for media which e.g. holds open a connection to a
         * server like FTP. After calling disconnect() the media object still is
         * valid and files are present.
	 *
         * After calling disconnect() it's not possible to call provideFile() or
         * provideDir() anymore.
	 *
	 * \throws MediaException
	 *
	 **/
        virtual void disconnectFrom() { return; }

	/**
	 * Call concrete handler to release the media.
	 *
	 * If eject is true, and the media is used in one handler
	 * instance only, physically eject the media (i.e. CD-ROM).
	 *
	 * Asserted that media is attached.
	 * \param ejectDev Device to eject. None if empty.
	 *
	 * \throws MediaException
	 *
	 **/
	virtual void releaseFrom( const std::string & ejectDev = "" ) = 0;

	/**
	 * Call concrete handler to physically eject the media (i.e. CD-ROM)
	 * in case the media is not attached..
	 *
	 * Asserted that media is not attached.
	 **/
	virtual void forceEject( const std::string & device ) {}

	/**
	 * Call concrete handler to provide file below attach point.
	 *
	 * Default implementation provided, that returns whether a file
	 * is located at 'localRoot + filename'.
	 *
	 * Asserted that media is attached.
	 *
	 * \throws MediaException
	 *
	 **/
	virtual void getFile( const Pathname & filename ) const = 0;

        /**
         * Call concrete handler to provide a file under a different place
         * in the file system (usually not under attach point) as a copy.
         * Media must be attached before by callee.
         *
         * Default implementation provided that calls getFile(srcFilename)
         * and copies the result around.
	 *
	 * \throws MediaException
	 *
         **/
        virtual void getFileCopy( const Pathname & srcFilename, const Pathname & targetFilename ) const;


	/**
	 * Call concrete handler to provide directory content (not recursive!)
	 * below attach point.
	 *
	 * Return E_not_supported_by_media if media does not support retrieval of
	 * directory content.
	 *
	 * Default implementation provided, that returns whether a directory
	 * is located at 'localRoot + dirname'.
	 *
	 * Asserted that media is attached.
	 *
	 * \throws MediaException
	 *
	 **/
	virtual void getDir( const Pathname & dirname, bool recurse_r ) const = 0;

	/**
	 * Call concrete handler to provide a content list of directory on media
	 * via retlist. If dots is false entries starting with '.' are not reported.
	 *
	 * Return E_not_supported_by_media if media does not support retrieval of
	 * directory content.
	 *
	 * Default implementation provided, that returns the content of a
	 * directory at 'localRoot + dirnname' retrieved via 'readdir'.
	 *
	 * Asserted that media is attached and retlist is empty.
	 *
	 * \throws MediaException
	 *
	 **/
        virtual void getDirInfo( std::list<std::string> & retlist,
                                 const Pathname & dirname, bool dots = true ) const = 0;

	/**
	 * Basically the same as getDirInfo above. The content list is returned as
	 * filesystem::DirContent, which includes name and filetype of each directory
	 * entry. Retrieving the filetype usg. requires an additional ::stat call for
	 * each entry, thus it's more expensive than a simple readdir.
	 *
	 * Asserted that media is attached and retlist is empty.
	 *
	 * \throws MediaException
	 *
	 **/
        virtual void getDirInfo( filesystem::DirContent & retlist,
                                 const Pathname & dirname, bool dots = true ) const = 0;

        /**
         * check if a file exists
         *
         * Asserted that url is a file and not a dir.
         *
         * \throws MediaException
         *
         **/
        virtual bool getDoesFileExist( const Pathname & filename ) const = 0;

  protected:

        /**
	 * Retrieve and if available scan dirname/directory.yast.
	 *
	 * Asserted that media is attached.
	 *
	 * \throws MediaException
	 *
	 **/
        void getDirectoryYast( std::list<std::string> & retlist,
                               const Pathname & dirname, bool dots = true ) const;

        /**
	 * Retrieve and if available scan dirname/directory.yast.
	 *
	 * Asserted that media is attached.
	 *
	 * \throws MediaException
	 *
	 **/
        void getDirectoryYast( filesystem::DirContent & retlist,
                               const Pathname & dirname, bool dots = true ) const;

  public:

	/**
	 * If the concrete media handler provides a nonempty
	 * attach_point, it must be an existing directory.
	 *
	 * On an empty attach_point, MediaHandler will create
	 * a temporay directory, which will be erased from
	 * destructor.
	 *
	 * On any error, the attach_point is set to an empty Pathname,
	 * which should lead to E_bad_attachpoint.
	 **/
	MediaHandler ( const Url&       url_r,
		       const Pathname & attach_point_r,
		       const Pathname & urlpath_below_attachpoint_r,
		       const bool       does_download_r );

	/**
	 * Contolling MediaAccess takes care, that attached media is released
	 * prior to deleting this.
	 **/
	virtual ~MediaHandler();

    public:


        ///////////////////////////////////////////////////////////////////
        //
        // MediaAccess interface. Does common checks and logging.
        // Invokes real action if necessary.
        //
        ///////////////////////////////////////////////////////////////////

	/**
	 * Hint if files are downloaded or not.
	 */
	bool        downloads() const { return _does_download; }

        /**
	 * Protocol hint for MediaAccess.
	 **/
        std::string protocol() const { return _url.getScheme(); }

	/**
	 * Url used.
	 **/
        Url url() const { return _url; }

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
	void attach(bool next);

	/**
	 * True if media is attached.
	 **/
	virtual bool isAttached() const { return _mediaSource; }

	/**
	 * Return the local directory that corresponds to medias url,
	 * no matter if media isAttached or not. Files requested will
	 * be available at 'localRoot() + filename' or better
	 * 'localPath( filename )'.
	 *
	 * Returns empty pathname if E_bad_attachpoint
	 **/
	Pathname localRoot() const;

	/**
	 * Files provided will be available at 'localPath(filename)'.
	 *
	 * Returns empty pathname if E_bad_attachpoint
	 **/
         Pathname localPath( const Pathname & pathname ) const;

        /**
	 * Use concrete handler to isconnect media.
	 *
	 * This is useful for media which e.g. holds open a connection to a
	 * server like FTP. After calling disconnect() the media object still is
	 * valid and files are present.
	 *
	 * After calling disconnect() it's not possible to call provideFile() or
	 * provideDir() anymore.
	 *
	 * \throws MediaException
	 *
	 **/
        void disconnect();

	/**
	 * Use concrete handler to release the media.
	 * @param eject Device to physically eject. None if empty.
	 *
	 * \throws MediaException
	 **/
	void release( const std::string & ejectDev = "" );

	/**
	 * Use concrete handler to provide file denoted by path below
	 * 'localRoot'. Filename is interpreted relative to the
	 * attached url and a path prefix is preserved.
	 *
	 * \throws MediaException
	 *
	 **/
	void provideFile( Pathname filename ) const;

	/**
	 * Call concrete handler to provide a copy of a file under a different place
         * in the file system (usually not under attach point) as a copy.
         * Media must be attached before by callee.
         *
         * @param srcFilename    Filename of source file on the media
         * @param targetFilename Filename for the target in the file system
	 *
	 * \throws MediaException
	 *
	 **/
        void provideFileCopy( Pathname srcFilename, Pathname targetFilename) const;

	/**
	 * Use concrete handler to provide directory denoted
	 * by path below 'localRoot' (not recursive!).
	 * dirname is interpreted relative to the
	 * attached url and a path prefix is preserved.
	 *
	 * \throws MediaException
	 *
	 **/
	void provideDir( Pathname dirname ) const;

	/**
	 * Use concrete handler to provide directory tree denoted
	 * by path below 'localRoot' (recursive!!).
	 * dirname is interpreted relative to the
	 * attached url and a path prefix is preserved.
	 *
	 * \throws MediaException
	 *
	 **/
	void provideDirTree( Pathname dirname ) const;

	/**
	 * Remove filename below localRoot IFF handler downloads files
	 * to the local filesystem. Never remove anything from media.
	 *
	 * \throws MediaException
	 *
	 **/
	void releaseFile( const Pathname & filename ) const { return releasePath( filename ); }

	/**
	 * Remove directory tree below localRoot IFF handler downloads files
	 * to the local filesystem. Never remove anything from media.
	 *
	 * \throws MediaException
	 *
	 **/
	void releaseDir( const Pathname & dirname ) const { return releasePath( dirname ); }

	/**
	 * Remove pathname below localRoot IFF handler downloads files
	 * to the local filesystem. Never remove anything from media.
	 *
	 * If pathname denotes a directory it is recursively removed.
	 * If pathname is empty or '/' everything below the localRoot
	 * is recursively removed.
	 * If pathname denotes a file it is unlinked.
	 *
	 * \throws MediaException
	 *
	 **/
	void releasePath( Pathname pathname ) const;

        /*
         * set a deltafile to be used in the next download
         */
	void setDeltafile( const Pathname &filename = Pathname()) const;

	/*
	 * return the deltafile set with setDeltafile()
	 */
	Pathname deltafile () const;
   
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
         * Check if the media has one more device available for attach(true).
         */
        virtual bool hasMoreDevices();

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
};

///////////////////////////////////////////////////////////////////

  } // namespace media
} // namespace zypp


#endif // ZYPP_MEDIA_MEDIAHANDLERL_H


