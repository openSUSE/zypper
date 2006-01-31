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
      **
      ** Do we need this?
      ** Returns the (cached) count of physical media
      ** (e.g. nr CDs on SLES10) of the set.
      **
      virtual MediaNr
      mediaCount() const { return 0; }
      */

      /*
      ** Check if the specified attached media in the source
      ** contains desired media number (e.g. SLES10 CD1).
      ** If not:
      **   - ask user for correct media
      **   - it will return retry/abort/....,
      **     behave according to it
      ** TODO: exact workflow, need callbacks?
      */
      virtual bool
      isDesiredMedia(MediaAccessRef &ref, MediaNr mediaNr)
      {
        return false;
      }

      /*
      ** FIXME: signal function to trigger user interactions?
      */
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
     * FIXME: may have static methods... perhaps we should
     *        use a zypp::media::manager namespace instead?
     */
    class MediaManager: public zypp::base::NonCopyable
    {
    public:
      MediaManager();
      ~MediaManager();

      /**
       * open the media, return the ID, throw exception on fail
       */
      MediaId
      open(const Url &url /*, preferedAttachPoint, ... */);

      /**
       * close the media
       */
      void
      close(MediaId id);

      /**
       * attach the media using the concrete handler
       */
      void attach(MediaId mediaId, bool next = false);

      /**
       * Release the attached media and optionally eject.
       */
      void release(MediaId mediaId, bool eject = false);

      /**
       * Add verifier for specified media id.
       */
      void addVerifier(MediaId mediaId, MediaVerifierRef &ref);

      /**
       * Remove verifier for specified media id.
       */
      void delVerifier(MediaId mediaId);

      /**
       * Provide provide file denoted by relative path below of the
       * 'attach point' of the specified media and the path prefix
       * on the media.
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
       */
      void provideFile(MediaId mediaId,
                       MediaNr mediaNr,
                       const Pathname &filename,
                       bool            cached    = false,
                       bool            checkonly = false) const;

      /*
      ** FIXME: other from MediaHandler/MediaAccess interface...
      */

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
