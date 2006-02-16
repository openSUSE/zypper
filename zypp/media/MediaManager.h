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
    typedef zypp::RW_pointer<MediaAccess> MediaAccessRef;
    typedef unsigned int                  MediaAccessId;

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
       * Open new access handler with specifier url and attach
       * point reusing the specified accessId.
      void
      reopen(MediaAccessId accessId, const Url &url,
             const Pathname & preferred_attach_point = "");
       */

      /**
       * Swap access handlers of idOne and idTwo.
       *
       * \returns True, if idOne and and idTwo was both valid.
      bool
      swap(MediaAccessId idOne, MediaAccessId idTwo);
       */

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
       * FIXME: Jiri, do you want this one?
       *
       * Attach the media if needed and verify, if desired
       * media is avaliable. If the access handler supports
       * multiple drives (e.g. CD/DVD), all drives will be
       * verified.
       * On success, the media is attached and verified.
       * On failure, the media is released and optionally
       * ejected if possible (not shared).
       *
       * \throws MediaNotDesiredException if unable to find
       *         desired media in any drive.
       * \throws FIXME if all drives are in use and no one
       *         was ejected.
      void
      attachDesiredMedia(MediaAccessId accessId, bool eject = true);
       */

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
       * \throws MediaException
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
      void dirInfo(MediaAccessId           accessId,
                   std::list<std::string> &retlist,
                   const Pathname         &dirname,
                   bool                    dots = true) const;

      /**
       */
      void dirInfo(MediaAccessId           accessId,
                   filesystem::DirContent &retlist,
                   const Pathname         &dirname,
                   bool                   dots = true) const;


    private:
      friend class MediaHandler;

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
