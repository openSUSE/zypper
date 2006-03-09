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
     * Manages coordinated access to 'physical' media, e.g CDROM
     * drives.
     *
     * \note The MediaManager class is just an envelope around an
     * inner singelton like implementation. This means, you can
     * create as many managers as you want, also temporary in a
     * function call.
     *
     * \note Don't declare static MediaManager instances, unless
     * you want to force (mutex) initialization order problems!
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
       * \param  url The media access url.
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
       * \param accessId The media access id query.
       * \return true, if access id is known and open.
       */
      bool
      isOpen(MediaAccessId accessId) const;

      /**
       * Query the protocol name used by the media access
       * handler. Similar to url().getScheme().
       *
       * \param accessId The media access id query.
       * \return The protocol name used by the media access
       *         handler, otherwise 'unknown'.
       * \throws MediaNotOpenException for invalid access id.
       */
      std::string
      protocol(MediaAccessId accessId) const;

      /**
	     * Hint if files are downloaded or not.
	     */
	    bool
      downloads(MediaAccessId accessId) const;

      /**
       * Url of the media access id, otherwise empty Url.
       *
       * \throws MediaNotOpenException for invalid access id.
       */
      Url
      url(MediaAccessId accessId) const;

    public:
      /**
       * Add verifier implementation for the specified media id.
       * By default, the NoVerifier is used.
       *
       * \throws MediaNotOpenException for invalid access id.
       */
      void
      addVerifier(MediaAccessId accessId,
                  const MediaVerifierRef &verifier);

      /**
       * Remove verifier for specified media id.
       *
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
       * \throws MediaNotOpenException for invalid access id.
       */
      void
      attach(MediaAccessId accessId, bool next = false);

      /**
       * Reattach to a new attach point.
       *
       * \deprecated This function will be removed, because the
       * reattach function has race conditions (e.g. open file
       * in the old attach point). Use setAttachPrefix() instead.
       *
       * \param accessId A media access Id.
       * \param attach_point A new attach point directory.
       * \param temporary    Whether to reattach to a temporary
       *      attach point bellow of \p attach_point and cleanup
       *      it on release (temporary=true), or use the provided
       *      directory as attach point without to cleanup it on
       *      release (temporary=false, default behaviour).
       * \throws MediaNotOpenException
       * \throws MediaNotSupportedException
       */
      void
      reattach(MediaAccessId   accessId,
               const Pathname &attach_point,
               bool            temporary = false) ZYPP_DEPRECATED;

      /**
       * Release the attached media and optionally eject.
       *
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
       * \throws MediaNotOpenException for invalid access id.
       */
      void
      disconnect(MediaAccessId accessId);

      /**
       * Check if media is attached or not.
       *
       * \return True if media is attached.
       * \throws MediaNotOpenException for invalid access id.
       */
      bool
      isAttached(MediaAccessId accessId) const;

      /**
       * Returns information if media is on a shared
       * physical device or not.
       *
       * \return True if it is shared, false if not.
       * \throws MediaNotOpenException for invalid access id.
       */
      bool
      isSharedMedia(MediaAccessId accessId) const;

      /**
       * Ask the registered verifier if the attached
       * media is the desired one or not.
       * \return True if media is attached and desired
       *         according to the actual verifier.
       * \throws MediaNotOpenException for invalid access id.
       */
      bool
      isDesiredMedia(MediaAccessId accessId) const;

      /**
       * Ask the specified verifier if the attached
       * media is the desired one or not.
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
       */
      void
      provideDir(MediaAccessId   accessId,
                 const Pathname &dirname) const;

      /**
       */
      void
      provideDirTree(MediaAccessId  accessId,
                     const Pathname &dirname) const;

      /**
       */
      void
      releaseFile(MediaAccessId   accessId,
                  const Pathname &filename) const;

      /**
       */
      void
      releaseDir(MediaAccessId   accessId,
                 const Pathname &dirname) const;

      /**
       */
      void
      releasePath(MediaAccessId   accessId,
                  const Pathname &pathname) const;

      /**
       */
      void
      dirInfo(MediaAccessId           accessId,
              std::list<std::string> &retlist,
              const Pathname         &dirname,
              bool                    dots = true) const;

      /**
       */
      void
      dirInfo(MediaAccessId           accessId,
              filesystem::DirContent &retlist,
              const Pathname         &dirname,
              bool                   dots = true) const;


    public:
      time_t
      getMountTableMTime() const;

      std::vector<MountEntry>
      getMountEntries() const;

      bool
      isUseableAttachPoint(const Pathname &path) const;

    private:
      friend class MediaHandler;

      AttachedMedia
      getAttachedMedia(MediaAccessId &accessId) const;

      AttachedMedia
      findAttachedMedia(const MediaSourceRef &media) const;

      void
      forceMediaRelease(const MediaSourceRef &media);

    private:
      class  Impl;
      static zypp::RW_pointer<MediaManager::Impl> m_impl;
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
