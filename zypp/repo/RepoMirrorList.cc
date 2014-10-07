/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repo/RepoMirrorList.cc
 *
*/

#include <iostream>
#include <vector>
#include <time.h>
#include "zypp/repo/RepoMirrorList.h"
#include "zypp/media/MetaLinkParser.h"
#include "zypp/MediaSetAccess.h"
#include "zypp/base/LogTools.h"
#include "zypp/ZConfig.h"
#include "zypp/PathInfo.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace
    {
      /** Provide mirrorlist in a local file */
      Pathname RepoMirrorListProvide( const Url & url_r )
      {
	Url abs_url( url_r );
	abs_url.setPathName( "/" );
	abs_url.setQueryParam( "mediahandler", "curl" );
	MediaSetAccess access( abs_url );
	return access.provideFile( url_r.getPathName() );
      }

      inline std::vector<Url> RepoMirrorListParseXML( const Pathname &tmpfile )
      {
	InputStream tmpfstream (tmpfile);
	media::MetaLinkParser metalink;
	metalink.parse(tmpfstream);
	return metalink.getUrls();
      }

      inline std::vector<Url> RepoMirrorListParseTXT( const Pathname &tmpfile )
      {
	InputStream tmpfstream (tmpfile);
	std::vector<Url> my_urls;
	string tmpurl;
	while (getline(tmpfstream.stream(), tmpurl))
	{
	  my_urls.push_back(Url(tmpurl));
	}
	return my_urls;
      }

      /** Parse a local mirrorlist \a listfile_r and return usable URLs */
      inline std::vector<Url> RepoMirrorListParse( const Url & url_r, const Pathname & listfile_r )
      {
	std::vector<Url> mirrorurls;
	if ( url_r.asString().find( "/metalink" ) != string::npos )
	  mirrorurls = RepoMirrorListParseXML( listfile_r );
	else
	  mirrorurls = RepoMirrorListParseTXT( listfile_r );


	std::vector<Url> ret;
	for ( auto & murl : mirrorurls )
	{
	  if ( murl.getScheme() != "rsync" )
	  {
	    size_t delpos = murl.getPathName().find("repodata/repomd.xml");
	    if( delpos != string::npos )
	    {
	      murl.setPathName( murl.getPathName().erase(delpos)  );
	    }
	    ret.push_back( murl );

	    if ( ret.size() >= 4 )	// why 4?
	      break;
	  }
	}
	return ret;
      }

    } // namespace
    ///////////////////////////////////////////////////////////////////


    RepoMirrorList::RepoMirrorList( const Url & url_r, const Pathname & metadatapath_r )
    {
      if ( url_r.getScheme() == "file" )
      {
	// no cache for local mirrorlist
	_urls = RepoMirrorListParse( url_r, url_r.getPathName() );
      }
      else
      {
	Pathname cachefile( metadatapath_r );
	if ( url_r.asString().find( "/metalink" ) != string::npos )
	  cachefile /= "mirrorlist.xml";
	else
	  cachefile /= "mirrorlist.txt";

	zypp::filesystem::PathInfo cacheinfo( cachefile );
	if ( !cacheinfo.isFile() || cacheinfo.mtime() < time(NULL) - (long) ZConfig::instance().repo_refresh_delay() * 60 )
	{
	  DBG << "Getting MirrorList from URL: " << url_r << endl;
	  Pathname localfile( RepoMirrorListProvide( url_r ) );

	  // Create directory, if not existing
	  DBG << "Copy MirrorList file to " << cachefile << endl;
	  zypp::filesystem::assert_dir( metadatapath_r );
	  zypp::filesystem::hardlinkCopy( localfile, cachefile );
	}

	_urls = RepoMirrorListParse( url_r, cachefile );
	if( _urls.empty() )
	{
	  DBG << "Removing Cachefile as it contains no URLs" << endl;
	  zypp::filesystem::unlink( cachefile );
	}
      }
    }

    RepoMirrorList::RepoMirrorList( const Url & url_r )
    {
      DBG << "Getting MirrorList from URL: " << url_r << endl;
      Pathname localfile( url_r.getScheme() == "file"
                        ? url_r.getPathName()
			: RepoMirrorListProvide( url_r ) );
      _urls = RepoMirrorListParse( url_r, localfile );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
