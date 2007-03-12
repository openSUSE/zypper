/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "zypp/base/Logger.h"
#include "zypp/Fetcher.h"
#include "zypp/MediaSetAccess.h"
#include "zypp/PathInfo.h"

using namespace std;
using namespace zypp::filesystem;

namespace zypp
{

Fetcher::Fetcher( const Url &url, const Pathname &path )
  : _url(url), _path(path)
{

}

void Fetcher::enqueue( const OnMediaLocation &resource )
{
  _resources.push_back(resource);
}

void Fetcher::reset()
{
  _resources.clear();
  _caches.clear();
}

void Fetcher::insertCache( const Pathname &cache_dir )
{
  _caches.push_back(cache_dir);
}

void Fetcher::start( const Pathname &dest_dir )
{
  MediaSetAccess media(_url, _path);
  
  for ( list<OnMediaLocation>::const_iterator it_res = _resources.begin(); it_res != _resources.end(); ++it_res )
  {
    bool got_from_cache = false;
    for ( list<Pathname>::const_iterator it_cache = _caches.begin(); it_cache != _caches.end(); ++it_cache )
    {
      // Pathinfos could be cached to avoid too many stats?
      PathInfo info(*it_cache);
      if ( info.isDir() )
      {
        // does the current file exists in the current cache?
        Pathname cached_file = *it_cache + (*it_res).filename();
        if ( PathInfo( cached_file ).isExist() )
        {
          // check the checksum
          if ( is_checksum( cached_file, (*it_res).checksum() ) )
          {
            // cached
            MIL << "file " << (*it_res).filename() << " found in previous cache. Using cached copy." << endl;
            // checksum is already checked.
            // we could later implement double failover and try to download if file copy fails.
          
            // replicate the complete path in the target directory
            Pathname dest_full_path = dest_dir + (*it_res).filename();
            if ( assert_dir( dest_full_path.dirname() ) != 0 )
              ZYPP_THROW( Exception("Can't create " + dest_full_path.dirname().asString()));

            if ( filesystem::copy(cached_file, dest_full_path ) != 0 )
            { //copy_file2dir
              //ZYPP_THROW(SourceIOException("Can't copy " + cached_file.asString() + " to " + destination.asString()));
              ERR << "Can't copy " << cached_file + " to " + dest_dir << endl;
              // try next cache
              continue;
            }
            
            got_from_cache = true;
            break;
          }
        }
        else
        {
          // File exists in cache but with a different checksum
          // so just try next cache
          continue; 
        }
      }
      else
      {
        // skip bad cache directory and try with next one
        ERR << "Skipping cache : " << *it_cache << endl;
        continue;
      }
    }
    
    if ( ! got_from_cache )
    {
      // try to get the file from the net
      try
      {
        Pathname tmp_file = media.provideFile(*it_res);
        Pathname dest_full_path = dest_dir + (*it_res).filename();
        if ( assert_dir( dest_full_path.dirname() ) != 0 )
              ZYPP_THROW( Exception("Can't create " + dest_full_path.dirname().asString()));
        if ( filesystem::copy(tmp_file, dest_full_path ) != 0 )
        {
          ZYPP_THROW( Exception("Can't copy " + tmp_file.asString() + " to " + dest_dir.asString()));
        }
      }
      catch (const Exception & excpt_r)
      {
        ZYPP_CAUGHT(excpt_r);
        ZYPP_THROW(Exception("Can't provide " + (*it_res).filename().asString() + " : " + excpt_r.msg() ));
      }
    }
    else
    {
      // We got the file from cache
      // continue with next file
      continue;
    }
  }
}
//       callback::SendReport<DigestReport> report;
//       if ( checksum.empty() )
//       {
//         MIL << "File " <<  file_url << " has no checksum available." << std::endl;
//         if ( report->askUserToAcceptNoDigest(file_to_download) )
//         {
//           MIL << "User accepted " <<  file_url << " with no checksum." << std::endl;
//           return;
//         }
//         else
//         {
//           ZYPP_THROW(SourceMetadataException( file_url.asString() + " " + N_(" miss checksum.") ));
//         }
//       }
//       else
//       {
//         if (! is_checksum( destination, checksum))
//           ZYPP_THROW(SourceMetadataException( file_url.asString() + " " + N_(" fails checksum verification.") ));
//       }

} // namespace zypp