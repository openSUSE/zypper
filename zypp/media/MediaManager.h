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
    class MountEntry;


    ///////////////////////////////////////////////////////////////////
    typedef zypp::RW_pointer<MediaAccess> MediaAccessRef;

    // OBSOLETE HERE:
    typedef MediaAccessId                 MediaId;
    typedef unsigned int                  MediaNr;


    ///////////////////////////////////////////////////////////////////
    //
    // CLASS NAME : MediaVerifierBase
    //
    /**
     * Interface to implement a media verifier.
     */
    class MediaVerifierBase //: public zypp::NonCopyable
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
     * Dummy media verifier, which is always happy.
     */
    class NoVerifier : public MediaVerifierBase
    {
    public:
      NoVerifier()
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
     * Manages coordinated access to media, e.g. CDROM drives.
     * \todo document me!
     */
    class MediaManager: public zypp::base::NonCopyable
    {
    public:
      MediaManager();
      ~MediaManager();

      /**
       * open the media, return the ID, throw exception on fail
       */
      MediaAccessId
      open(const Url &url, const Pathname & preferred_attach_point = "");

      /**
       * close the media
       */
      void
      close(MediaAccessId accessId);

      /**
       * Query if media is open.
       * \return true, if media id is known.
       */
      bool
      isOpen(MediaAccessId accessId) const;

      /**
       * Used Protocol if media is opened, otherwise 'unknown'.
       */
      std::string
      protocol(MediaAccessId accessId) const;

      /**
       * Url of the media, otherwise empty.
       */
      Url
      url(MediaAccessId accessId) const;

    public:
      /**
       * Add verifier for specified media id.
       */
      void
      addVerifier(MediaAccessId accessId,
                  const MediaVerifierRef &verifier);

      /**
       * Remove verifier for specified media id.
       */
      void
      delVerifier(MediaAccessId accessId);

    public:
      /**
       * Attach the media using the concrete handler.
       */
      void
      attach(MediaAccessId accessId, bool next = false);

      /**
       * Release the attached media and optionally eject.
       * \throws MediaIsSharedException if eject is true
       *         and media is shared.
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
       * \throws MediaException
       */
      void
      disconnect(MediaAccessId accessId);

      /**
       * Check if media is attached or not.
       * \return True if media is attached.
       */
      bool
      isAttached(MediaAccessId accessId) const;

      /**
       * Returns information if media is on a shared
       * physical device or not.
       * \return True if it is shared, false if not.
       */
      bool
      isSharedMedia(MediaAccessId accessId) const;

      /**
       * Ask the registered verifier if the attached
       * media is the desired one or not.
       * \return True if desired media is attached.
       */
      bool
      isDesiredMedia(MediaAccessId accessId) const;

      /**
       * Ask the specified verifier if the attached
       * media is the desired one or not.
       * \return True if desired media is attached.
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
       * If media is not open an empty pathname is returned.
       */
      Pathname
      localRoot(MediaAccessId accessId) const;

      /**
       * Shortcut for 'localRoot() + pathname', but returns an empty
       * pathname if media is not open.
       * Files provided will be available at 'localPath(filename)'.
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
