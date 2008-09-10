
#include <iostream>
#include <fstream>

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
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

Downloader::Downloader(const RepoInfo &info )
    : _info(info)
{

}

Downloader::Downloader(const Pathname &path )
{
    RepoInfo info;
    info.setPath(path);
    _info = info;
}

RepoStatus Downloader::status( MediaSetAccess &media )
{
    Pathname content = media.provideFile( _info.path() + "/content");
    Pathname mediafile = media.provideFile( _info.path() + "/media.1/media" );

  return RepoStatus(content) && RepoStatus(mediafile);
}

void Downloader::download( MediaSetAccess &media,
                           const Pathname &dest_dir,
                           const ProgressData::ReceiverFnc & progress )
{
  downloadMediaInfo( dest_dir, media );

  SignatureFileChecker sigchecker(_info.name());

  Pathname sig = _info.path() + "/content.asc";
  if ( media.doesFileExist(sig) )
  {
    this->enqueue( OnMediaLocation( sig, 1 ) );
    this->start( dest_dir, media );
    this->reset();

    sigchecker = SignatureFileChecker( dest_dir + sig, _info.name() );
  }

  Pathname key = _info.path() + "/content.key";
  if ( media.doesFileExist(key) )
  {
    this->enqueue( OnMediaLocation( key, 1 ) );
    this->start( dest_dir, media );
    this->reset();
    sigchecker.addPublicKey(dest_dir + key);
  }

  if ( ! _info.gpgCheck() )
  {
    WAR << "Signature checking disabled in config of repository " << _info.alias() << endl;
  }
  this->enqueue( OnMediaLocation( _info.path() + "/content", 1 ),
                 _info.gpgCheck() ? FileChecker(sigchecker) : FileChecker(NullFileChecker()) );
  this->start( dest_dir, media );
  this->reset();

  Pathname descr_dir;

  // Content file first to get the repoindex
  {
      Pathname inputfile( dest_dir +  _info.path() + "/content" );
    ContentFileReader content;
    content.setRepoIndexConsumer( bind( &Downloader::consumeIndex, this, _1 ) );
    content.parse( inputfile );
  }
  if ( ! _repoindex )
  {
    ZYPP_THROW( ParseException( (dest_dir+_info.path()).asString() + ": " + "No repository index in content file." ) );
  }
  MIL << "RepoIndex: " << _repoindex << endl;
  if ( _repoindex->metaFileChecksums.empty() )
  {
    ZYPP_THROW( ParseException( (dest_dir+_info.path()).asString() + ": " + "No metadata checksums in content file." ) );
  }
  if ( _repoindex->signingKeys.empty() )
  {
    WAR << "No signing keys defined." << endl;
  }

  // Prepare parsing
  descr_dir = _repoindex->descrdir; // path below reporoot
  //_datadir  = _repoIndex->datadir;  // path below reporoot


  for ( RepoIndex::FileChecksumMap::const_iterator it = _repoindex->metaFileChecksums.begin();
        it != _repoindex->metaFileChecksums.end();
        ++it )
  {
    // omit unwanted translations
    if ( str::hasPrefix( it->first, "packages" ) )
    {
      std::string rest( str::stripPrefix( it->first, "packages" ) );
      if ( ! (   rest.empty()
              || rest == ".DU"
              || rest == ".en"
              || rest == ".gz"
              || rest == ".DU.gz"
              || rest == ".en.gz" ) )
      {
        // Not 100% correct as we take each fallback of textLocale
        Locale toParse( ZConfig::instance().textLocale() );
        while ( toParse != Locale::noCode )
        {
          if ( rest == ("."+toParse.code()) || (rest == ("."+toParse.code()+".gz")) )
            break;
          toParse = toParse.fallback();
        }
        if ( toParse == Locale::noCode )
        {
          // discard
          continue;
        }
      }
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
    OnMediaLocation location( _info.path() + descr_dir + it->first, 1 );
    location.setChecksum( it->second );
    this->enqueueDigested(location);
  }

  for ( RepoIndex::FileChecksumMap::const_iterator it = _repoindex->signingKeys.begin();
        it != _repoindex->signingKeys.end();
        ++it )
  {
    OnMediaLocation location( _info.path() + it->first, 1 );
    location.setChecksum( it->second );
    this->enqueueDigested(location);
  }

  this->start( dest_dir, media );
}

void Downloader::consumeIndex( const RepoIndex_Ptr & data_r )
{
  MIL << "Consuming repo index" << endl;
  _repoindex = data_r;
}

}// ns susetags
}// ns source
} // ns zypp
