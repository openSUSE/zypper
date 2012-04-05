/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/RepoProvideFile.cc
 *
*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>

#include "zypp/base/Gettext.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/UserRequestException.h"
#include "zypp/repo/RepoProvideFile.h"
#include "zypp/ZYppCallbacks.h"
#include "zypp/MediaSetAccess.h"
#include "zypp/ZConfig.h"
#include "zypp/ZYppFactory.h"
#include "zypp/repo/SUSEMediaVerifier.h"
#include "zypp/repo/RepoException.h"

#include "zypp/repo/SUSEMediaVerifier.h"
#include "zypp/repo/RepoException.h"
#include "zypp/FileChecker.h"
#include "zypp/Fetcher.h"

using std::endl;
using std::set;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	provideFile
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace
    { /////////////////////////////////////////////////////////////////

      /** Hack to extract progress information from media::DownloadProgressReport.
       * We redirect the static report triggered from RepoInfo::provideFile
       * to feed the ProvideFilePolicy callbacks in addition to any connected
       * media::DownloadProgressReport.
      */
      struct DownloadFileReportHack : public callback::ReceiveReport<media::DownloadProgressReport>
      {
	typedef callback::ReceiveReport<ReportType> BaseType;
	typedef function<bool(int)>                 RedirectType;

	DownloadFileReportHack( RedirectType redirect_r )
	: _oldRec( Distributor::instance().getReceiver() )
	, _redirect( redirect_r )
	{ connect(); }
	~DownloadFileReportHack()
	{ if ( _oldRec ) Distributor::instance().setReceiver( *_oldRec ); else Distributor::instance().noReceiver(); }

	virtual void start( const Url & file, Pathname localfile )
	{
	  if ( _oldRec )
	    _oldRec->start( file, localfile );
	  else
	    BaseType::start( file, localfile );
	}

	virtual bool progress( int value, const Url & file, double dbps_avg = -1, double dbps_current = -1 )
	{
	  bool ret = true;
	  if ( _oldRec )
	    ret &= _oldRec->progress( value, file, dbps_avg, dbps_current );
          if ( _redirect )
            ret &= _redirect( value );
	  return ret;
	}

	virtual Action problem( const Url & file, Error error, const std::string & description )
	{
	  if ( _oldRec )
	    return _oldRec->problem( file, error, description );
	  return BaseType::problem( file, error, description );
	}
	virtual void finish( const Url & file, Error error, const std::string & reason )
	{
	  if ( _oldRec )
	    _oldRec->finish( file, error, reason );
	  else
	    BaseType::finish( file, error, reason );
	}

	private:
	  Receiver * _oldRec;
	  RedirectType _redirect;
      };

      /////////////////////////////////////////////////////////////////
    } // namespace
    ///////////////////////////////////////////////////////////////////

    ManagedFile provideFile( RepoInfo repo_r,
                             const OnMediaLocation & loc_r,
                             const ProvideFilePolicy & policy_r )
    {
      RepoMediaAccess access;
      return access.provideFile(repo_r, loc_r, policy_r );
    }

    ///////////////////////////////////////////////////////////////////
    class RepoMediaAccess::Impl
    {
    public:
      Impl( const ProvideFilePolicy & defaultPolicy_r )
        : _defaultPolicy( defaultPolicy_r )
      {}

      ~Impl()
      {
        std::map<Url, shared_ptr<MediaSetAccess> >::iterator it;
        for ( it = _medias.begin();
              it != _medias.end();
              ++it )
        {
          it->second->release();
        }
      }

      /** Provide a MediaSetAccess for \c url with label and verifyer adjusted.
       *
       * As the same url (e.g. \c 'dvd:///' ) might be used for multiple repos
       * we must always adjust the repo specific data (label,verifyer).
       *
       * \todo This mixture of media and repos specific data is fragile.
      */
      shared_ptr<MediaSetAccess> mediaAccessForUrl( const Url &url, RepoInfo repo )
      {
        std::map<Url, shared_ptr<MediaSetAccess> >::const_iterator it;
        it = _medias.find(url);
        shared_ptr<MediaSetAccess> media;
        if ( it != _medias.end() )
        {
          media = it->second;
        }
        else
        {
          media.reset( new MediaSetAccess(url) );
          _medias[url] = media;
        }
        setVerifierForRepo( repo, media );
        return media;
      }

      private:
        void setVerifierForRepo( RepoInfo repo, shared_ptr<MediaSetAccess> media )
        {
          // Always set the MediaSetAccess label.
          media->setLabel( repo.name() );

          // set a verifier if the repository has it

          Pathname mediafile = repo.metadataPath() + "/media.1/media";
          if ( ! repo.metadataPath().empty() )
          {
            if ( PathInfo(mediafile).isExist() )
            {
              std::map<shared_ptr<MediaSetAccess>, RepoInfo>::const_iterator it;
              it = _verifier.find(media);
              if ( it != _verifier.end() )
              {
                if ( it->second.alias() == repo.alias() )
                {
                  // this media is already using this repo verifier
                  return;
                }
              }

              std::ifstream str(mediafile.asString().c_str());
              std::string vendor;
              std::string mediaid;
              std::string buffer;
              if ( str )
              {
                getline(str, vendor);
                getline(str, mediaid);
                getline(str, buffer);

                unsigned media_nr = str::strtonum<unsigned>(buffer);
                MIL << "Repository '" << repo.alias() << "' has " << media_nr << " medias"<< endl;

                for ( unsigned i=1; i <= media_nr; ++i )
                {
                  media::MediaVerifierRef verifier( new repo::SUSEMediaVerifier( vendor, mediaid, i ) );

                  media->setVerifier( i, verifier);
                }
                _verifier[media] = repo;
              }
              else
              {
                ZYPP_THROW(RepoMetadataException(repo));
              }
            }
            else
            {
              WAR << "No media verifier for repo '" << repo.alias() << "' media/media.1 does not exist in '" << repo.metadataPath() << "'" << endl;
            }
          }
          else
          {
            WAR << "'" << repo.alias() << "' metadata path is empty. Can't set verifier. Probably this repository does not come from RepoManager." << endl;
          }
        }

      private:
        std::map<shared_ptr<MediaSetAccess>, RepoInfo> _verifier;
        std::map<Url, shared_ptr<MediaSetAccess> > _medias;

      public:
        ProvideFilePolicy _defaultPolicy;
    };
    ///////////////////////////////////////////////////////////////////


    RepoMediaAccess::RepoMediaAccess( const ProvideFilePolicy & defaultPolicy_r )
      : _impl( new Impl( defaultPolicy_r ) )
    {}

    RepoMediaAccess::~RepoMediaAccess()
    {}

    void RepoMediaAccess::setDefaultPolicy( const ProvideFilePolicy & policy_r )
    { _impl->_defaultPolicy = policy_r; }

    const ProvideFilePolicy & RepoMediaAccess::defaultPolicy() const
    { return _impl->_defaultPolicy; }

    ManagedFile RepoMediaAccess::provideFile( RepoInfo repo_r,
                                              const OnMediaLocation & loc_r,
                                              const ProvideFilePolicy & policy_r )
    {
      MIL << loc_r << endl;
      // Arrange DownloadFileReportHack to recieve the source::DownloadFileReport
      // and redirect download progress triggers to call the ProvideFilePolicy
      // callback.
      DownloadFileReportHack dumb( bind( mem_fun_ref( &ProvideFilePolicy::progress ), ref( policy_r ), _1 ) );

      RepoException repo_excpt(repo_r,
			       str::form(_("Can't provide file '%s' from repository '%s'"),
                               loc_r.filename().c_str(),
                               repo_r.alias().c_str() ) );

      if ( repo_r.baseUrlsEmpty() )
      {
        repo_excpt.remember(RepoException(_("No url in repository.")));
        ZYPP_THROW(repo_excpt);
      }

      Fetcher fetcher;
      fetcher.addCachePath( repo_r.packagesPath() );
      MIL << "Added cache path " << repo_r.packagesPath() << endl;

      // Test whether download destination is writable, if not
      // switch into the tmpspace (e.g. bnc#755239, download and
      // install srpms as user).
      Pathname destinationDir( repo_r.packagesPath() );
      if ( ! PathInfo( destinationDir ).userMayW() )
      {
        WAR << "Destination dir '" << destinationDir << "' is not user writable, using tmp space." << endl;
        destinationDir = getZYpp()->tmpPath() / destinationDir;
	assert_dir( destinationDir );
        fetcher.addCachePath( destinationDir );
        MIL << "Added cache path " << destinationDir << endl;
      }

      for ( RepoInfo::urls_const_iterator it = repo_r.baseUrlsBegin();
            it != repo_r.baseUrlsEnd();
            /* incremented in the loop */ )
      {
        Url url( *it );
        ++it;
        try
        {
          MIL << "Providing file of repo '" << repo_r.alias()
              << "' from " << url << endl;
          shared_ptr<MediaSetAccess> access = _impl->mediaAccessForUrl( url, repo_r );

	  fetcher.enqueue( loc_r );

	  // FIXME: works for packages only
	  fetcher.start( destinationDir, *access );

	  // reached if no exception has been thrown, so this is the correct file
          ManagedFile ret( destinationDir + loc_r.filename() );

          std::string scheme( url.getScheme() );
          if ( !repo_r.keepPackages() )
          {
            ret.setDispose( filesystem::unlink );
          }

          if ( loc_r.checksum().empty() )
          {
            // no checksum in metadata
            WAR << "No checksum in metadata " << loc_r << endl;
          }
          else
          {
            std::ifstream input( ret->asString().c_str() );
            CheckSum retChecksum( loc_r.checksum().type(), input );
            input.close();

            if ( loc_r.checksum() != retChecksum )
            {
              // failed integity check
              std::ostringstream err;
              err << "File " << ret << " fails integrity check. Expected: [" << loc_r.checksum() << "] Got: [";
              if ( retChecksum.empty() )
                err << "Failed to compute checksum";
              else
                err << retChecksum;
              err << "]";

              WAR << err.str() << endl;

              if ( policy_r.failOnChecksumError() )
                ZYPP_THROW( FileCheckException( err.str() ) );
              else
                WAR << "NO failOnChecksumError: " << err.str() << endl;
            }
          }

          MIL << "provideFile at " << ret << endl;
          return ret;
        }
        catch ( const SkipRequestException &e )
        {
          ZYPP_CAUGHT( e );
          ZYPP_RETHROW(e);
        }
        catch ( const AbortRequestException &e )
        {
          ZYPP_CAUGHT( e );
          ZYPP_RETHROW(e);
        }
        catch ( const Exception &e )
        {
          ZYPP_CAUGHT( e );

          repo_excpt.remember(e);

          WAR << "Trying next url" << endl;
          continue;
        }
      } // iteration over urls

      ZYPP_THROW(repo_excpt);
      return ManagedFile(); // not reached
    }

    /////////////////////////////////////////////////////////////////
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
