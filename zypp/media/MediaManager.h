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


//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
  namespace media
  { //////////////////////////////////////////////////////////////////

    typedef zypp::RW_pointer<MediaAccess> MediaAccessRef;
    typedef unsigned int                  MediaId;
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
      ** the desired media number (e.g. SLES10 CD1).
      */
      virtual bool
      isDesiredMedia(const MediaAccessRef &ref, MediaNr mediaNr)
      {
        return false;
      }
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
      ** Check if the specified attached media contains
      ** the desired media number (e.g. SLES10 CD1). Always return true.
      */
      virtual bool
      isDesiredMedia(const MediaAccessRef &ref, MediaNr mediaNr)
      {
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
    class MediaManager: public zypp::base::NonCopyable
    {
    public:
      MediaManager();
      ~MediaManager();

      /**
       * open the media, return the ID, throw exception on fail
       */
      MediaId
      open(const Url &url, const Pathname & preferred_attach_point = "");

      /**
       * close the media
       */
      void
      close(MediaId mediaId);

      /**
       * Query if media is open.
       * \return true, if media id is known.
       */
      bool
      isOpen(MediaId mediaId) const;

      /**
       * Used Protocol if media is opened, otherwise 'unknown'.
       */
      std::string
      protocol(MediaId mediaId) const;

      /**
       * Url of the media, otherwise empty.
       */
      Url
      url(MediaId mediaId) const;

    public:
      /**
       * Add verifier for specified media id.
       */
      void
      addVerifier(MediaId mediaId, const MediaVerifierRef &ref);

      /**
       * Remove verifier for specified media id.
       */
      void
      delVerifier(MediaId mediaId);

    public:
      /**
       * attach the media using the concrete handler
       */
      void
      attach(MediaId mediaId, bool next = false);

      /**
       * Release the attached media and optionally eject.
       */
      void
      release(MediaId mediaId, bool eject = false);

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
      disconnect(MediaId mediaId);

      /**
       * Check if media is attached or not.
       * \return True if media is attached.
       */
      bool
      isAttached(MediaId mediaId) const;

      /**
       * Ask the registered verifier if the attached
       * media is the desired one or not.
       * \return True if desired media is attached.
       */
      bool
      isDesiredMedia(MediaId mediaId, MediaNr mediaNr) const;

      /**
       * Return the local directory that corresponds to medias url,
       * no matter if media isAttached or not. Files requested will
       * be available at 'localRoot() + filename' or even better
       * 'localPath( filename )'
       *
       * If media is not open an empty pathname is returned.
       */
      Pathname
      localRoot(MediaId mediaId) const;

      /**
       * Shortcut for 'localRoot() + pathname', but returns an empty
       * pathname if media is not open.
       * Files provided will be available at 'localPath(filename)'.
       */
      Pathname
      localPath(MediaId mediaId, const Pathname & pathname) const;

    public:
      /**
       * Provide provide file denoted by relative path below of the
       * 'attach point' of the specified media and the path prefix
       * on the media.
       *
       * \param mediaId The media source id to use.
       * \param mediaNr The desired media number that should be in the source.
       * \param cached  If cached is set to true, the function checks, if
       *                the file already exists and doesn't download it again
       *                if it does. Currently only the existence is checked,
       *                no other file attributes.
       * \param checkonly If this and 'cached' are set to true only the
       *                  existence of the file is checked but it's not
       *                  downloaded. If 'cached' is unset an errer is
       *                  returned always.
       *
       * \throws MediaException
       *
       */
      void
      provideFile(MediaId mediaId,
                  MediaNr mediaNr,
                  const Pathname &filename,
                  bool            cached    = false,
                  bool            checkonly = false) const;

      /**
       */
      void
      provideDir(MediaId mediaId,
                 MediaNr mediaNr,
                 const Pathname & dirname ) const;

      /**
       */
      void
      provideDirTree(MediaId mediaId,
                     MediaNr mediaNr,
                     const Pathname & dirname ) const;

      /**
       */
      void
      releaseFile(MediaId mediaId,
                  const Pathname & filename) const;

      /**
       */
      void
      releaseDir(MediaId mediaId,
                 const Pathname & dirname ) const;

      /**
       */
      void
      releasePath(MediaId mediaId,
                  const Pathname & pathname ) const;

      /**
       */
      void dirInfo(MediaId mediaId,
                   std::list<std::string> & retlist,
                   const Pathname & dirname, bool dots = true ) const;

      /**
       */
      void dirInfo(MediaId mediaId,
                   filesystem::DirContent & retlist,
                   const Pathname & dirname, bool dots = true ) const;

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
