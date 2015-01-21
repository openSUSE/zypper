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
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/media/Mount.h"

#include "zypp/media/MediaISO.h"


#define LOSETUP_TOOL_PATH "/sbin/losetup"

using std::string;
using std::endl;

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
      MIL << "MediaISO::MediaISO(" << url_r << ", "
          << attach_point_hint_r << ")" << std::endl;

      _isofile    = _url.getQueryParam("iso");
      if( _isofile.empty())
      {
        ERR << "Media url does not contain iso filename" << std::endl;
        ZYPP_THROW(MediaBadUrlEmptyDestinationException(_url));
      }

      _filesystem = _url.getQueryParam("filesystem");
      if( _filesystem.empty())
        _filesystem = "auto";

      std::string arg;
      zypp::Url   src;
      try
      {
        // this percent-decodes the query parameter, it must be later encoded
        // again before used in a Url object
        arg = _url.getQueryParam("url");
        if( arg.empty() && _isofile.dirname().absolute())
        {
          src = std::string("dir:///");
          src.setPathName(url::encode(_isofile.dirname().asString(), URL_SAFE_CHARS));
          _isofile = _isofile.basename();
        }
        else
        {
          src = url::encode(arg, URL_SAFE_CHARS);
        }
      }
      catch(const zypp::url::UrlException &e)
      {
        ZYPP_CAUGHT(e);
        ERR << "Unable to parse iso filename source media url" << std::endl;
	MediaBadUrlException ne(_url);
	ne.remember(e);
	ZYPP_THROW(ne);
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
      else
      if( !(src.getScheme() == "hd"   ||
            src.getScheme() == "dir"  ||
            src.getScheme() == "file" ||
            src.getScheme() == "nfs"  ||
            src.getScheme() == "nfs4" ||
            src.getScheme() == "smb"  ||
            src.getScheme() == "cifs"))
      {
        ERR << "ISO filename source media url scheme is not supported: "
            << src.asString() << std::endl;
        ZYPP_THROW(MediaUnsupportedUrlSchemeException(src));
      }

      MediaManager manager;

      _parentId = manager.open(src, _url.getQueryParam("mnt"));
    }

    // ---------------------------------------------------------------
    MediaISO::~MediaISO()
    {
      try
      {
        release();

        if( _parentId)
        {
          DBG << "Closing parent handler..." << std::endl;
          MediaManager manager;
          if(manager.isOpen(_parentId))
            manager.close(_parentId);
          _parentId = 0;
        }
      }
      catch( ... )
      {}
    }

    // ---------------------------------------------------------------
    bool
    MediaISO::isAttached() const
    {
      return checkAttached(false);
    }

    // ---------------------------------------------------------------
    string MediaISO::findUnusedLoopDevice()
    {
      const char* argv[] =
      {
        LOSETUP_TOOL_PATH,
        "-f",
        NULL
      };
      ExternalProgram losetup(argv, ExternalProgram::Stderr_To_Stdout);

      string out = losetup.receiveLine();
      string device = out.substr(0, out.size() - 1); // remove the trailing endl
      for(; out.length(); out = losetup.receiveLine())
        DBG << "losetup: " << out;

      if (losetup.close() != 0)
      {
        ERR << LOSETUP_TOOL_PATH " failed to find an unused loop device." << std::endl;
        ZYPP_THROW(MediaNoLoopDeviceException(_url));
      }

      DBG << "found " << device << endl;
      return device;
    }

    // ---------------------------------------------------------------
    void MediaISO::attachTo(bool next)
    {
      if(next)
        ZYPP_THROW(MediaNotSupportedException(_url));

      MediaManager manager;
      manager.attach(_parentId);

      try
      {
        manager.provideFile(_parentId, _isofile);
      }
      catch(const MediaException &e1)
      {
        ZYPP_CAUGHT(e1);
        try
        {
          manager.release(_parentId);
        }
        catch(const MediaException &e2)
        {
          ZYPP_CAUGHT(e2);
        }

	MediaMountException e3(
          "Unable to find iso filename on source media",
          _url.asString(), attachPoint().asString()
        );
	e3.remember(e1);
	ZYPP_THROW(e3);
      }

      // if the provided file is a symlink, expand it (#274651)
      // (this will probably work only for file/dir and cd/dvd schemes)
      Pathname isofile = expandlink(manager.localPath(_parentId, _isofile));
      if( isofile.empty() || !PathInfo(isofile).isFile())
      {
        ZYPP_THROW(MediaNotSupportedException(_url));
      }

      //! \todo make this thread-safe - another thread might pick up the same device
      string loopdev = findUnusedLoopDevice(); // (bnc #428009)

      MediaSourceRef media( new MediaSource("iso",  loopdev));
      PathInfo dinfo(loopdev);
      if( dinfo.isBlk())
      {
        media->maj_nr = dinfo.devMajor();
        media->min_nr = dinfo.devMinor();
      }
      else
        ERR << loopdev << " is not a block device" << endl;

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

      std::string mountopts("ro,loop=" + loopdev);

      Mount mount;
      mount.mount(isofile.asString(), mountpoint,
                  _filesystem, mountopts);

      setMediaSource(media);

      // wait for /etc/mtab update ...
      // (shouldn't be needed)
      int limit = 3;
      bool mountsucceeded;
      while( !(mountsucceeded=isAttached()) && --limit)
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
        ZYPP_THROW(MediaMountException(
          "Unable to verify that the media was mounted",
          isofile.asString(), mountpoint
        ));
      }
    }

    // ---------------------------------------------------------------

    void MediaISO::releaseFrom(const std::string & ejectDev)
    {
      Mount mount;
      mount.umount(attachPoint().asString());

      if( _parentId)
      {
        // Unmounting the iso already succeeded,
        // so don't let exceptions escape.
        MediaManager manager;
        try
        {
          manager.release(_parentId);
        }
        catch ( const Exception & excpt_r )
        {
          ZYPP_CAUGHT( excpt_r );
          WAR << "Not been able to cleanup the parent mount." << endl;
        }
      }
      // else:
      // the media manager has reset the _parentId
      // and will destroy the parent handler itself.
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

    bool MediaISO::getDoesFileExist( const Pathname & filename ) const
    {
      return MediaHandler::getDoesFileExist( filename );
    }

    //////////////////////////////////////////////////////////////////
  } // namespace media
  ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////

// vim: set ts=2 sts=2 sw=2 ai et:

