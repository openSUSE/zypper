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
#include "zypp/repo/SUSEMediaVerifier.h"
#include "zypp/repo/RepoException.h"

#include "zypp/repo/SUSEMediaVerifier.h"
#include "zypp/repo/RepoException.h"

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

      /** Hack to extract progress information from source::DownloadFileReport.
       * We redirect the static report triggered from Repository::provideFile
       * to feed the ProvideFilePolicy callbacks.
      */
      struct DownloadFileReportHack : public callback::ReceiveReport<repo::RepoReport>
      {
        virtual bool progress( const ProgressData &progress )
        {
          if ( _redirect )
            return _redirect( progress.val() );
          return true;
        }
        function<bool ( int )> _redirect;
      };

      /////////////////////////////////////////////////////////////////
    } // namespace
    ///////////////////////////////////////////////////////////////////

    ManagedFile provideFile( Repository repo_r,
                             const OnMediaLocation & loc_r,
                             const ProvideFilePolicy & policy_r )
    {
      RepoMediaAccess access;
      return access.provideFile(repo_r, loc_r, policy_r );
    }

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

      shared_ptr<MediaSetAccess> mediaAccessForUrl( const Url &url )
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
        return media;
      }

      void setVerifierForRepo( Repository repo, shared_ptr<MediaSetAccess> media )
      {
        RepoInfo info = repo.info();
        // set a verifier if the repository has it
        Pathname mediafile = info.metadataPath() + "/media.1/media";
        if ( ! mediafile.empty() )
        {
          if ( PathInfo(mediafile).isExist() )
          {
            std::map<shared_ptr<MediaSetAccess>, Repository>::const_iterator it;
            it = _verifier.find(media);
            if ( it != _verifier.end() )
            {
              if ( it->second == repo )
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
              MIL << "Repository '" << info.alias() << "' has " << media_nr << " medias"<< endl;

              for ( unsigned i=1; i <= media_nr; ++i )
              {
                media::MediaVerifierRef verifier( new repo::SUSEMediaVerifier( vendor, mediaid, i ) );

                media->setVerifier( i, verifier);
              }
              _verifier[media] = repo;
            }
            else
            {
              ZYPP_THROW(RepoMetadataException(info));
            }
          }
          else
          {
            WAR << "No media verifier for repo '" << info.alias() << "'" << endl;
          }
        }
        else
        {
          MIL << "Unknown metadata path for repo '" << info.alias() << "'. Can't set media verifier."<< endl;
        }
      }

      std::map<shared_ptr<MediaSetAccess>, Repository> _verifier;
      std::map<Url, shared_ptr<MediaSetAccess> > _medias;
      ProvideFilePolicy _defaultPolicy;
    };



    RepoMediaAccess::RepoMediaAccess( const ProvideFilePolicy & defaultPolicy_r )
      : _impl( new Impl( defaultPolicy_r ) )
    {}

    RepoMediaAccess::~RepoMediaAccess()
    {}

    void RepoMediaAccess::setDefaultPolicy( const ProvideFilePolicy & policy_r )
    { _impl->_defaultPolicy = policy_r; }

    const ProvideFilePolicy & RepoMediaAccess::defaultPolicy() const
    { return _impl->_defaultPolicy; }

    ManagedFile RepoMediaAccess::provideFile( Repository repo_r,
                                              const OnMediaLocation & loc_r,
                                              const ProvideFilePolicy & policy_r )
    {
      MIL << loc_r << endl;
      // Arrange DownloadFileReportHack to recieve the source::DownloadFileReport
      // and redirect download progress triggers to call the ProvideFilePolicy
      // callback.
      DownloadFileReportHack dumb;
      dumb._redirect = bind( mem_fun_ref( &ProvideFilePolicy::progress ),
                             ref( policy_r ), _1 );
      callback::TempConnect<repo::RepoReport> temp( dumb );

      Url url;
      RepoInfo info = repo_r.info();
      
      RepoException repo_excpt(str::form(_("Can't provide file %s from repository %s"),
                               loc_r.filename().c_str(),
                               info.alias().c_str() ) );
      
      if ( info.baseUrlsEmpty() )
      {
        repo_excpt.remember(RepoException(_("No url in repository.")));
        ZYPP_THROW(repo_excpt);
      }
      
      for ( RepoInfo::urls_const_iterator it = info.baseUrlsBegin();
            it != info.baseUrlsEnd();
            /* incremented in the loop */ )
      {
        url = *it;
        ++it;
        try
        {
          MIL << "Providing file of repo '" << info.alias()
              << "' from " << url << endl;
          shared_ptr<MediaSetAccess> access = _impl->mediaAccessForUrl(url);
          _impl->setVerifierForRepo(repo_r, access);

          ManagedFile ret( access->provideFile(loc_r) );

          std::string scheme( url.getScheme() );
          if ( scheme == "http" || scheme == "https" || scheme == "ftp" )
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

              if ( policy_r.failOnChecksumError() )
                ZYPP_THROW( Exception( err.str() ) );
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
