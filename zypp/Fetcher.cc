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
#include <fstream>
#include <list>
#include <map>

#include "zypp/base/Easy.h"
#include "zypp/base/Logger.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/base/DefaultIntegral.h"
#include "zypp/base/String.h"
#include "zypp/Fetcher.h"
#include "zypp/CheckSum.h"
#include "zypp/base/UserRequestException.h"
#include "zypp/parser/susetags/ContentFileReader.h"
#include "zypp/parser/susetags/RepoIndex.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /**
   * class that represents indexes which add metadata
   * to fetcher jobs and therefore need to be retrieved
   * in advance.
   */
  struct FetcherIndex
  {
    FetcherIndex( const OnMediaLocation &loc )
      : location(loc)
    {
    }
      
    OnMediaLocation location;
  };
  typedef shared_ptr<FetcherIndex> FetcherIndex_Ptr;
      
  /**
   * Class to encapsulate the \ref OnMediaLocation object
   * and the \ref FileChecker together
   */
  struct FetcherJob
  {
    FetcherJob( const OnMediaLocation &loc )
      : location(loc)
      , directory(false)
      , recursive(false)
    {
      //MIL << location << endl;
    }
    
    ~FetcherJob()
    {
      //MIL << location << " | * " << checkers.size() << endl;
    }

    OnMediaLocation location;
    //CompositeFileChecker checkers;
    list<FileChecker> checkers;
    bool directory;
    bool recursive;
  };

  typedef shared_ptr<FetcherJob> FetcherJob_Ptr;

  std::ostream & operator<<( std::ostream & str, const FetcherJob_Ptr & obj )
  {
    return str << obj->location;
  }


  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Fetcher::Impl
  //
  /** Fetcher implementation. */
  class Fetcher::Impl
  {
    friend std::ostream & operator<<( std::ostream & str, const Fetcher::Impl & obj );
  public:
      Impl();
      
    ~Impl() {
      MIL << endl;
     }
      
      void setOptions( Fetcher::Options options );
      Fetcher::Options options() const;

      void addIndex( const OnMediaLocation &resource );
      void enqueue( const OnMediaLocation &resource, const FileChecker &checker = FileChecker()  );
      void enqueueDir( const OnMediaLocation &resource, bool recursive, const FileChecker &checker = FileChecker() );
      void enqueueDigested( const OnMediaLocation &resource, const FileChecker &checker = FileChecker() );
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
      /**
       * download the indexes and reads them
       */
      void readIndexes( MediaSetAccess &media, const Pathname &dest_dir);

      /**
       * reads a downloaded index file and updates internal
       * attributes table
       *
       * The index lists files relative to a directory, which is
       * normally the same as the index file is located.
       */
      void readIndex( const Pathname &index, const Pathname &basedir );

      /** specific version of \ref readIndex for SHA1SUMS file */
      void readSha1sumsIndex( const Pathname &index, const Pathname &basedir );
      
      /** specific version of \ref readIndex for SHA1SUMS file */
      void readContentFileIndex( const Pathname &index, const Pathname &basedir );
      
      /**
       * tries to provide the file represented by job into dest_dir by
       * looking at the cache. If success, returns true, and the desired
       * file should be available on dest_dir
       */
      bool provideFromCache( const OnMediaLocation &resource, const Pathname &dest_dir );
      /**
       * Validates the job against is checkers, by using the file instance
       * on dest_dir
       * \throws Exception
       */
      void validate( const OnMediaLocation &resource, const Pathname &dest_dir, const list<FileChecker> &checkers );

      /**
       * scan the directory and adds the individual jobs
       */
          void addDirJobs( MediaSetAccess &media, const OnMediaLocation &resource,
                       const Pathname &dest_dir, bool recursive );
      /**
       * Provide the resource to \ref dest_dir
       */
      void provideToDest( MediaSetAccess &media, const OnMediaLocation &resource, const Pathname &dest_dir );

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }

    list<FetcherJob_Ptr> _resources;
    list<FetcherIndex_Ptr> _indexes;
    list<Pathname> _caches;
    // checksums read from the indexes
    map<string, CheckSum> _checksums;
    Fetcher::Options _options;      
  };
  ///////////////////////////////////////////////////////////////////

  void Fetcher::Impl::enqueueDigested( const OnMediaLocation &resource, const FileChecker &checker )
  {
    FetcherJob_Ptr job;
    job.reset(new FetcherJob(resource));
    ChecksumFileChecker digest_check(resource.checksum());
    job->checkers.push_back(digest_check);
    if ( checker )
      job->checkers.push_back(checker);
    _resources.push_back(job);
  }
   
  Fetcher::Impl::Impl()
      : _options(0)
  {
  }
    
  void Fetcher::Impl::setOptions( Fetcher::Options options )
  { _options = options; }
    
  Fetcher::Options Fetcher::Impl::options() const
  { return _options; }
    
  void Fetcher::Impl::enqueueDir( const OnMediaLocation &resource,
                                  bool recursive,
                                  const FileChecker &checker )
  {
    FetcherJob_Ptr job;
    job.reset(new FetcherJob(resource));
    if ( checker )
        job->checkers.push_back(checker);
    job->directory = true;
    job->recursive = recursive;
    _resources.push_back(job);
  }

  void Fetcher::Impl::enqueue( const OnMediaLocation &resource, const FileChecker &checker )
  {
    FetcherJob_Ptr job;
    job.reset(new FetcherJob(resource));
    if ( checker )
      job->checkers.push_back(checker);
    _resources.push_back(job);
  }

  void Fetcher::Impl::addIndex( const OnMediaLocation &resource )
  {
      MIL << "adding index " << resource << endl;
      FetcherIndex_Ptr index;
      index.reset(new FetcherIndex(resource));
      _indexes.push_back(index);
  }


  void Fetcher::Impl::reset()
  {
    _resources.clear();
    _indexes.clear();
  }

  void Fetcher::Impl::addCachePath( const Pathname &cache_dir )
  {
    PathInfo info(cache_dir);
    if ( info.isExist() )
    {
      if ( info.isDir() )
      {
        DBG << "Adding fetcher cache: '" << cache_dir << "'." << endl;
        _caches.push_back(cache_dir);
      }
      else
      {
        // don't add bad cache directory, just log the error
        ERR << "Not adding cache: '" << cache_dir << "'. Not a directory." << endl;
      }
    }
    else
    {
        ERR << "Not adding cache '" << cache_dir << "'. Path does not exists." << endl;
    }

  }

  bool Fetcher::Impl::provideFromCache( const OnMediaLocation &resource, const Pathname &dest_dir )
  {
    Pathname dest_full_path = dest_dir + resource.filename();

    // first check in the destination directory
    if ( PathInfo(dest_full_path).isExist() )
    {
      if ( is_checksum( dest_full_path, resource.checksum() )
           && (! resource.checksum().empty() ) )
          return true;
    }

    MIL << "start fetcher with " << _caches.size() << " cache directories." << endl;
    for_ ( it_cache, _caches.begin(), _caches.end() )
    {
      // does the current file exists in the current cache?
      Pathname cached_file = *it_cache + resource.filename();
      if ( PathInfo( cached_file ).isExist() )
      {
        DBG << "File '" << cached_file << "' exist, testing checksum " << resource.checksum() << endl;
         // check the checksum
        if ( is_checksum( cached_file, resource.checksum() ) && (! resource.checksum().empty() ) )
        {
          // cached
          MIL << "file " << resource.filename() << " found in previous cache. Using cached copy." << endl;
          // checksum is already checked.
          // we could later implement double failover and try to download if file copy fails.
           // replicate the complete path in the target directory
          if( dest_full_path != cached_file )
          {
            if ( assert_dir( dest_full_path.dirname() ) != 0 )
              ZYPP_THROW( Exception("Can't create " + dest_full_path.dirname().asString()));

            if ( filesystem::hardlink(cached_file, dest_full_path ) != 0 )
            {
              WAR << "Can't hardlink '" << cached_file << "' to '" << dest_dir << "'. Trying copying." << endl;
              if ( filesystem::copy(cached_file, dest_full_path ) != 0 )
              {
                ERR << "Can't copy " << cached_file + " to " + dest_dir << endl;
                // try next cache
                continue;
              }
            }
          }
          // found in cache
          return true;
        }
      }
    } // iterate over caches
    return false;
  }
    
    void Fetcher::Impl::validate( const OnMediaLocation &resource, const Pathname &dest_dir, const list<FileChecker> &checkers )
  {
    // no matter where did we got the file, try to validate it:
    Pathname localfile = dest_dir + resource.filename();
    // call the checker function
    try
    {
      MIL << "Checking job [" << localfile << "] (" << checkers.size() << " checkers )" << endl;

      for ( list<FileChecker>::const_iterator it = checkers.begin();
            it != checkers.end();
            ++it )
      {
        if (*it)
        {
          (*it)(localfile);
        }
        else
        {
          ERR << "Invalid checker for '" << localfile << "'" << endl;
        }
      }

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
      ZYPP_THROW(Exception("Unknown error while validating " + resource.filename().asString()));
    }
  }

  void Fetcher::Impl::addDirJobs( MediaSetAccess &media,
                                  const OnMediaLocation &resource,
                                  const Pathname &dest_dir, bool recursive  )
  {
      // first get the content of the directory so we can add
      // individual transfer jobs
      MIL << "Adding directory " << resource.filename() << endl;
      filesystem::DirContent content;
      media.dirInfo( content, resource.filename(), false /* dots */, resource.medianr());

      // only try to add an index if it exists
      filesystem::DirEntry shafile;
      shafile.name = "SHA1SUMS"; shafile.type = filesystem::FT_FILE;
      if ( find( content.begin(), content.end(), shafile ) != content.end() )
      {              
          // add the index of this directory
          OnMediaLocation indexloc(resource);
          indexloc.changeFilename(resource.filename() + "SHA1SUMS");
          addIndex(indexloc);
          // we need to read it now
          readIndexes(media, dest_dir);
      }

      for ( filesystem::DirContent::const_iterator it = content.begin();
            it != content.end();
            ++it )
      {
          // skip SHA1SUMS* as they were already retrieved
          if ( str::hasPrefix(it->name, "SHA1SUMS") )
              continue;

          Pathname filename = resource.filename() + it->name;

          switch ( it->type )
          {
          case filesystem::FT_NOT_AVAIL: // old directory.yast contains no typeinfo at all
          case filesystem::FT_FILE:
          {
              CheckSum chksm(resource.checksum());              
              if ( _checksums.find(filename.asString()) != _checksums.end() )
              {
                  // the checksum can be replaced with the one in the index.
                  chksm = _checksums[filename.asString()];
                  //MIL << "resource " << filename << " has checksum in the index file." << endl;
              }
              else
                  WAR << "Resource " << filename << " has no checksum in the index either." << endl;

              enqueueDigested(OnMediaLocation(filename, resource.medianr()).setChecksum(chksm));
              break;
          }
          case filesystem::FT_DIR: // newer directory.yast contain at least directory info
              if ( recursive )
                  addDirJobs(media, filename, dest_dir, recursive);
              break;
          default:
              // don't provide devices, sockets, etc.
              break;
          }
      }
  }

  void Fetcher::Impl::provideToDest( MediaSetAccess &media, const OnMediaLocation &resource, const Pathname &dest_dir )
  {
    bool got_from_cache = false;

    // start look in cache
    got_from_cache = provideFromCache(resource, dest_dir);

    if ( ! got_from_cache )
    {
      MIL << "Not found in cache, downloading" << endl;

      // try to get the file from the net
      try
      {
        Pathname tmp_file = media.provideFile(resource);
        Pathname dest_full_path = dest_dir + resource.filename();
        if ( assert_dir( dest_full_path.dirname() ) != 0 )
              ZYPP_THROW( Exception("Can't create " + dest_full_path.dirname().asString()));
        if ( filesystem::copy(tmp_file, dest_full_path ) != 0 )
        {
          ZYPP_THROW( Exception("Can't copy " + tmp_file.asString() + " to " + dest_dir.asString()));
        }

        media.releaseFile(resource); //not needed anymore, only eat space
      }
      catch (Exception & excpt_r)
      {
        ZYPP_CAUGHT(excpt_r);
        excpt_r.remember("Can't provide " + resource.filename().asString() + " : " + excpt_r.msg());
        ZYPP_RETHROW(excpt_r);
      }
    }
    else
    {
      // We got the file from cache
      // continue with next file
        return;
    }
  }

  // helper class to consume a content file
  struct ContentReaderHelper : public parser::susetags::ContentFileReader
  {
    ContentReaderHelper()
    {
      setRepoIndexConsumer( bind( &ContentReaderHelper::consumeIndex, this, _1 ) );
    }
        
    void consumeIndex( const parser::susetags::RepoIndex_Ptr & data_r )
    { _repoindex = data_r; }

    parser::susetags::RepoIndex_Ptr _repoindex;
  };

  // generic function for reading indexes
  void Fetcher::Impl::readIndex( const Pathname &index, const Pathname &basedir )
  {
    if ( index.basename() == "SHA1SUMS" )
      readSha1sumsIndex(index, basedir);
    else if ( index.basename() == "content" )
      readContentFileIndex(index, basedir);
    else
      WAR << index << ": index file format not known" << endl;
  }

  // reads a content file index
  void Fetcher::Impl::readContentFileIndex( const Pathname &index, const Pathname &basedir )
  {
      ContentReaderHelper reader;
      reader.parse(index);
      MIL << index << " contains " << reader._repoindex->mediaFileChecksums.size() << " checksums." << endl;
      for_( it, reader._repoindex->mediaFileChecksums.begin(), reader._repoindex->mediaFileChecksums.end() )
      {
          // content file entries don't start with /
          _checksums[(basedir + it->first).asString()] = it->second;
      }
  }

  // reads a SHA1SUMS file index
  void Fetcher::Impl::readSha1sumsIndex( const Pathname &index, const Pathname &basedir )
  {
      std::ifstream in( index.c_str() );
      string buffer;
      if ( ! in.fail() )
      {
          while ( getline(in, buffer) )
          {
              vector<string> words;
              str::split( buffer, back_inserter(words) );
              if ( words.size() != 2 )
                  ZYPP_THROW(Exception("Wrong format for SHA1SUMS file"));
              //MIL << "check: '" << words[0] << "' | '" << words[1] << "'" << endl;
              if ( ! words[1].empty() )
                  _checksums[(basedir + words[1]).asString()] = CheckSum::sha1(words[0]);
          }
      }
      else
          ZYPP_THROW(Exception("Can't open SHA1SUMS file: " + index.asString()));
  }
    
  void Fetcher::Impl::readIndexes( MediaSetAccess &media, const Pathname &dest_dir)
  {
      // if there is no indexes, then just return to avoid
      // the directory listing
      if ( _indexes.empty() )
      {
          MIL << "No indexes to read." << endl;
          return;
      }

      // create a new fetcher with a different state to transfer the
      // file containing checksums and its signature
      Fetcher fetcher;
      // signature checker for index. We havent got the signature from
      // the nextwork yet.
      SignatureFileChecker sigchecker;
      
      for ( list<FetcherIndex_Ptr>::const_iterator it_idx = _indexes.begin();
            it_idx != _indexes.end(); ++it_idx )
      {
          MIL << "reading index " << (*it_idx)->location << endl;
          // build the name of the index and the signature
          OnMediaLocation idxloc((*it_idx)->location);
          OnMediaLocation sigloc((*it_idx)->location.setOptional(true));
          OnMediaLocation keyloc((*it_idx)->location.setOptional(true));

          // calculate signature and key name
          sigloc.changeFilename( sigloc.filename().extend(".asc") );
          keyloc.changeFilename( keyloc.filename().extend(".key") );

          //assert_dir(dest_dir + idxloc.filename().dirname());

          // transfer the signature
          fetcher.enqueue(sigloc);
          fetcher.start( dest_dir, media );
          // if we get the signature, update the checker
          sigchecker = SignatureFileChecker(dest_dir + sigloc.filename());
          fetcher.reset();
          
          // now the key
          fetcher.enqueue(keyloc);
          fetcher.start( dest_dir, media );
          fetcher.reset();

          // now the index itself
          fetcher.enqueue( idxloc, FileChecker(sigchecker) );
          fetcher.start( dest_dir, media );
          fetcher.reset();

          // now we have the indexes in dest_dir
          readIndex( dest_dir + idxloc.filename(), idxloc.filename().dirname() );
      }
      MIL << "done reading indexes" << endl;
  }
    
  void Fetcher::Impl::start( const Pathname &dest_dir,
                             MediaSetAccess &media,
                             const ProgressData::ReceiverFnc & progress_receiver )
  {
    ProgressData progress(_resources.size());
    progress.sendTo(progress_receiver);

    readIndexes(media, dest_dir);

    for ( list<FetcherJob_Ptr>::const_iterator it_res = _resources.begin(); it_res != _resources.end(); ++it_res )
    {

      if ( (*it_res)->directory )
      {
          const OnMediaLocation location((*it_res)->location);
          addDirJobs(media, location, dest_dir, true);
          continue;
      }

      provideToDest(media, (*it_res)->location, dest_dir);

      // if the checksum is empty, but the checksum is in one of the
      // indexes checksum, then add a checker
      if ( (*it_res)->location.checksum().empty() )
      {
          if ( _checksums.find((*it_res)->location.filename().asString()) 
               != _checksums.end() )
          {
              // the checksum can be replaced with the one in the index.
              CheckSum chksm = _checksums[(*it_res)->location.filename().asString()];
              ChecksumFileChecker digest_check(chksm);    
              (*it_res)->checkers.push_back(digest_check);
          }
      }
      
      // validate job, this throws if not valid
      validate((*it_res)->location, dest_dir, (*it_res)->checkers);

      if ( ! progress.incr() )
        ZYPP_THROW(AbortRequestException());
    } // for each job
  }

  /** \relates Fetcher::Impl Stream output */
  inline std::ostream & operator<<( std::ostream & str, const Fetcher::Impl & obj )
  {
      for ( list<FetcherJob_Ptr>::const_iterator it_res = obj._resources.begin(); it_res != obj._resources.end(); ++it_res )
      {
          str << *it_res;
      }
      return str;
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
  : _pimpl( new Impl() )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Fetcher::~Fetcher
  //	METHOD TYPE : Dtor
  //
  Fetcher::~Fetcher()
  {}

  void Fetcher::setOptions( Fetcher::Options options )
  {
    _pimpl->setOptions(options);
  }
    
  Fetcher::Options Fetcher::options() const
  {
    return _pimpl->options();
  }

  void Fetcher::enqueueDigested( const OnMediaLocation &resource, const FileChecker &checker )
  {
    _pimpl->enqueueDigested(resource, checker);
  }

  void Fetcher::enqueueDir( const OnMediaLocation &resource,
                            bool recursive,
                            const FileChecker &checker )
  {
      _pimpl->enqueueDir(resource, recursive, checker);
  }

  void Fetcher::addIndex( const OnMediaLocation &resource )
  {
    _pimpl->addIndex(resource);
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

