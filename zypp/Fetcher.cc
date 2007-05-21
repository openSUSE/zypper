/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Fetcher.cc
 *
*/
#include <iostream>
#include <list>

#include "zypp/base/Logger.h"
#include "zypp/base/DefaultIntegral.h"
#include "zypp/Fetcher.h"


using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  struct FetcherJob
  {
    FetcherJob( const OnMediaLocation &loc )
      : location(loc)
    {

    }

    OnMediaLocation location;
  };

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Fetcher::Impl
  //
  /** Fetcher implementation. */
  struct Fetcher::Impl
  {

  public:

    void enqueue( const OnMediaLocation &resource );    
    void addCachePath( const Pathname &cache_dir );
    void reset();
    void start( const Pathname &dest_dir, MediaSetAccess &media );

    /** Offer default Impl. */
    static shared_ptr<Impl> nullimpl()
    {
      static shared_ptr<Impl> _nullimpl( new Impl );
      return _nullimpl;
    }

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }

    std::list<FetcherJob> _resources;
    std::list<Pathname> _caches;
  };
  ///////////////////////////////////////////////////////////////////


  void Fetcher::Impl::enqueue( const OnMediaLocation &resource )
  {
    _resources.push_back(FetcherJob(resource));
  }
  
  void Fetcher::Impl::reset()
  {
    _resources.clear();
    _caches.clear();
  }
  
  void Fetcher::Impl::addCachePath( const Pathname &cache_dir )
  {
    _caches.push_back(cache_dir);
  }
  
  void Fetcher::Impl::start( const Pathname &dest_dir, MediaSetAccess &media )
  {
    for ( list<FetcherJob>::const_iterator it_res = _resources.begin(); it_res != _resources.end(); ++it_res )
    {
      bool got_from_cache = false;
      for ( list<Pathname>::const_iterator it_cache = _caches.begin(); it_cache != _caches.end(); ++it_cache )
      {
        // Pathinfos could be cached to avoid too many stats?
        PathInfo info(*it_cache);
        if ( info.isDir() )
        {
          // does the current file exists in the current cache?
          Pathname cached_file = *it_cache + (*it_res).location.filename();
          if ( PathInfo( cached_file ).isExist() )
          {
            // check the checksum
            if ( is_checksum( cached_file, (*it_res).location.checksum() ) )
            {
              // cached
              MIL << "file " << (*it_res).location.filename() << " found in previous cache. Using cached copy." << endl;
              // checksum is already checked.
              // we could later implement double failover and try to download if file copy fails.
  
              // replicate the complete path in the target directory
              Pathname dest_full_path = dest_dir + (*it_res).location.filename();
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
          Pathname tmp_file = media.provideFile((*it_res).location);
          Pathname dest_full_path = dest_dir + (*it_res).location.filename();
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
          ZYPP_THROW(Exception("Can't provide " + (*it_res).location.filename().asString() + " : " + excpt_r.msg() ));
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

  /** \relates Fetcher::Impl Stream output */
  inline std::ostream & operator<<( std::ostream & str, const Fetcher::Impl & obj )
  {
    return str << "Fetcher::Impl";
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Fetcher
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Fetcher::Fetcher
  //	METHOD TYPE : Ctor
  //
  Fetcher::Fetcher()
  : _pimpl( Impl::nullimpl() )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Fetcher::~Fetcher
  //	METHOD TYPE : Dtor
  //
  Fetcher::~Fetcher()
  {}

  void Fetcher::enqueue( const OnMediaLocation &resource )
  {
    _pimpl->enqueue(resource);
  }
  
  void Fetcher::addCachePath( const Pathname &cache_dir )
  {
    _pimpl->addCachePath(cache_dir);
  }
  
  void Fetcher::reset()
  {
    _pimpl->reset();
  }

  void Fetcher::start( const Pathname &dest_dir, MediaSetAccess &media )
  {
    _pimpl->start(dest_dir, media);
  }


  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const Fetcher & obj )
  {
    return str << *obj._pimpl;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

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

