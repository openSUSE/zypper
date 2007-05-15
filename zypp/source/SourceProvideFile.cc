/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/SourceProvideFile.cc
 *
*/
#include <iostream>
#include <fstream>
#include <sstream>
#include "zypp/base/Logger.h"

#include "zypp/source/SourceProvideFile.h"
#include "zypp/ZYppCallbacks.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
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
       * We redirect the static report triggered from Source_Ref::provideFile
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
       * The Pathname argument is ignored, as Source_Ref::releaseFile expects the filename
       * relative to the medias root (i.e. same args as to provideFile).
      */
      struct SourceReleaseFile
      {
	SourceReleaseFile( Source_Ref source_r, const Pathname & location_r, unsigned mediaNr_r )
	  : _source( source_r ), _location( location_r ), _medianr( mediaNr_r )
	{}

	void operator()( const Pathname & /*UNUSED*/ )
	{
	  _source.releaseFile( _location, _medianr );
	}

	Source_Ref _source;
	Pathname   _location;
	unsigned   _medianr;
      };

      /////////////////////////////////////////////////////////////////
    } // namespace
    ///////////////////////////////////////////////////////////////////

    ManagedFile provideFile( Source_Ref source_r,
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


      ManagedFile ret( source_r.provideFile( loc_r.filename(), loc_r.medianr() ),
		       SourceReleaseFile( source_r, loc_r.filename(), loc_r.medianr() ) );

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
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
