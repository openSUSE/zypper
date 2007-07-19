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
#include "zypp/repo/RepoProvideFile.h"
#include "zypp/ZYppCallbacks.h"
#include "zypp/MediaSetAccess.h"

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
        virtual bool progress( int value )
        {
          if ( _redirect )
            return _redirect( value );
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
      MIL << "provideFile " << loc_r << endl;
      // Arrange DownloadFileReportHack to recieve the source::DownloadFileReport
      // and redirect download progress triggers to call the ProvideFilePolicy
      // callback.
      DownloadFileReportHack dumb;
      dumb._redirect = bind( mem_fun_ref( &ProvideFilePolicy::progress ),
                             ref( policy_r ), _1 );
      callback::TempConnect<repo::RepoReport> temp( dumb );

      Url url;
      RepoInfo info = repo_r.info();
      set<Url> urls = info.baseUrls();
      if ( urls.empty() )
        ZYPP_THROW(Exception(_("No url in repository.")));

      for ( RepoInfo::urls_const_iterator it = urls.begin();
            it != urls.end();
            ++it )
      {
        url = *it;
        try
        {

          MediaSetAccess access(url);

          ManagedFile ret( access.provideFile(loc_r) );

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
        catch ( const Exception &e )
        {
          ZYPP_CAUGHT( e );
          WAR << "Trying next url" << endl;
          continue;
        }
      } // iteration over urls

      ZYPP_THROW(Exception(_("No more urls in repository.")));
      return ManagedFile(); // not reached
    }

    /////////////////////////////////////////////////////////////////
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
