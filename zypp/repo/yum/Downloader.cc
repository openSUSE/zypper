/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <fstream>
#include <solv/solvversion.h>
#include <zypp/base/String.h>
#include <zypp/base/LogTools.h>
#include <zypp/base/Function.h>
#include <zypp/ZConfig.h>

#include "Downloader.h"
#include <zypp/repo/MediaInfoDownloader.h>
#include <zypp/base/UserRequestException.h>
#include <zypp/parser/xml/Reader.h>
#include <zypp/parser/yum/RepomdFileReader.h>

using namespace zypp::xml;
using namespace zypp::parser::yum;

namespace zypp
{
namespace repo
{
namespace yum
{
  ///////////////////////////////////////////////////////////////////
  namespace
  {
    inline OnMediaLocation loc_with_path_prefix( OnMediaLocation loc_r, const Pathname & prefix_r )
    {
      if ( ! prefix_r.empty() && prefix_r != "/" )
	loc_r.changeFilename( prefix_r / loc_r.filename() );
      return loc_r;
    }

    // search old repository file to run the delta algorithm on
    Pathname search_deltafile( const Pathname & dir, const Pathname & file )
    {
      Pathname deltafile;
      if ( ! PathInfo(dir).isDir() )
	return deltafile;

      // Strip the checksum preceding the file stem so we can look for an
      // old *-primary.xml which may contain some reusable blocks.
      std::string base { file.basename() };
      size_t hypoff = base.find( "-" );
      if ( hypoff != std::string::npos )
	base.replace( 0, hypoff + 1, "" );

      std::list<std::string> retlist;
      if ( ! filesystem::readdir( retlist, dir, false ) )
      {
	for ( const auto & fn : retlist )
	{
	  if ( str::endsWith( fn, base ) )
	    deltafile = fn;
	}
      }
      return deltafile;
    }
  } // namespace
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  /// \class Downloader::Impl
  /// \brief Helper filtering the files offered by a RepomdFileReader
  ///
  /// Clumsy construct; basically an Impl class for Downloader, maintained
  /// in Downloader::download only while parsing a repomd.xml.
  ///     File types:
  ///         type        (plain)
  ///         type_db     (sqlite, ignored by zypp)
  ///         type_zck    (zchunk, preferred)
  ///     Localized type:
  ///         susedata.LOCALE
  ///////////////////////////////////////////////////////////////////
  struct Downloader::Impl
  {
    NON_COPYABLE( Impl );
    NON_MOVABLE( Impl );

    Impl( Downloader & downloader_r, MediaSetAccess & media_r, const Pathname & destDir_r )
    : _downloader { downloader_r }
    , _media { media_r }
    , _destDir { destDir_r }
    {
      addWantedLocale( ZConfig::instance().textLocale() );
      for ( const Locale & it : ZConfig::instance().repoRefreshLocales() )
	addWantedLocale( it );
    }

    /** The callback invoked by the RepomdFileReader.
     * It's a pity, but in the presence of separate "type" and "type_zck" entries,
     * we have to scan the whole file before deciding what to download....
     */
    bool operator()( const OnMediaLocation & loc_r, const std::string & typestr_r )
    {
      if ( str::endsWith( typestr_r, "_db" ) )
	return true;	// skip sqlitedb

      bool zchk { str::endsWith( typestr_r, "_zck" ) };
#if defined(LIBSOLVEXT_FEATURE_ZCHUNK_COMPRESSION)
      const std::string & basetype { zchk ? typestr_r.substr( 0, typestr_r.size()-4 ) : typestr_r };
#else
      if ( zchk )
	return true;	// skip zchunk if not supported by libsolv
      const std::string & basetype { typestr_r };
#endif

      // filter well known resource types
      if ( basetype == "other" || basetype == "filelists" )
	return true;	// skip it

      // filter localized susedata
      if ( str::startsWith( basetype, "susedata." ) )
      {
	// susedata.LANG
	if ( ! wantLocale( Locale(basetype.c_str()+9) ) )
	  return true;	// skip it
      }

      // may take it... (prefer zchnk)
      if ( zchk || !_wantedFiles.count( basetype ) )
	_wantedFiles[basetype] = loc_r;

      return true;
    }

    void finalize()
    {
      // schedule fileS for download
      for ( const auto & el : _wantedFiles )
      {
	const OnMediaLocation & loc { el.second };
	const OnMediaLocation & loc_with_path { loc_with_path_prefix( loc, _downloader.repoInfo().path() ) };
	_downloader.enqueueDigested( loc_with_path, FileChecker(), search_deltafile( deltaDir()/"repodata", loc.filename() ) );
      }
    }

  private:
    const Pathname & deltaDir() const
    { return _downloader._deltaDir; }

    bool wantLocale( const Locale & locale_r ) const
    { return _wantedLocales.count( locale_r ); }

    void addWantedLocale( Locale locale_r )
    {
      while ( locale_r )
      {
	_wantedLocales.insert( locale_r );
	locale_r = locale_r.fallback();
      }
    }

  private:
    Downloader & _downloader;
    MediaSetAccess & _media;
    const Pathname & _destDir;

    LocaleSet _wantedLocales;	///< Locales do download
    std::map<std::string,OnMediaLocation> _wantedFiles;
  };

  ///////////////////////////////////////////////////////////////////
  //
  // class Downloader
  //
  ///////////////////////////////////////////////////////////////////

  Downloader::Downloader( const RepoInfo & info_r, const Pathname & deltaDir_r )
  : repo::Downloader { info_r}
  , _deltaDir { deltaDir_r }
  {}

  void Downloader::download( MediaSetAccess & media_r, const Pathname & destDir_r, const ProgressData::ReceiverFnc & progress_r )
  {
    downloadMediaInfo( destDir_r, media_r );

    Pathname masterIndex { repoInfo().path() / "/repodata/repomd.xml" };
    defaultDownloadMasterIndex( media_r, destDir_r, masterIndex );

    //enable precache
    setMediaSetAccess( media_r );

    // setup parser
    Impl pimpl( *this, media_r, destDir_r );
    RepomdFileReader( destDir_r / masterIndex, std::ref(pimpl) );
    pimpl.finalize();

    // ready, go!
    start( destDir_r );
  }

  RepoStatus Downloader::status( MediaSetAccess & media_r )
  {
    RepoStatus ret { media_r.provideOptionalFile( repoInfo().path() / "/repodata/repomd.xml" ) };
    if ( !ret.empty() )	// else: mandatory master index is missing
      ret = ret && RepoStatus( media_r.provideOptionalFile( "/media.1/media" ) );
    // else: mandatory master index is missing -> stay empty
    return ret;
  }
} // namespace yum
} // namespace repo
} // namespace zypp



