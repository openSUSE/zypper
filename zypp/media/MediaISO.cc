/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaISO.cc
 *
 */
#include "zypp/media/MediaISO.h"
#include "zypp/base/Logger.h"
#include "zypp/media/Mount.h"

#include <iostream>

//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
  namespace media
  { //////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    // MediaISO Url:
    //
    //   Schema: iso
    //   Path name: subdir to the location of desired files inside
    //              of the ISO.
    //   Query parameters:
    //     url:        The iso filename source media url pointing
    //                 to a directory containing the ISO file.
    //     mnt:        Prefered attach point for source media url.
    //     iso:        The name of the iso file.
    //     filesystem: Optional, defaults to "auto".
    //
    ///////////////////////////////////////////////////////////////////
    MediaISO::MediaISO(const Url      &url_r,
                       const Pathname &attach_point_hint_r)
      : MediaHandler(url_r, attach_point_hint_r,
                     url_r.getPathName(), // urlpath below attachpoint
                     false)               // does_download
    {
      _isofile    = _url.getQueryParam("iso");
      if( _isofile.empty())
      {
        ERR << "Media url does not contain iso filename" << std::endl;
        ZYPP_THROW(MediaBadUrlEmptyDestinationException(_url));
      }

      _filesystem = _url.getQueryParam("filesystem");
      if( _filesystem.empty())
        _filesystem = "auto";

      zypp::Url src;
      try
      {
        src = _url.getQueryParam("url");
      }
      catch(const zypp::url::UrlException &e)
      {
        ZYPP_CAUGHT(e);
        ERR << "Unable to parse iso filename source media url" << std::endl;
        ZYPP_THROW(MediaBadUrlException(_url));
      }
      if( !src.isValid())
      {
        ERR << "Invalid iso filename source media url" << std::endl;
        ZYPP_THROW(MediaBadUrlException(src));
      }
      if( src.getScheme() == "iso")
      {
        ERR << "ISO filename source media url with iso scheme (nested iso): "
            << src.asString() << std::endl;
        ZYPP_THROW(MediaUnsupportedUrlSchemeException(src));
      }
#if 0
      else
      if( src.getScheme() == "ftp"   ||
          src.getScheme() == "http"  ||
          src.getScheme() == "https")
      {
        ERR << "ISO filename source media url scheme is not supported: "
            << src.asString() << endl;
        ZYPP_THROW(MediaUnsupportedUrlSchemeException(src));
      }
#endif

      MediaManager manager;

      _parentId = manager.open(src, _url.getQueryParam("mnt"));

      MIL << "MediaISO::MediaISO(" << url_r << ", "
          << attach_point_hint_r << ")" << std::endl;
    }

    // ---------------------------------------------------------------
    MediaISO::~MediaISO()
    {
      try
      {
        release();
      }
      catch( ... )
      {}
    }

    // ---------------------------------------------------------------
    bool
    MediaISO::isAttached() const
    {
      MediaManager manager;
      return checkAttached(false, false);
      // FIXME: what's about manager.isAttached(_parentId) ?
    }

    // ---------------------------------------------------------------
    void MediaISO::attachTo(bool next)
    {
      if(next)
        ZYPP_THROW(MediaNotSupportedException(_url));

      MediaManager manager;
      manager.attach(_parentId, false);

      try
      {
        manager.provideFile(_parentId, _isofile);
      }
      catch(const MediaException &e1)
      {
        ZYPP_CAUGHT(e1);
        try
        {
          manager.release(_parentId, false);
        }
        catch(const MediaException &e2)
        {
          ZYPP_CAUGHT(e2);
        }
        ZYPP_THROW(MediaMountException(
          "Unable to find iso filename on source media",
          _url.asString(), attachPoint().asString()
        ));
      }

      Pathname isofile = manager.localPath(_parentId, _isofile);
      PathInfo isoinfo( isofile, PathInfo::LSTAT);
      if( !isoinfo.isFile())
      {
        ZYPP_THROW(MediaNotSupportedException(_url));
      }

      MediaSourceRef media( new MediaSource(
        "iso", isofile.asString()
      ));

      AttachedMedia  ret( findAttachedMedia( media));
      if( ret.mediaSource &&
          ret.attachPoint &&
          !ret.attachPoint->empty())
      {
        DBG << "Using a shared media "
            << ret.mediaSource->name
            << " attached on "
            << ret.attachPoint->path
            << std::endl;
        removeAttachPoint();
        setAttachPoint(ret.attachPoint);
        setMediaSource(ret.mediaSource);
        return;
      }

      std::string mountpoint = attachPoint().asString();
      if( !isUseableAttachPoint(attachPoint()))
      {
        mountpoint = createAttachPoint().asString();
        if( mountpoint.empty())
          ZYPP_THROW( MediaBadAttachPointException(url()));
        setAttachPoint( mountpoint, true);
      }

      std::string mountopts("ro,loop");

      Mount mount;
      mount.mount(isofile.asString(), mountpoint,
                  _filesystem, mountopts);

      setMediaSource(media);

      // wait for /etc/mtab update ...
      // (shouldn't be needed)
      int limit = 10;
      bool mountsucceeded;
      while( !(mountsucceeded=isAttached()) && limit--)
      {
        sleep(1);
      }

      if( !mountsucceeded)
      {
        setMediaSource(MediaSourceRef());
        try
        {
          mount.umount(attachPoint().asString());
          manager.release(_parentId);
        }
        catch (const MediaException & excpt_r)
        {
          ZYPP_CAUGHT(excpt_r);
        }
        ZYPP_THROW(MediaMountException(isofile.asString(), mountpoint,
          "Unable to verify that the media was mounted"
        ));
      }
    }

    // ---------------------------------------------------------------
    void MediaISO::releaseFrom(bool eject)
    {
      Mount mount;
      mount.umount(attachPoint().asString());

      MediaManager manager;
      manager.release(_parentId);
    }

    // ---------------------------------------------------------------
    void MediaISO::getFile(const Pathname &filename) const
    {
      MediaHandler::getFile(filename);
    }

    // ---------------------------------------------------------------
    void MediaISO::getDir(const Pathname &dirname,
                           bool            recurse_r) const
    {
      MediaHandler::getDir(dirname, recurse_r);
    }

    // ---------------------------------------------------------------
    void MediaISO::getDirInfo(std::list<std::string> &retlist,
                               const Pathname         &dirname,
                               bool                    dots) const
    {
      MediaHandler::getDirInfo( retlist, dirname, dots );
    }

    // ---------------------------------------------------------------
    void MediaISO::getDirInfo(filesystem::DirContent &retlist,
                               const Pathname         &dirname,
                               bool                    dots) const
    {
      MediaHandler::getDirInfo(retlist, dirname, dots);
    }


    //////////////////////////////////////////////////////////////////
  } // namespace media
  ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////

// vim: set ts=2 sts=2 sw=2 ai et:

