/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaManager.h
 *
*/
#ifndef ZYPP_MEDIA_MEDIAMANAGER_H
#define ZYPP_MEDIA_MEDIAMANAGER_H

#include <zypp/media/MediaAccess.h>

#include <zypp/base/Deprecated.h>
#include <zypp/base/NonCopyable.h>
#include <zypp/base/PtrTypes.h>
#include <zypp/Pathname.h>
#include <zypp/Url.h>

#include <list>


//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
  namespace media
  { //////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////
    typedef zypp::RW_pointer<MediaAccess> MediaAccessRef;

    // OBSOLETE HERE:
    typedef MediaAccessId                 MediaId;
    typedef unsigned int                  MediaNr;


    ///////////////////////////////////////////////////////////////////
    // forward declaration
    class MountEntry;
    class MediaManager_Impl;

    ///////////////////////////////////////////////////////////////////
    //
    // CLASS NAME : MediaVerifierBase
    //
    /**
     * Interface to implement a media verifier.
     */
    class MediaVerifierBase //: private zypp::NonCopyable
    {
    public:
      MediaVerifierBase()
      {}

      virtual
      ~MediaVerifierBase()
      {}

      /**
       * Returns a string with some info about the verifier.
       * By default, the type info name is returned.
       */
      virtual std::string
      info() const;

      /*
      ** Check if the specified attached media contains
      ** the desired media (e.g. SLES10 CD1).
      */
      virtual bool
      isDesiredMedia(const MediaAccessRef &ref) = 0;
    };


    ///////////////////////////////////////////////////////////////////
    //
    // CLASS NAME : NoVerifier
    //
    /**
     * Dummy default media verifier, which is always happy.
     */
    class NoVerifier : public MediaVerifierBase
    {
    public:
      NoVerifier(): MediaVerifierBase()
      {}

      virtual
      ~NoVerifier()
      {}

      /**
       * Returns the "zypp::media::NoVerifier" string.
       */
      virtual std::string
      info() const;

      /*
      ** Don't check if the specified attached media contains
      ** the desired media number. Always return true.
      */
      virtual bool
      isDesiredMedia(const MediaAccessRef &ref)
      {
        (void)ref;
        return true;
      }
    };


    ///////////////////////////////////////////////////////////////////
    //
    // CLASS NAME : MediaVerifierRef
    //
    /**
     * A shared reference to the MediaVerifier implementation.
     */
    typedef zypp::RW_pointer<MediaVerifierBase> MediaVerifierRef;


    ///////////////////////////////////////////////////////////////////
    //
    // CLASS NAME : MediaManager
    //
    /**
     * Manages access to the 'physical' media, e.g CDROM drives,
     * Disk volumes, directory trees, etc, using \ref MediaAccessUrl's.
     *
     * \note The MediaManager class is just an envelope around an
     *       inner singleton like implementation.<br>
     *       That is, you can create as many managers as you want,
     *       also temporary in a function call.<br>
     *       But <b>don't</b> declare static MediaManager instances,
     *       unless you want to force (mutex) initialization order
     *       problems!
     *
     * \section MediaAccessUrl Media Access Url
     * The MediaManager uses several media access handlers (backends),
     * that can be specified by a Media Access URL in its open() method.
     *
     * All URL's may contain following query parameters, that are
     * reserved by the Source classes and unused/ignored by the media
     * manager:
     * - <tt>alias</tt>: A source specific media alias string.
     *
     * Currently, following access handlers (backends) are implemented:
     *   - \ref MediaCD_Url
     *   - \ref MediaDISK_Url
     * .
     *   - \ref MediaISO_Url
     *   - \ref MediaDIR_Url
     * .
     *   - \ref MediaNFS_Url
     *   - \ref MediaCIFS_Url
     *   - \ref MediaCurl_Url
     *
     * \subsection MediaCD_Url MediaCD - CD/DVD drives (cd, dvd)
     * The access handler for media on CD / DVD drives.
     *   - Scheme:
     *     - <b>cd</b>: Requires a drive supporting CD media.
     *     - <b>dvd</b>: Prefers a drive supporting DVD media.
     *   - Examples:
     *     \code
     *       "cd:/"
     *       "cd:/?devices=/dev/hda,/dev/hdb"
     *       "cd:/subdir?devices=/dev/hda,/dev/hdb"
     *
     *       "dvd:/"
     *       "dvd:/?devices=/dev/hda,/dev/hdb"
     *       "dvd:/subdir?devices=/dev/hda,/dev/hdb"
     *     \endcode
     *     Note: You can use either "dvd:/" (just path, no authority)
     *           or "dvd:///" (path and empty authority).
     *   - Query parameters:
     *     - <tt>devices</tt>:
     *       Optional parameter, containing a comma separated list of
     *       block device names to use, e.g.: "/dev/sr0,/dev/sr1".
     *       <br>
     *       The device names will be verified using a HAL query. If one
     *       of the provided devices is not usable (not a block device,
     *       or does not support required media type), an exception is
     *       thrown.
     *       <br>
     *       If the devices parameter is not provided (or empty), all
     *       avaliable CD/DVD drives 'detected' using a HAL query. The
     *       preferred drive (used as first drive) is the drive pointed
     *       to by the symlink "/dev/dvd" ("dvd" scheme only) or
     *       "/dev/cdrom".
     *   - Authority:
     *     A non-empty authority URL component (e.g. containing a host
     *     name) is not allowed.
     *   - Path name:
     *     Mandatory URL component, that specifies a subdirectory on the
     *     CD/DVD, where the desired files are located.
     *
     * \subsection MediaDISK_Url MediaDISK - HD disk volumes (hd)
     * The access handler for media on a disk volume (partition).
     *   - Scheme:
     *     - <b>hd</b>
     *   - Examples:
     *     \code
     *       "hd:/?device=/dev/hda1"
     *       "hd:/subdir?device=/dev/sda1"
     *       "hd:/subdir?device=/dev/sda1&filesystem=reiserfs"
     *     \endcode
     *     Note: You can use either "hd:/" (just path, no authority)
     *     or "hd:///" (path and empty authority).
     *   - Query parameters:
     *     - <tt>device</tt>:
     *       Mandatory parameter specifying the name of the block device of
     *       the partition to mount.
     *     - <tt>filesystem</tt>:
     *       The name of the filesystem. Defaults to "auto". 
     *   - Authority:
     *     A non-empty authority URL component is not allowed.
     *   - Path name:
     *     Mandatory URL component, that specifies a subdirectory on the disk
     *     partition, where the desired files are located.
     *
     * \subsection MediaDIR_Url MediaDIR - Local directory tree (dir, file)
     * The access handler to media stored in a local directory tree.
     *   - Scheme:
     *     - <b>dir</b>
     *     - <b>file</b>
     *   - Examples:
     *     \code
     *       "dir:/directory/name"
     *       "file:/directory/name"
     *     \endcode
     *   - Query parameters:
     *     none 
     *   - Authority:
     *     A non-empty authority URL component (e.g. containing
     *     a host name) is not allowed.
     *   - Path name:
     *     Mandatory URL component, that specifies a directory, where
     *     the desired files are located.
     *
     * \subsection MediaISO_Url MediaISO - Loopback ISO images (iso)
     * The access handler for media in a ISO image (loopback mount).
     *   - Scheme:
     *     - <b>iso</b>
     *   - Examples:
     *     \code
     *       "iso:/?iso=/path/to/CD1.iso"
     *       "iso:/?iso=CD1.iso&url=dir:/path/to"
     *
     *       "iso:/?iso=CD1.iso&url=nfs://server/path/to/media"
     *       "iso:/?iso=CD1.iso&url=hd:/?device=/dev/hda"
     * 
     *        "iso:/subdir?iso=DVD1.iso&url=nfs://nfs-server/directory&mnt=/nfs/attach/point&filesystem=udf"
     *     \endcode
     *   - Query parameters:
     *     - <tt>iso</tt>:
     *       Mandatory parameter specifying the name of the iso file.<br>
     *       If the url parameter is missed, the iso parameter has to contain
     *       an absolute iso file name.
     *     - <tt>url</tt>:
     *       Optional parameter specifying the URL to the directory containing
     *       the iso file.<br>
     *       The supported URL schemes are: <i><b>hd</b>, <b>dir</b>,
     *       <b>file</b>, <b>nfs</b>, <b>smb</b>, <b>cifs</b>.</i>
     *     - <tt>mnt</tt>:
     *       Optional parameter specifying the prefered attach point for the
     *       source media url.
     *     - <tt>filesystem</tt>:
     *       Optional name of the filesystem used in the iso file. Defaults
     *       to "auto". 
     *   - Authority:
     *     A non-empty authority URL component is not allowed.
     *   - Path name:
     *     Mandatory URL component, that specifies a subdirectory inside of
     *     the iso file, where the desired files are located.
     *
     * \subsection MediaNFS_Url MediaNFS  - NFS directory tree (nfs)
     * The access handler for media on NFS exported directory tree.
     *   - Scheme:
     *     - <b>nfs</b>
     *   - Examples:
     *     \code
     *        "nfs://nfs-server/exported/path"
     *        "nfs://nfs-server/exported/path?mountoptions=ro"
     *     \endcode
     *   - Query parameters:
     *     - <tt>mountoptions</tt>:
     *       The mount options separated by comma ','.
     *       Default is the "ro" option.
     *   - Authority:
     *     The authority component has to provide a hostname.
     *     Username, password and port are currently ignored.
     *   - Path name:
     *     Mandatory URL component, that specifies the exported
     *     (sub-)directory on the NFS server, where the desired
     *     files are located.
     *
     * \subsection MediaCIFS_Url MediaCIFS - CIFS/SMB directory tree (cifs, smb)
     * The access handler for media in a CIFS/SMB shared directory tree.
     *   - Scheme:
     *     - <b>cifs</b>
     *     - <b>smb</b>
     *   - Examples:
     *     \code
     *       "cifs://servername/share/path/on/the/share"
     *       "cifs://username:passwd@servername/share/path/on/the/share?mountoptions=ro"
     *       "smb://servername/share/path/on/the/share"
     *       "smb://username:passwd@servername/share/path/on/the/share?mountoptions=ro"
     *     \endcode
     *     Note: There is no difference between cifs and smb scheme
     *     (any more). In both cases the 'cifs' filesystem is used.
     *   - Query parameters:
     *     - <tt>mountoptions</tt>:
     *       The mount options separated by a comma ','. Default are the
     *       "ro" and "guest" options.
     *     - <tt>workgroup</tt>:
     *       The name of the workgroup.
     *     - <tt>username</tt>:
     *       Alternative username to username in URL authority.
     *     - <tt>password</tt>:
     *       Alternative password to password in URL authority.
     *     - <tt>user</tt>:
     *       Alternative username (cifs specific variant?)
     *     - <tt>pass</tt>:
     *       Alternative password (cifs specific variant?)
     *   - Authority:
     *     The authority component has to provide a hostname. Optionally
     *     also a username and password.
     *   - Path name:
     *     Mandatory URL component, that specifies the share name with
     *     optional subdirectory, where the desired files are located.
     *
     * \subsection MediaCurl_Url MediaCurl - FTP/HTTP directory tree (ftp, http, https) 
     * The access handler to media directory tree on a ftp/http server.
     *   - Scheme:
     *     - <b>ftp</b>
     *     - <b>http</b>
     *     - <b>https</b>
     *   - Examples:
     *     \code
     *       "ftp://server/relative/path/to/media/dir"
     *       "ftp://server/%2fabsolute/path/to/media/dir"
     *
     *       "ftp://user:pass@server/path/to/media/dir"
     *       "ftp://user:pass@server/%2f/home/user/path/to/media/dir"
     *
     *       "http://server/path/on/server"
     *       "http://user:pass@server/path"
     *       "https://user:pass@server/path?proxy=foo&proxyuser=me&proxypass=pw"
     *     \endcode
     *     Note: The "ftp" url scheme supports absolute and relative
     *     paths to the default ftp server directory
     *     (<a href="http://rfc.net/rfc1738.html">RFC1738, Section 3.2.2</a>).<br>
     *     To use an absolute path, you have to prepend the path with an
     *     additional slash, what results in a "/%2f" combination
     *     (second "/" encoded to "%2f") at the begin of the URL path.
     *     <br>
     *     This is important, especially in user authenticated ftp,
     *     where the users home is usually the default directory of the
     *     server (except when the server chroots into the users home
     *     directory).
     *     <br>
     *     For example, if the user "user" has a home directory
     *     "/home/user", you can use either an URL with a relative path
     *     to the home directory "ftp://user:pass@server/path/to/media"
     *     or the absolute path
     *     "ftp://user:pass@server/%2fhome/user/path/to/media" -- both
     *     URLs points to the same directory on the server.
     *   - Query parameters:
     *     - <tt>proxy</tt>:
     *       A proxy hostname or hostname and port separated by ':'.
     *     - <tt>proxyport</tt>:
     *       Alternative way to provide the proxy port.
     *     - <tt>proxyuser</tt>:
     *       The proxy username.
     *     - <tt>proxypass</tt>:
     *       The proxy password. 
     *   - Authority:
     *     The authority component has to provide a hostname. Optionally
     *     also a username and password. In case of the 'ftp' scheme,
     *     username and password defaults to 'anonymous' and 'yast2@'.
     *   - Path name:
     *     Mandatory URL component, that specifies the path name on the
     *     server, where the desired files are located.
     *
     */
    class MediaManager: private zypp::base::NonCopyable
    {
    public:
      /**
       * Creates a MediaManager envelope instance.
       *
       * In the case, that the inner implementation is not already
       * allocated, and the MediaManager constructor was unable to
       * allocate it, a std::bad_alloc exception is thrown.
       *
       * All further instances increase the use counter only.
       *
       * \throws std::bad_alloc
       */
      MediaManager();

      /**
       * Destroys MediaManager envelope instance.
       * Decreases the use counter of the inner implementation.
       */
      ~MediaManager();

      /**
       * Opens the media access for specified with the url.
       *
       * If the \p preferred_attach_point parameter does not
       * point to a usable attach point directory, the media
       * manager automatically creates a temporary attach
       * point in a default directory. This default directory
       * can be changed using setAttachPrefix() function.
       *
       * Remember to close() each id you've opened and not
       * need any more. It is like a new and delete!
       *
       * \param  url The \ref MediaAccessUrl.
       * \param  preferred_attach_point The preferred, already
       *         existing directory, where the media should be
       *         attached.
       * \return a new media access id.
       * \throws std::bad_alloc
       * \throws MediaException
       */
      MediaAccessId
      open(const Url &url, const Pathname & preferred_attach_point = "");

      /**
       * Close the media access with specified id.
       * \param accessId The media access id to close.
       */
      void
      close(MediaAccessId accessId);

      /**
       * Query if the media access is open / exists.
       *
       * \param accessId The media access id to query.
       * \return true, if access id is known and open.
       */
      bool
      isOpen(MediaAccessId accessId) const;

      /**
       * Query the protocol name used by the media access
       * handler. Similar to url().getScheme().
       *
       * \param accessId The media access id to query.
       * \return The protocol name used by the media access
       *         handler, otherwise 'unknown'.
       * \throws MediaNotOpenException for invalid access id.
       */
      std::string
      protocol(MediaAccessId accessId) const;

      /**
       * Hint if files are downloaded or not.
       * \param accessId The media access id to query.
       * \return True, if provideFile downloads files.
       */
      bool
      downloads(MediaAccessId accessId) const;

      /**
       * Hint if files will be downloaded when using the
       * specified media \p url.
       *
       * @note This hint is based on the \p url scheme
       * only and does not imply, that the URL is valid.
       *
       * @param url The media URL to check.
       * @return True, if the files are downloaded.
       */
      static bool
      downloads(const Url &url);

      /**
       * Returns the \ref MediaAccessUrl of the media access id.
       *
       * \param accessId The media access id to query.
       * \return The \ref MediaAccessUrl used by the media access id.
       * \throws MediaNotOpenException for invalid access id.
       */
      Url
      url(MediaAccessId accessId) const;

    public:
      /**
       * Add verifier implementation for the specified media id.
       * By default, the NoVerifier is used.
       *
       * \param accessId A media access id.
       * \param verifier The new verifier.
       * \throws MediaNotOpenException for invalid access id.
       */
      void
      addVerifier(MediaAccessId accessId,
                  const MediaVerifierRef &verifier);

      /**
       * Remove verifier for specified media id.
       * It resets the verifier to NoVerifier.
       *
       * \param accessId A media access id.
       * \throws MediaNotOpenException for invalid access id.
       */
      void
      delVerifier(MediaAccessId accessId);

    public:
      /**
       * Set or resets the directory name, where the media manager
       * handlers create their temporary attach points (see open()
       * function).
       * It has effect to newly created temporary attach points only.
       *
       * \param attach_prefix The new prefix for temporary attach
       *        points, or empty pathname to reset to defaults.
       * \return True on success, false if the \p attach_prefix
       *         parameters contains a path name, that does not
       *         point to a writable directory.
       */
      bool
      setAttachPrefix(const Pathname &attach_prefix);

      /**
       * Attach the media using the concrete handler.
       *
       * Remember to release() or close() each id you've attached
       * and not need any more. Attach is like an open of a file!
       *
       * \param accessId A media access id.
       * \param next     Whether to try the next drive if avaliable.
       * \throws MediaNotOpenException for invalid access id.
       */
      void
      attach(MediaAccessId accessId, bool next = false);

      /**
       * Release the attached media and optionally eject.
       *
       * If the \p eject parameter is set to true and there
       * is currently an attached drive, all other access
       * id's are released and the drive (CD/DVD drive) is
       * ejected.
       * In case that there is currently no attached drive,
       * a \p eject set to true causes to eject all drives
       * that are _not_ used by another access id's.
       *
       * \param accessId A media access id.
       * \param eject    Whether to eject the drive.
       * \throws MediaNotOpenException for invalid access id.
       */
      void
      release(MediaAccessId accessId, bool eject = false);

      /**
       * Disconnect a remote media.
       *
       * This is useful for media which e.g. holds open a connection
       * to a server like FTP. After calling disconnect() the media
       * object (attach point) is still valid and files are present.
       *
       * But after calling disconnect() it's not possible to call
       * fetch more data using the provideFile() or provideDir()
       * functions anymore.
       *
       * \param accessId A media access id.
       * \throws MediaNotOpenException for invalid access id.
       */
      void
      disconnect(MediaAccessId accessId);

      /**
       * Check if media is attached or not.
       *
       * \param accessId A media access id.
       * \return True if media is attached.
       * \throws MediaNotOpenException for invalid access id.
       */
      bool
      isAttached(MediaAccessId accessId) const;

      /**
       * Returns information if media is on a shared
       * physical device or not.
       *
       * \param accessId A media access id.
       * \return True if it is shared, false if not.
       * \throws MediaNotOpenException for invalid access id.
       */
      bool
      isSharedMedia(MediaAccessId accessId) const;

      /**
       * Ask the registered verifier if the attached
       * media is the desired one or not.
       *
       * \param accessId A media access id.
       * \return True if media is attached and desired
       *         according to the actual verifier.
       * \throws MediaNotOpenException for invalid access id.
       */
      bool
      isDesiredMedia(MediaAccessId accessId) const;

      /**
       * Ask the specified verifier if the attached
       * media is the desired one or not.
       *
       * \param accessId A media access id.
       * \param verifier A verifier to use.
       * \return True if media is attached and desired
       *         according to the specified verifier.
       * \throws MediaNotOpenException for invalid access id.
       */
      bool
      isDesiredMedia(MediaAccessId           accessId,
                     const MediaVerifierRef &verifier) const;

      /**
       * Return the local directory that corresponds to medias url,
       * no matter if media isAttached or not. Files requested will
       * be available at 'localRoot() + filename' or even better
       * 'localPath( filename )'
       *
       * \param accessId A media access id.
       * \returns The directory name pointing to the media root
       *          in local filesystem or an empty pathname if the
       *          media is not attached.
       * \throws MediaNotOpenException for invalid access id.
       */
      Pathname
      localRoot(MediaAccessId accessId) const;

      /**
       * Shortcut for 'localRoot() + pathname', but returns an empty
       * pathname if media is not attached.
       * Files provided will be available at 'localPath(filename)'.
       *
       * \param accessId A media access id.
       * \param pathname A path name relative to the localRoot().
       * \returns The directory name in local filesystem pointing
       *          to the desired relative pathname on the media
       *          or an empty pathname if the media is not attached.
       * \throws MediaNotOpenException for invalid access id.
       */
      Pathname
      localPath(MediaAccessId accessId, const Pathname & pathname) const;

    public:
      /**
       * Provide provide file denoted by relative path below of the
       * 'attach point' of the specified media and the path prefix
       * on the media.
       *
       * \param accessId  The media access id to use.
       * \param filename  The filename to provide, relative to localRoot().
       * \param cached    If cached is set to true, the function checks, if
       *                  the file already exists and doesn't download it again
       *                  if it does. Currently only the existence is checked,
       *                  no other file attributes.
       * \param checkonly If this and 'cached' are set to true only the
       *                  existence of the file is checked but it's not
       *                  downloaded. If 'cached' is unset an errer is
       *                  returned always.
       *
       * \throws MediaNotOpenException in case of invalid access id.
       * \throws MediaNotAttachedException in case, that the media is not attached.
       * \throws MediaNotDesiredException in case, that the media verification failed.
       * \throws MediaNotAFileException in case, that the requested filename is not a file.
       * \throws MediaFileNotFoundException in case, that the requested filenamedoes not exists.
       * \throws MediaWriteException in case, that the file can't be copied from from remote source.
       * \throws MediaSystemException in case a system operation fails.
       * \throws MediaException derived exception, depending on the url (handler).
       */
      void
      provideFile(MediaAccessId   accessId,
                  const Pathname &filename,
                  bool            cached    = false,
                  bool            checkonly = false) const;

      /**
       * FIXME: see MediaAccess class.
       */
      void
      provideDir(MediaAccessId   accessId,
                 const Pathname &dirname) const;

      /**
       * FIXME: see MediaAccess class.
       */
      void
      provideDirTree(MediaAccessId  accessId,
                     const Pathname &dirname) const;

      /**
       * FIXME: see MediaAccess class.
       */
      void
      releaseFile(MediaAccessId   accessId,
                  const Pathname &filename) const;

      /**
       * FIXME: see MediaAccess class.
       */
      void
      releaseDir(MediaAccessId   accessId,
                 const Pathname &dirname) const;

      /**
       * FIXME: see MediaAccess class.
       */
      void
      releasePath(MediaAccessId   accessId,
                  const Pathname &pathname) const;

      /**
       * FIXME: see MediaAccess class.
       */
      void
      dirInfo(MediaAccessId           accessId,
              std::list<std::string> &retlist,
              const Pathname         &dirname,
              bool                    dots = true) const;

      /**
       * FIXME: see MediaAccess class.
       */
      void
      dirInfo(MediaAccessId           accessId,
              filesystem::DirContent &retlist,
              const Pathname         &dirname,
              bool                   dots = true) const;


    public:
      /**
       * Get the modification time of the /etc/mtab file.
       * \return Modification time of the /etc/mtab file.
       */
      static time_t
      getMountTableMTime();

      /**
       * Get current mount entries from /etc/mtab file.
       * \return Current mount entries from /etc/mtab file.
       */
      static std::vector<MountEntry>
      getMountEntries();

      /**
       * Check if the specified \p path is useable as
       * attach point.
       *
       * \param path The attach point to check.
       * \param mtab Whether to check against the mtab, too.
       * \return True, if it is a directory and there are
       *         no another attach points bellow of it.
       */
      bool
      isUseableAttachPoint(const Pathname &path,
                           bool            mtab=true) const;

    private:
      friend class MediaHandler;

      /**
       * \internal
       * Return the attached media reference of the specified
       * media access id. Used to resolve nested attachments
       * as used in the MediaISO (iso-loop) handler.
       * Causes temporary creation of a shared attachment
       * (increases reference counters on attachedMedia).
       * \param media A media access id.
       */
      AttachedMedia
      getAttachedMedia(MediaAccessId &accessId) const;

      /**
       * \internal
       * Called by media handler in while attach() to retrieve
       * attached media reference matching the specified media
       * source reference.
       * Causes temporary creation of a shared attachment
       * (increases reference counters on attachedMedia).
       * \param media The media source reference to search for.
       */
      AttachedMedia
      findAttachedMedia(const MediaSourceRef &media) const;

      /**
       * \internal
       * Called by media handler in case of relase(eject=true)
       * to release all access id's using the specified media.
       * Causes temporary creation of a shared attachment
       * (increases reference counters on attachedMedia).
       * \param media The media source reference to release.
       */
      void
      forceReleaseShared(const MediaSourceRef &media);

    private:
      /**
       * Static reference to the implementation (singleton).
       */
      static zypp::RW_pointer<MediaManager_Impl> m_impl;
    };


    //////////////////////////////////////////////////////////////////
  } // namespace media
  ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////

#endif // ZYPP_MEDIA_MEDIAMANAGER_H

/*
** vim: set ts=2 sts=2 sw=2 ai et:
*/
