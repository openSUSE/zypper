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
#include "zypp/base/Logger.h"

#include "zypp/repo/RepoProvideFile.h"
#include "zypp/ZYppCallbacks.h"
#include "zypp/MediaSetAccess.h"

using std::endl;

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
      struct DownloadFileReportHack : public callback::ReceiveReport<source::SourceReport>
      {
        virtual bool progress( int value )
        {
          if ( _redirect )
            return _redirect( value );
          return true;
        }
        function<bool ( int )> _redirect;
      };

      /** ManagedFile Dispose functor.
       * The Pathname argument is ignored, as Repository::releaseFile expects the filename
       * relative to the medias root (i.e. same args as to provideFile).
      */
      struct RepoReleaseFile
      {
	RepoReleaseFile( Repository repo_r, const Pathname & location_r, unsigned mediaNr_r )
	  : _repo( repo_r ), _location( location_r ), _medianr( mediaNr_r )
	{}

	void operator()( const Pathname & /*UNUSED*/ )
	{
	  //_repo.releaseFile( _location, _medianr );
	}

	Repository _repo;
	Pathname   _location;
	unsigned   _medianr;
      };

      /////////////////////////////////////////////////////////////////
    } // namespace
    ///////////////////////////////////////////////////////////////////

    ManagedFile provideFile( Repository repo_r,
                             const OnMediaLocation & loc_r,
                             const ProvideFilePolicy & policy_r )
    {
      MIL << "sourceProvideFile " << loc_r << endl;
      // Arrange DownloadFileReportHack to recieve the source::DownloadFileReport
      // and redirect download progress triggers to call the ProvideFilePolicy
      // callback.
      DownloadFileReportHack dumb;
      dumb._redirect = bind( mem_fun_ref( &ProvideFilePolicy::progress ),
                             ref( policy_r ), _1 );
      callback::TempConnect<source::SourceReport> temp( dumb );

      Url url;
      RepoInfo info = repo_r.info();
      // FIXME we only support the first url for now.
      if ( info.baseUrls().empty() )
        ZYPP_THROW(Exception("No url in repository."));
      else
        url = * info.baseUrls().begin();
      
      MediaSetAccess access(url);
      
      ManagedFile ret( access.provideFile(loc_r),
		       RepoReleaseFile( repo_r, loc_r.filename(), loc_r.medianr() ) );

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

      MIL << "sourceProvideFile at " << ret << endl;
      return ret;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
