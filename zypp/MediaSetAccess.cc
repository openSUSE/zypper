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
    }
    else
    {
      // save the verifier in the map, and set it when
      // the media number is first attached
      _verifiers[media_nr] = verifier;
    }
  }

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

  void MediaSetAccess::dirInfo( filesystem::DirContent &retlist, const Pathname &dirname,
                                bool dots, unsigned media_nr )
  {
    media::MediaManager media_mgr;
    media::MediaAccessId media;
    media = getMediaAccessId(media_nr);

    // try to attach the media
    if ( ! media_mgr.isAttached(media) )
        media_mgr.attach(media);

    media_mgr.dirInfo(media, retlist, dirname, dots);
  }

  struct ProvideFileOperation
  {
    Pathname result;
    void operator()( media::MediaAccessId media, const Pathname &file )
    {
      media::MediaManager media_mgr;
      media_mgr.provideFile(media, file);
      result = media_mgr.localPath(media, file);
    }
  };

  struct ProvideDirTreeOperation
  {
    Pathname result;
    void operator()( media::MediaAccessId media, const Pathname &file )
    {
      media::MediaManager media_mgr;
      media_mgr.provideDirTree(media, file);
      result = media_mgr.localPath(media, file);
    }
  };

  struct ProvideDirOperation
  {
    Pathname result;
    void operator()( media::MediaAccessId media, const Pathname &file )
    {
      media::MediaManager media_mgr;
      media_mgr.provideDir(media, file);
      result = media_mgr.localPath(media, file);
    }
  };

  struct ProvideFileExistenceOperation
  {
    bool result;
    ProvideFileExistenceOperation()
        : result(false)
    {}

    void operator()( media::MediaAccessId media, const Pathname &file )
    {
      media::MediaManager media_mgr;
      result = media_mgr.doesFileExist(media, file);
    }
  };



  Pathname MediaSetAccess::provideFile( const OnMediaLocation & resource, ProvideFileOptions options, const Pathname &deltafile )
  {
    ProvideFileOperation op;
    provide( boost::ref(op), resource, options, deltafile );
    return op.result;
  }

  Pathname MediaSetAccess::provideFile(const Pathname & file, unsigned media_nr, ProvideFileOptions options )
  {
    OnMediaLocation resource;
    ProvideFileOperation op;
    resource.setLocation(file, media_nr);
    provide( boost::ref(op), resource, options, Pathname() );
    return op.result;
  }

  bool MediaSetAccess::doesFileExist(const Pathname & file, unsigned media_nr )
  {
    ProvideFileExistenceOperation op;
    OnMediaLocation resource;
    resource.setLocation(file, media_nr);
    provide( boost::ref(op), resource, PROVIDE_DEFAULT, Pathname());
    return op.result;
  }

  void MediaSetAccess::provide( ProvideOperation op,
                                const OnMediaLocation &resource,
                                ProvideFileOptions options,
                                const Pathname &deltafile )
  {
    Pathname file(resource.filename());
    unsigned media_nr(resource.medianr());

    callback::SendReport<media::MediaChangeReport> report;
    media::MediaManager media_mgr;

    media::MediaAccessId media;

    do
    {
      // get the mediaId, but don't try to attach it here
      media = getMediaAccessId( media_nr);
      bool deltafileset = false;

      try
      {
        DBG << "Going to try to provide " << (resource.optional() ? "optional" : "") << " file " << file
            << " from media number " << media_nr << endl;
        // try to attach the media
        if ( ! media_mgr.isAttached(media) )
          media_mgr.attach(media);
	media_mgr.setDeltafile(media, deltafile);
	deltafileset = true;
        op(media, file);
	media_mgr.setDeltafile(media, Pathname());
        break;
      }
      catch ( media::MediaException & excp )
      {
        ZYPP_CAUGHT(excp);
	if (deltafileset)
	  media_mgr.setDeltafile(media, Pathname());
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
          else if( typeid(excp) == typeid( media::MediaTimeoutException) ||
                   typeid(excp) == typeid( media::MediaTemporaryProblemException))
          {
            reason = media::MediaChangeReport::IO_SOFT;
          }

          // Propagate the original error if _no_ callback receiver is connected, or
	  // non_interactive mode (for optional files) is used (except for wrong media).
          if ( ! callback::SendReport<media::MediaChangeReport>::connected()
	     || (( options & PROVIDE_NON_INTERACTIVE ) && reason != media::MediaChangeReport::WRONG ) )
          {
              MIL << "Can't provide file. Non-Interactive mode." << endl;
              ZYPP_RETHROW(excp);
          }
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
            AbortRequestException aexcp("Aborting requested by user");
            aexcp.remember(excp);
            ZYPP_THROW(aexcp);
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
	    try
	    {
	      media_mgr.releaseAll();
	      media_mgr.release (media, devindex < devices.size() ? devices[devindex] : "");
	    }
	    catch ( const Exception & e)
	    {
	      ZYPP_CAUGHT(e);
	    }
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
  }

  Pathname MediaSetAccess::provideDir(const Pathname & dir,
                                      bool recursive,
                                      unsigned media_nr,
                                      ProvideFileOptions options )
  {
    OnMediaLocation resource;
    resource.setLocation(dir, media_nr);
    if ( recursive )
    {
        ProvideDirTreeOperation op;
        provide( boost::ref(op), resource, options, Pathname());
        return op.result;
    }
    ProvideDirOperation op;
    provide( boost::ref(op), resource, options, Pathname());
    return op.result;
  }

  media::MediaAccessId MediaSetAccess::getMediaAccessId (media::MediaNr medianr)
  {
    media::MediaManager media_mgr;

    if (_medias.find(medianr) != _medias.end())
    {
      media::MediaAccessId id = _medias[medianr];
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
      str::regex e("^(.*)(cd|dvd|media)[0-9]+\\.iso$", str::regex::icase);

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
      str::regex e("^(.*)(cd|dvd|media)[0-9]+(/)?$", str::regex::icase);
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

/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
