/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <fstream>
#include "zypp/base/String.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/Function.h"
#include "zypp/ZConfig.h"

#include "zypp/parser/yum/RepomdFileReader.h"
#include "zypp/parser/yum/PatchesFileReader.h"
#include "Downloader.h"
#include "zypp/repo/MediaInfoDownloader.h"
#include "zypp/base/UserRequestException.h"
#include "zypp/parser/xml/Reader.h"

using namespace std;
using namespace zypp::xml;
using namespace zypp::parser::yum;

namespace zypp
{
namespace repo
{
namespace yum
{

Downloader::Downloader( const RepoInfo &repoinfo , const Pathname &delta_dir)
  : repo::Downloader(repoinfo), _delta_dir(delta_dir), _media_ptr(0L)
{}

RepoStatus Downloader::status( MediaSetAccess &media )
{
  Pathname repomd = media.provideFile( repoInfo().path() + "/repodata/repomd.xml");
  return RepoStatus(repomd);
}

static OnMediaLocation loc_with_path_prefix( const OnMediaLocation & loc, const Pathname & prefix )
{
  if (prefix.empty() || prefix == "/")
    return loc;

  OnMediaLocation loc_with_path(loc);
  loc_with_path.changeFilename(prefix / loc.filename());
  return loc_with_path;
}

// search old repository file file to run the delta algorithm on
static Pathname search_deltafile( const Pathname & dir, const Pathname & file )
{
  Pathname deltafile;
  if (!PathInfo(dir).isDir())
    return deltafile;
  string base = file.basename();
  size_t hypoff = base.find("-");
  if (hypoff != string::npos)
    base.replace(0, hypoff + 1, "");
  size_t basesize = base.size();
  std::list<Pathname> retlist;
  if (!filesystem::readdir(retlist, dir, false))
  {
    for_( it, retlist.begin(), retlist.end() )
    {
      string fn = it->asString();
      if (fn.size() >= basesize && fn.substr(fn.size() - basesize, basesize) == base)
	deltafile = *it;
    }
  }
  return deltafile;
}

bool Downloader::patches_Callback( const OnMediaLocation & loc_r, const string & id_r )
{
  OnMediaLocation loc_with_path(loc_with_path_prefix(loc_r, repoInfo().path()));
  MIL << id_r << " : " << loc_with_path << endl;
  this->enqueueDigested(loc_with_path,  FileChecker(), search_deltafile(_delta_dir + "repodata", loc_r.filename()));
  return true;
}


//bool repomd_Callback2( const OnMediaLocation &loc, const ResourceType &dtype, const std::string &typestr, UserData & userData_r );

///////////////////////////////////////////////////////////////////
namespace
{
  ///////////////////////////////////////////////////////////////////
  /// \class Impl
  /// \brief Helper filtering the files offered by a RepomdFileReader
  ///
  /// Clumsy construct; basically an Impl class for Downloader, maintained
  /// in Downloader::download only while parsing a repomd.xml.
  ///
  /// Introduced because Downloader itself lacks an Impl class, thus can't
  /// be extended to provide more data to the callbacks without losing
  /// binary compatibility.
  ///////////////////////////////////////////////////////////////////
  struct RepomdFileReaderCallback2
  {
    RepomdFileReaderCallback2( const RepomdFileReader::ProcessResource & origCallback_r )
    : _origCallback( origCallback_r )
    {
      addWantedLocale( ZConfig::instance().textLocale() );
      for ( const Locale & it : ZConfig::instance().repoRefreshLocales() )
	addWantedLocale( it );
    }

    /** The callback invoked by the RepomdFileReader */
    bool repomd_Callback2( const OnMediaLocation & loc_r, const ResourceType & dtype_r, const std::string & typestr_r )
    {
      // filter well known resource types
      if ( dtype_r == ResourceType::OTHER || dtype_r == ResourceType::FILELISTS )
	return true;	// skip it

      // filter custom resource types (by string)
      if ( dtype_r == ResourceType::NONE )
      {
	// susedata.LANG
	if ( str::hasPrefix( typestr_r, "susedata." ) && ! wantLocale( Locale(typestr_r.c_str()+9) ) )
	  return true;	// skip it
      }

      // take it
      return( _origCallback ? _origCallback( loc_r, dtype_r ) : true );
    }

  private:
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
    RepomdFileReader::ProcessResource _origCallback;	///< Original Downloader callback
    LocaleSet _wantedLocales;				///< Locales do download

  };
} // namespace
///////////////////////////////////////////////////////////////////

bool Downloader::repomd_Callback( const OnMediaLocation & loc_r, const ResourceType & dtype_r )
{
  // NOTE: Filtering of unwanted files is done in RepomdFileReaderCallback2!

  // schedule file for download
  const OnMediaLocation & loc_with_path(loc_with_path_prefix(loc_r, repoInfo().path()));
  this->enqueueDigested(loc_with_path, FileChecker(), search_deltafile(_delta_dir + "repodata", loc_r.filename()));

  // We got a patches file we need to read, to add patches listed
  // there, so we transfer what we have in the queue, and
  // queue the patches in the patches callback
  if ( dtype_r == ResourceType::PATCHES )
  {
    this->start( _dest_dir, *_media_ptr );
    // now the patches.xml file must exists
    PatchesFileReader( _dest_dir + repoInfo().path() + loc_r.filename(),
                       bind( &Downloader::patches_Callback, this, _1, _2));
  }
  return true;
}

void Downloader::download( MediaSetAccess & media, const Pathname & dest_dir, const ProgressData::ReceiverFnc & progressrcv )
{
  Pathname masterIndex( repoInfo().path() / "/repodata/repomd.xml" );
  defaultDownloadMasterIndex( media, dest_dir, masterIndex );

  // init the data stored in Downloader itself
  _media_ptr = (&media);
  _dest_dir = dest_dir;

  // init the extended data
  RepomdFileReaderCallback2 pimpl( bind(&Downloader::repomd_Callback, this, _1, _2) );

  // setup parser
  RepomdFileReader( dest_dir / masterIndex,
		    RepomdFileReader::ProcessResource2( bind(&RepomdFileReaderCallback2::repomd_Callback2, &pimpl, _1, _2, _3) ) );

  // ready, go!
  start( dest_dir, media );
}

} // namespace yum
} // namespace repo
} // namespace zypp



