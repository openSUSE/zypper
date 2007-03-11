#include <sys/time.h>

#include <iostream>
#include <fstream>

#include <list>
#include <set>

#include <zypp/base/Logger.h>
#include <zypp/base/String.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>
#include <zypp/media/MediaAccess.h>
#include <zypp/media/MediaManager.h>
#include <zypp/MediaSetAccess.h>
#include <zypp/source/SUSEMediaVerifier.h>
#include <zypp/source/OnMediaLocation.h>

#include "zypp/Product.h"
#include "zypp/Package.h"


using namespace std;
using namespace zypp;
using namespace media;
using namespace source;

class Fetcher
{
private:
  Url _url;
  Pathname _path;
  std::list<OnMediaLocation> _resources;
  std::list<Pathname> _caches;

public:

  Fetcher( const Url &url, const Pathname &path )
    : _url(url), _path(path)
  {
  
  }

  void enqueue( const OnMediaLocation &resource )
  {
    _resources.push_back(resource);
  }

  void reset()
  {
    _resources.clear();
    _caches.clear();
  }

  /**
   * adds a directory to the list of directories
   * where to look for cached files
   */
  void insertCache( const Pathname &cache_dir )
  {
    _caches.push_back(cache_dir);
  }

  void start( const Pathname &dest_dir )
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
          ZYPP_THROW(SourceIOException("Can't provide " + (*it_res).filename().asString() + " : " + excpt_r.msg() ));
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

};

class SUSETagsDownloader
{
  Url _url;
  Pathname _path;
  public:
  SUSETagsDownloader( const Url &url, const Pathname &path )
     : _url(url), _path(path)
  {
    
  }

  void download( const Pathname &dest_dir )
  {
    Fetcher fetcher(_url, _path);
    fetcher.enqueue( OnMediaLocation().filename("/content") );
    fetcher.start( dest_dir );
    fetcher.reset();

    std::ifstream file((dest_dir + "/content").asString().c_str());
    std::string buffer;
    Pathname descr_dir;

    // Note this code assumes DESCR comes before as META

    while (file && !file.eof())
    {
      getline(file, buffer);
      if ( buffer.substr( 0, 5 ) == "DESCR" )
      {
        std::vector<std::string> words;
        if ( str::split( buffer, std::back_inserter(words) ) != 2 )
        {
          // error
          ZYPP_THROW(Exception("bad DESCR line")); 
        }
        descr_dir = words[1];
      }
      else if ( buffer.substr( 0, 4 ) == "META" )
      {
        std::vector<std::string> words;
        if ( str::split( buffer, std::back_inserter(words) ) != 4 )
        {
          // error
          ZYPP_THROW(Exception("bad DESCR line"));
        }
        OnMediaLocation location;
        location.filename( descr_dir + words[3]).checksum( CheckSum( words[1], words[2] ) );
        fetcher.enqueue(location);
      }
    }
    file.close();
    fetcher.start( dest_dir );
  }
};


int main(int argc, char **argv)
{
    try
    {
      ZYpp::Ptr z = getZYpp();
      SUSETagsDownloader downloader(Url("dir:/home/duncan/suse/repo/stable-x86"), "/");
      downloader.download("/tmp/repo-cache");
    }
    catch ( const Exception &e )
    {
      cout << "ups! " << e.msg() << std::endl;
    }
    return 0;
}



