/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SourceCache.cc
 *
*/
#include <iostream>
#include <fstream>
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/base/String.h"

#include "zypp/SourceCache.h"
#include "zypp/source/Builtin.h"
#include "zypp/media/MediaAccess.h"
#include "zypp/SourceManager.h"
#include "zypp/Pathname.h"

using std::endl;
using namespace zypp::source;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////


  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : SourceCache
  //
  ///////////////////////////////////////////////////////////////////
  Pathname SourceCache::_cache_dir = "/var/adm/ZYPP/SourceCache";
  unsigned SourceCache::_next_cache_id = 0;

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SourceCache::SourceCache
  //	METHOD TYPE : Ctor
  //
  SourceCache::SourceCache()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SourceCache::~SourceCache
  //	METHOD TYPE : Dtor
  //
  SourceCache::~SourceCache()
  {}
  
  void SourceCache::setCacheDir( const Pathname & dir_r )
  {
    _cache_dir = dir_r;
  }

  void SourceCache::storeSource(Source_Ref src)
  {
    if (0 != assert_dir(_cache_dir, 0700))
      ZYPP_THROW(Exception("Cannot create cache directory"));
    Pathname cache_dir = _cache_dir + str::hexstring(_next_cache_id++);
    if (0 != assert_dir(cache_dir, 0700))
      ZYPP_THROW(Exception("Cannot create cache directory"));
    src.storeMetadata(cache_dir);
    Url url = src.url();
    Pathname path = src.path();
    std::string alias = src.alias();
    std::ofstream data((cache_dir + "source_info").asString().c_str());
    data << url.asCompleteString() << endl;
    data << path.asString() << endl;
    data << alias << endl;
  }

  void SourceCache::restoreSources()
  {
    std::list<std::string> contents;
    if (0 != readdir( contents, _cache_dir, false))
      ZYPP_THROW(Exception("Cannot read contents of the cache directory"));
    for (std::list<std::string>::const_iterator it = contents.begin();
      it != contents.end(); it++)
    {
      Pathname cache_dir = _cache_dir + *it;
      std::ifstream data((cache_dir + "source_info").asString().c_str());
      std::string url;
      std::string path;
      std::string alias;
      getline(data, url);
      getline(data, path);
      getline(data, alias);
      SourceManager::sourceManager()->addSource(url, path, alias, cache_dir);
    }
  }

  void SourceCache::removeSource(unsigned id)
  {
    Pathname cache_dir = _cache_dir + str::hexstring(_next_cache_id++);
    if (0 != recursive_rmdir(cache_dir))
      ZYPP_THROW(Exception("Cannot delete directory with cached metadata"));
  }

  void SourceCache::removeSource(const Url & url_r, const Pathname & path_r)
  {
    std::list<std::string> contents;
    if (0 != readdir( contents, _cache_dir, false))
      ZYPP_THROW(Exception("Cannot read contents of the cache directory"));
    for (std::list<std::string>::const_iterator it = contents.begin();
      it != contents.end(); it++)
    {
      Pathname cache_dir = _cache_dir + *it;
      std::ifstream data((cache_dir + "source_info").asString().c_str());
      std::string url;
      std::string path;
      getline(data, url);
      getline(data, path);
      if (url == url_r.asCompleteString() && path == path_r)
      {
	if (0 != recursive_rmdir(cache_dir))
	  ZYPP_THROW(Exception("Cannot delete directory with cached metadata"));
	return;
      }
    }
    ZYPP_THROW(Exception("Specified source not stored in the cache"));
  }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const SourceCache & obj )
  {
    return str << "SourceCache";
  }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
