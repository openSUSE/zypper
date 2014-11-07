
#include <iostream>
#include <fstream>

#include "zypp/base/LogTools.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/String.h"
#include "zypp/base/Regex.h"
#include "zypp/OnMediaLocation.h"
#include "zypp/MediaSetAccess.h"
#include "zypp/Fetcher.h"
#include "zypp/Locale.h"
#include "zypp/ZConfig.h"
#include "zypp/repo/MediaInfoDownloader.h"
#include "zypp/repo/susetags/Downloader.h"
#include "zypp/parser/ParseException.h"
#include "zypp/parser/susetags/RepoIndex.h"
#include "zypp/base/UserRequestException.h"

using namespace std;
using namespace zypp::parser;
using namespace zypp::parser::susetags;

namespace zypp
{
namespace repo
{
namespace susetags
{

Downloader::Downloader( const RepoInfo &repoinfo, const Pathname &delta_dir )
  : repo::Downloader(repoinfo), _delta_dir(delta_dir)
{
}

RepoStatus Downloader::status( MediaSetAccess &media )
{
  Pathname content = media.provideFile( repoInfo().path() + "/content");
  // the media.1 is always in the root of the media, not like the content
  // file which is in the path() location
  Pathname mediafile = media.provideFile( "/media.1/media" );

  return RepoStatus(content) && RepoStatus(mediafile);
}

// search old repository file file to run the delta algorithm on
static Pathname search_deltafile( const Pathname &dir, const Pathname &file )
{
  Pathname deltafile(dir + file.basename());
  if (PathInfo(deltafile).isExist())
    return deltafile;
  return Pathname();
}


/** \todo: Downloading/sigcheck of master index shoudl be common in base class */
void Downloader::download( MediaSetAccess &media,
                           const Pathname &dest_dir,
                           const ProgressData::ReceiverFnc & progress )
{
  downloadMediaInfo( dest_dir, media );

  Pathname masterIndex( repoInfo().path() / "/content" );
  defaultDownloadMasterIndex( media, dest_dir, masterIndex );

  // Content file first to get the repoindex
  {
    Pathname inputfile( dest_dir / masterIndex );
    ContentFileReader content;
    content.setRepoIndexConsumer( bind( &Downloader::consumeIndex, this, _1 ) );
    content.parse( inputfile );
  }
  if ( ! _repoindex )
  {
    ZYPP_THROW( ParseException( (dest_dir+repoInfo().path()).asString() + ": " + "No repository index in content file." ) );
  }
  MIL << "RepoIndex: " << _repoindex << endl;
  if ( _repoindex->metaFileChecksums.empty() )
  {
    ZYPP_THROW( ParseException( (dest_dir+repoInfo().path()).asString() + ": " + "No metadata checksums in content file." ) );
  }
  if ( _repoindex->signingKeys.empty() )
  {
    WAR << "No signing keys defined." << endl;
  }

  // Prepare parsing
  Pathname descr_dir = _repoindex->descrdir; // path below reporoot
  //_datadir  = _repoIndex->datadir;  // path below reporoot

  std::map<std::string,RepoIndex::FileChecksumMap::const_iterator> availablePackageTranslations;

  for_( it, _repoindex->metaFileChecksums.begin(), _repoindex->metaFileChecksums.end() )
  {
    // omit unwanted translations
    if ( str::hasPrefix( it->first, "packages" ) )
    {
      static const str::regex rx_packages( "^packages((.gz)?|(.([^.]*))(.gz)?)$" );
      str::smatch what;
      if ( str::regex_match( it->first, what, rx_packages ) )
      {
	if ( what[4].empty() // packages(.gz)?
	  || what[4] == "DU"
	  || what[4] == "en" )
	{ ; /* always downloaded */ }
	else if ( what[4] == "FL" )
	{ continue; /* never downloaded */ }
	else
	{
	  // remember and decide later
	  availablePackageTranslations[what[4]] = it;
	  continue;
	}
      }
      else
	continue; // discard
    }
    else if ( it->first == "patterns.pat"
              || it->first == "patterns.pat.gz" )
    {
      // take all patterns in one go
    }
    else if ( str::endsWith( it->first, ".pat" )
              || str::endsWith( it->first, ".pat.gz" ) )
    {

      // *** see also zypp/parser/susetags/RepoParser.cc ***

      // omit unwanted patterns, see https://bugzilla.novell.com/show_bug.cgi?id=298716
      // expect "<name>.<arch>.pat[.gz]", <name> might contain additional dots
      // split at dots, take .pat or .pat.gz into account

      std::vector<std::string> patparts;
      unsigned archpos = 2;
      // expect "<name>.<arch>.pat[.gz]", <name> might contain additional dots
      unsigned count = str::split( it->first, std::back_inserter(patparts), "." );
      if ( patparts[count-1] == "gz" )
          archpos++;

      if ( count > archpos )
      {
        try				// might by an invalid architecture
        {
          Arch patarch( patparts[count-archpos] );
          if ( !patarch.compatibleWith( ZConfig::instance().systemArchitecture() ) )
          {
            // discard, if not compatible
            MIL << "Discarding pattern " << it->first << endl;
            continue;
          }
        }
        catch ( const Exception & excpt )
        {
          WAR << "Pattern file name does not contain recognizable architecture: " << it->first << endl;
          // keep .pat file if it doesn't contain an recognizable arch
        }
      }
    }
    MIL << "adding job " << it->first << endl;
    OnMediaLocation location( repoInfo().path() + descr_dir + it->first, 1 );
    location.setChecksum( it->second );
    enqueueDigested(location, FileChecker(), search_deltafile(_delta_dir + descr_dir, it->first));
  }

  // check whether to download more package translations:
  {
    auto fnc_checkTransaltions( [&]( const Locale & locale_r ) {
      for ( Locale toGet( locale_r ); toGet != Locale::noCode; toGet = toGet.fallback() )
      {
	auto it( availablePackageTranslations.find( toGet.code() ) );
	if ( it != availablePackageTranslations.end() )
	{
	  auto mit( it->second );
	  MIL << "adding job " << mit->first << endl;
	  OnMediaLocation location( repoInfo().path() + descr_dir + mit->first, 1 );
	  location.setChecksum( mit->second );
	  enqueueDigested(location, FileChecker(), search_deltafile(_delta_dir + descr_dir, mit->first));
	  break;
	}
      }
    });
    for ( const Locale & it : ZConfig::instance().repoRefreshLocales() )
    {
      fnc_checkTransaltions( it );
    }
    fnc_checkTransaltions( ZConfig::instance().textLocale() );
  }

  for_( it, _repoindex->mediaFileChecksums.begin(), _repoindex->mediaFileChecksums.end() )
  {
    // Repo adopts license files listed in HASH
    if ( it->first != "license.tar.gz" )
      continue;

    MIL << "adding job " << it->first << endl;
    OnMediaLocation location( repoInfo().path() + it->first, 1 );
    location.setChecksum( it->second );
    enqueueDigested(location, FileChecker(), search_deltafile(_delta_dir, it->first));
  }

  for_( it, _repoindex->signingKeys.begin(),_repoindex->signingKeys.end() )
  {
    MIL << "adding job " << it->first << endl;
    OnMediaLocation location( repoInfo().path() + it->first, 1 );
    location.setChecksum( it->second );
    enqueueDigested(location);
  }

  start( dest_dir, media );
}

void Downloader::consumeIndex( const RepoIndex_Ptr & data_r )
{
  MIL << "Consuming repo index" << endl;
  _repoindex = data_r;
}

}// ns susetags
}// ns source
} // ns zypp
