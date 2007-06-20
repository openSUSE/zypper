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
#include "zypp/base/UserRequestException.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /**
   * Class to encapsulate the \ref OnMediaLocation object
   * and the \ref FileChecker together
   */
  struct FetcherJob
  {
    FetcherJob( const OnMediaLocation &loc )
      : location(loc)
    {

    }

    OnMediaLocation location;
    CompositeFileChecker checkers;
  };

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Fetcher::Impl
  //
  /** Fetcher implementation. */
  struct Fetcher::Impl
  {

  public:

    void enqueue( const OnMediaLocation &resource, const FileChecker &checker  );
    void enqueueDigested( const OnMediaLocation &resource, const FileChecker &checker );
    void addCachePath( const Pathname &cache_dir );
    void reset();
    void start( const Pathname &dest_dir,
                MediaSetAccess &media,
                const ProgressData::ReceiverFnc & progress_receiver );

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


  void Fetcher::Impl::enqueueDigested( const OnMediaLocation &resource, const FileChecker &checker )
  {
    CompositeFileChecker composite;
    composite.add(ChecksumFileChecker(resource.checksum()));
    composite.add(checker);
    enqueue(resource, composite);
  }
  
  void Fetcher::Impl::enqueue( const OnMediaLocation &resource, const FileChecker &checker )
  {
    FetcherJob job(resource);
    job.checkers.add(checker);
    _resources.push_back(resource);
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
  
  void Fetcher::Impl::start( const Pathname &dest_dir,
                             MediaSetAccess &media,
                             const ProgressData::ReceiverFnc & progress_receiver )
  {
    ProgressData progress(_resources.size());
    progress.sendTo(progress_receiver);

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
      
      // no matter where did we got the file, try to validate it:
       Pathname localfile = dest_dir + (*it_res).location.filename();
       // call the checker function
       try {
         (*it_res).checkers(localfile);
       }
       catch ( const FileCheckException &e )
       {
          ZYPP_RETHROW(e);
       }
       catch ( const Exception &e )
       {
          ZYPP_RETHROW(e);
       }
       catch (...)
       {
          ZYPP_THROW(Exception("Unknown error while validating " + (*it_res).location.filename().asString()));
       }

       if ( ! progress.incr() )
        ZYPP_THROW(AbortRequestException());
    } // for each job
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

  void Fetcher::enqueueDigested( const OnMediaLocation &resource, const FileChecker &checker )
  {
    _pimpl->enqueue(resource, checker);
  }
  
  void Fetcher::enqueue( const OnMediaLocation &resource, const FileChecker &checker  )
  {
    _pimpl->enqueue(resource, checker);
  }
  
  void Fetcher::addCachePath( const Pathname &cache_dir )
  {
    _pimpl->addCachePath(cache_dir);
  }
  
  void Fetcher::reset()
  {
    _pimpl->reset();
  }

  void Fetcher::start( const Pathname &dest_dir,
                       MediaSetAccess &media,
                       const ProgressData::ReceiverFnc & progress_receiver )
  {
    _pimpl->start(dest_dir, media, progress_receiver);
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

