/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <iostream>
#include <fstream>

#include "zypp/base/LogTools.h"
#include "zypp/base/Regex.h"
#include "zypp/base/UserRequestException.h"
#include "zypp/ZYppCallbacks.h"
#include "zypp/MediaSetAccess.h"
#include "zypp/PathInfo.h"
//#include "zypp/source/MediaSetAccessReportReceivers.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

IMPL_PTR_TYPE(MediaSetAccess);

///////////////////////////////////////////////////////////////////

  MediaSetAccess::MediaSetAccess(const Url &url,
                                 const Pathname & prefered_attach_point)
      : _url(url)
      , _prefAttachPoint(prefered_attach_point)
  {}

  MediaSetAccess::MediaSetAccess(const std::string & label_r,
                                 const Url &url,
                                 const Pathname & prefered_attach_point)
      : _url(url)
      , _prefAttachPoint(prefered_attach_point)
      , _label( label_r )
  {}

  MediaSetAccess::~MediaSetAccess()
  {
    try
    {
      release();
    }
    catch(...) {} // don't let exception escape a dtor.
  }


  void MediaSetAccess::setVerifier( unsigned media_nr, media::MediaVerifierRef verifier )
  {
    if (_medias.find(media_nr) != _medias.end())
    {
      // the media already exists, set theverifier
      media::MediaAccessId id = _medias[media_nr];
      media::MediaManager media_mgr;
      media_mgr.addVerifier( id, verifier );
      // remove any saved verifier for this media
      _verifiers.erase(media_nr);
      //if (! noattach && ! media_mgr.isAttached(id))
      //media_mgr.attach(id);
    }
    else
    {
      // save the verifier in the map, and set it when
      // the media number is first attached
      _verifiers[media_nr] = verifier;
    }
  }

//       callback::SendReport<source::DownloadFileReport> report;
//       DownloadProgressFileReceiver download_report( report );
//       SourceFactory source_factory;
//       Url file_url( url().asString() + file_r.asString() );
//       report->start( source_factory.createFrom(this), file_url );
//       callback::TempConnect<media::DownloadProgressReport> tmp_download( download_report );
//       Pathname file = provideJustFile( file_r, media_nr, cached, checkonly );
//       report->finish( file_url, source::DownloadFileReport::NO_ERROR, "" );
//       return file;

  void MediaSetAccess::releaseFile( const OnMediaLocation & on_media_file )
  {
    releaseFile( on_media_file.filename(), on_media_file.medianr() );
  }

  void MediaSetAccess::releaseFile( const Pathname & file, unsigned media_nr)
  {
    media::MediaManager media_mgr;
    media::MediaAccessId media;

    media = getMediaAccessId( media_nr);
    DBG << "Going to release file " << file
        << " from media number " << media_nr << endl;

    if ( ! media_mgr.isAttached(media) )
      return; //disattached media is free

    media_mgr.releaseFile (media, file);
  }

  Pathname MediaSetAccess::provideFile( const OnMediaLocation & on_media_file )
  {
    return provideFile( on_media_file.filename(), on_media_file.medianr() );
  }


  Pathname MediaSetAccess::provideFile(const Pathname & file, unsigned media_nr )
  {
    return provideFileInternal( file, media_nr, false, false);
  }

  bool MediaSetAccess::doesFileExist(const Pathname & file, unsigned media_nr )
  {
    callback::SendReport<media::MediaChangeReport> report;
    media::MediaManager media_mgr;

    bool exists = false;

    do
    {
      // get the mediaId, but don't try to attach it here
      media::MediaAccessId media = getMediaAccessId( media_nr);

      try
      {
        DBG << "Cheking if file " << file
            << " from media number " << media_nr << " exists." << endl;
        // try to attach the media
        if ( ! media_mgr.isAttached(media) )
          media_mgr.attachDesiredMedia(media);
        exists = media_mgr.doesFileExist(media, file);
        break;
      }
      catch ( media::MediaException & excp )
      {
        ZYPP_CAUGHT(excp);
        media::MediaChangeReport::Action user;
        do
        {
          DBG << "Media couldn't provide file " << file << " , releasing." << endl;
          try
          {
            media_mgr.release (media);
          }
          catch (const Exception & excpt_r)
          {
              ZYPP_CAUGHT(excpt_r);
              MIL << "Failed to release media " << media << endl;
          }

          // set up the reason
          media::MediaChangeReport::Error reason = media::MediaChangeReport::INVALID;

          if( typeid(excp) == typeid( media::MediaFileNotFoundException )  ||
              typeid(excp) == typeid( media::MediaNotAFileException ) )
          {
            reason = media::MediaChangeReport::NOT_FOUND;
          }
          else if( typeid(excp) == typeid( media::MediaNotDesiredException)  ||
              typeid(excp) == typeid( media::MediaNotAttachedException) )
          {
            reason = media::MediaChangeReport::WRONG;
          }
          else if( typeid(excp) == typeid( media::MediaTimeoutException))
          {
            reason = media::MediaChangeReport::IO_SOFT;
          }

          vector<string> devices;
          unsigned int devindex;
          media_mgr.getDetectedDevices(media, devices, devindex);

          // release all media before requesting another (#336881)
          media_mgr.releaseAll();

          user = report->requestMedia (
              _url,
              media_nr,
              _label,
              reason,
              excp.asUserString(),
              devices, devindex
            );

          MIL << "doesFileExist exception caught, callback answer: " << user << endl;

          if( user == media::MediaChangeReport::ABORT )
          {
            DBG << "Aborting" << endl;
            ZYPP_RETHROW ( excp );
          }
          else if ( user == media::MediaChangeReport::IGNORE )
          {
            DBG << "Skipping" << endl;
	    SkipRequestException nexcp("User-requested skipping of a file");
	    nexcp.remember(excp);
	    ZYPP_THROW(nexcp);
          }
          else if ( user == media::MediaChangeReport::EJECT )
          {
            DBG << "Eject: try to release" << endl;
            media_mgr.releaseAll();
            // eject
            media_mgr.release (media,
              devindex < devices.size() ? devices[devindex] : "");
          }
          else if ( user == media::MediaChangeReport::RETRY  ||
            user == media::MediaChangeReport::CHANGE_URL )
          {
            // retry
            DBG << "Going to try again" << endl;
            // invalidate current media access id
            media_mgr.close(media);
            _medias.erase(media_nr);

            // not attaching, media set will do that for us
            // this could generate uncaught exception (#158620)
            break;
          }
          else
          {
            DBG << "Don't know, let's ABORT" << endl;
            ZYPP_RETHROW ( excp );
          }
        } while( user == media::MediaChangeReport::EJECT );
      }

      // retry or change URL
    } while( true );

    return exists;
  }

  Pathname MediaSetAccess::provideFileInternal(const Pathname & file, unsigned media_nr, bool cached, bool checkonly )
  {
    callback::SendReport<media::MediaChangeReport> report;
    media::MediaManager media_mgr;

    media::MediaAccessId media;

    do
    {
      // get the mediaId, but don't try to attach it here
      media = getMediaAccessId( media_nr);

      try
      {
        DBG << "Going to try to provide file " << file
            << " from media number " << media_nr << endl;
        // try to attach the media
        if ( ! media_mgr.isAttached(media) )
          media_mgr.attachDesiredMedia(media);
        media_mgr.provideFile (media, file, false, false);
        break;
      }
      catch ( media::MediaException & excp )
      {
        ZYPP_CAUGHT(excp);
        media::MediaChangeReport::Action user = media::MediaChangeReport::ABORT;
        unsigned int devindex = 0;
        vector<string> devices;
        media_mgr.getDetectedDevices(media, devices, devindex);

        do
        {
          if (user != media::MediaChangeReport::EJECT) // no use in calling this again
          {
            DBG << "Media couldn't provide file " << file << " , releasing." << endl;
            try
            {
              media_mgr.release(media);
            }
            catch (const Exception & excpt_r)
            {
                ZYPP_CAUGHT(excpt_r);
                MIL << "Failed to release media " << media << endl;
            }
          }

          // set up the reason
          media::MediaChangeReport::Error reason = media::MediaChangeReport::INVALID;

          if( typeid(excp) == typeid( media::MediaFileNotFoundException )  ||
              typeid(excp) == typeid( media::MediaNotAFileException ) )
          {
            reason = media::MediaChangeReport::NOT_FOUND;
          }
          else if( typeid(excp) == typeid( media::MediaNotDesiredException)  ||
              typeid(excp) == typeid( media::MediaNotAttachedException) )
          {
            reason = media::MediaChangeReport::WRONG;
          }


          if (checkonly)
            user  = media::MediaChangeReport::ABORT;
          else
          {
            // release all media before requesting another (#336881)
            media_mgr.releaseAll();

            user = report->requestMedia (
              _url,
              media_nr,
              _label,
              reason,
              excp.asUserString(),
              devices,
              devindex
            );
          }

          MIL << "ProvideFile exception caught, callback answer: " << user << endl;

          if( user == media::MediaChangeReport::ABORT )
          {
            DBG << "Aborting" << endl;
            ZYPP_RETHROW ( excp );
          }
          else if ( user == media::MediaChangeReport::IGNORE )
          {
            DBG << "Skipping" << endl;
	    SkipRequestException nexcp("User-requested skipping of a file");
	    nexcp.remember(excp);
	    ZYPP_THROW(nexcp);
	  }
          else if ( user == media::MediaChangeReport::EJECT )
          {
            DBG << "Eject: try to release" << endl;
            media_mgr.releaseAll();
            // eject
            media_mgr.release (media,
              devindex < devices.size() ? devices[devindex] : "");
          }
          else if ( user == media::MediaChangeReport::RETRY  ||
            user == media::MediaChangeReport::CHANGE_URL )
          {
            // retry
            DBG << "Going to try again" << endl;
            // invalidate current media access id
            media_mgr.close(media);
            _medias.erase(media_nr);

            // not attaching, media set will do that for us
            // this could generate uncaught exception (#158620)
            break;
          }
          else
          {
            DBG << "Don't know, let's ABORT" << endl;
            ZYPP_RETHROW ( excp );
          }
        } while( user == media::MediaChangeReport::EJECT );
      }

      // retry or change URL
    } while( true );

    return media_mgr.localPath( media, file );
  }


  Pathname MediaSetAccess::provideDir(const Pathname & dir,
                                      bool recursive,
                                      unsigned media_nr)
  {
    callback::SendReport<media::MediaChangeReport> report;
    media::MediaManager media_mgr;
    media::MediaAccessId _media;

    do
    {
      // get the mediaId, but don't try to attach it here
      _media = getMediaAccessId(media_nr);

      try
      {
        DBG << "Going to try provide direcotry " << dir
            << (recursive ? " (recursively)" : "")
            << " from media nr. " << media_nr << endl;

        // try to attach the media
        if (!media_mgr.isAttached(_media))
          media_mgr.attachDesiredMedia(_media);

        _media = getMediaAccessId(media_nr); // in case of redirect

        if (recursive)
          media_mgr.provideDirTree(_media, dir);
        else
          media_mgr.provideDir(_media, dir);

        break; // quit the retry loop
      }
      catch (media::MediaException & excp)
      {
        ZYPP_CAUGHT(excp);
        media::MediaChangeReport::Action user;

        do
        {
          DBG << "Media couldn't provide dir " << dir << ", releasing." << endl;
          try
          {
            media_mgr.release (_media);
          }
          catch (const Exception & excpt_r)
          {
            ZYPP_CAUGHT(excpt_r);
            ERR << "Failed to release media " << _media << endl;
          }

          //MIL << "Releasing all medias of all sources" << endl;
          try
          {
            //! \todo do we need replacement for this at all?
            //zypp::SourceManager::sourceManager()->releaseAllSources();
          }
          catch (const zypp::Exception& excpt_r)
          {
            ZYPP_CAUGHT(excpt_r);
            ERR << "Failed to release all sources" << endl;
          }

          // set up the reason
          media::MediaChangeReport::Error reason = media::MediaChangeReport::INVALID;

          if (typeid(excp) == typeid(media::MediaFileNotFoundException)
              || typeid(excp) == typeid(media::MediaNotAFileException))
          {
            reason = media::MediaChangeReport::NOT_FOUND;
          }
          else if (typeid(excp) == typeid( media::MediaNotDesiredException)
              || typeid(excp) == typeid(media::MediaNotAttachedException))
          {
            reason = media::MediaChangeReport::WRONG;
          }

          vector<string> devices;
          unsigned int devindex;
          media_mgr.getDetectedDevices(_media, devices, devindex);

          // release all media before requesting another (#336881)
          media_mgr.releaseAll();

          user = report->requestMedia(_url,
                                      media_nr,
                                      _label,
                                      reason,
                                      excp.asUserString(),
                                      devices,
                                      devindex);

          MIL << "ProvideDir exception caught, callback answer: " << user << endl;

          if (user == media::MediaChangeReport::ABORT)
          {
            DBG << "Aborting" << endl;
            ZYPP_RETHROW ( excp );
          }
          else if (user == media::MediaChangeReport::EJECT)
          {
            DBG << "Eject: try to release" << endl;
            media_mgr.releaseAll();
            // eject
            media_mgr.release (_media,
              devindex < devices.size() ? devices[devindex] : "");
          }
          else if (user == media::MediaChangeReport::RETRY ||
              user == media::MediaChangeReport::CHANGE_URL)
          {
            // retry
            DBG << "Going to try again" << endl;
            // invalidate current media access id
            media_mgr.close(_media);
            _medias.erase(media_nr);

            // not attaching, media set will do that for us
            // this could generate uncaught exception (#158620)

            break;
          }
          else
          {
            DBG << "Don't know, let's ABORT" << endl;

            ZYPP_RETHROW (excp);
          }
        }
        while (user == media::MediaChangeReport::EJECT);
      }
      // retry or change URL
    }
    while (true);

    return media_mgr.localPath(_media, dir);
  }

  media::MediaAccessId MediaSetAccess::getMediaAccessId (media::MediaNr medianr)
  {
    media::MediaManager media_mgr;

    if (_medias.find(medianr) != _medias.end())
    {
      media::MediaAccessId id = _medias[medianr];
      //if (! noattach && ! media_mgr.isAttached(id))
      //media_mgr.attach(id);
      return id;
    }
    Url url;
    url = rewriteUrl (_url, medianr);
    media::MediaAccessId id = media_mgr.open(url, _prefAttachPoint);
    _medias[medianr] = id;

    try
    {
      if (_verifiers.find(medianr) != _verifiers.end())
      {
        // a verifier is set for this media
        // FIXME check the case where the verifier exists
        // but we have no access id for the media
        media::MediaAccessId id = _medias[medianr];
        media::MediaManager media_mgr;
        media_mgr.delVerifier(id);
        media_mgr.addVerifier( id, _verifiers[medianr] );
        // remove any saved verifier for this media
        _verifiers.erase(medianr);
      }
    }
    catch ( const Exception &e )
    {
      ZYPP_CAUGHT(e);
      WAR << "Verifier not found" << endl;
    }

    return id;
  }


  Url MediaSetAccess::rewriteUrl (const Url & url_r, const media::MediaNr medianr)
  {
    std::string scheme = url_r.getScheme();
    if (scheme == "cd" || scheme == "dvd")
      return url_r;

    DBG << "Rewriting url " << url_r << endl;

    if( scheme == "iso")
    {
      // TODO the iso parameter will not be required in the future, this
      // code has to be adapted together with the MediaISO change.
      // maybe some MediaISOURL interface should be used.
      std::string isofile = url_r.getQueryParam("iso");
      str::regex e("^(.*)(cd|dvd)[0-9]+\\.iso$", str::regex::icase);

      str::smatch what;
      if(str::regex_match(isofile, what, e))
      {
        Url url( url_r);
        isofile = what[1] + what[2] + str::numstring(medianr) + ".iso";
        url.setQueryParam("iso", isofile);
        DBG << "Url rewrite result: " << url << endl;
        return url;
      }
    }
    else
    {
      std::string pathname = url_r.getPathName();
      str::regex e("^(.*)(cd|dvd)[0-9]+(/)?$", str::regex::icase);
      str::smatch what;
      if(str::regex_match(pathname, what, e))
      {
        Url url( url_r);
        pathname = what[1] + what[2] + str::numstring(medianr) + what[3];
        url.setPathName(pathname);
        DBG << "Url rewrite result: " << url << endl;
        return url;
      }
    }
    return url_r;
  }

  void MediaSetAccess::release()
  {
    DBG << "Releasing all media IDs held by this MediaSetAccess" << endl;
    media::MediaManager manager;
    for (MediaMap::const_iterator m = _medias.begin(); m != _medias.end(); ++m)
      manager.release(m->second, "");
  }

  std::ostream & MediaSetAccess::dumpOn( std::ostream & str ) const
  {
    str << "MediaSetAccess (URL='" << _url << "', attach_point_hint='" << _prefAttachPoint << "')";
    return str;
  }

//     media::MediaVerifierRef MediaSetAccess::verifier(unsigned media_nr)
//     { return media::MediaVerifierRef(new media::NoVerifier()); }

/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
