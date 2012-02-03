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
#include "zypp/base/LogControl.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/base/DefaultIntegral.h"
#include "zypp/base/String.h"
#include "zypp/Fetcher.h"
#include "zypp/ZYppFactory.h"
#include "zypp/CheckSum.h"
#include "zypp/base/UserRequestException.h"
#include "zypp/parser/susetags/ContentFileReader.h"
#include "zypp/parser/susetags/RepoIndex.h"

using namespace std;

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp:fetcher"

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
    {}
    /** Index localtion. */
    OnMediaLocation             location;
    /** Whether we read this index. */
    DefaultIntegral<bool,false> read;
  };

  typedef shared_ptr<FetcherIndex> FetcherIndex_Ptr;

  /** std::set ordering (less semantic) */
  struct SameFetcherIndex
  {
    bool operator()( const FetcherIndex_Ptr & lhs, const FetcherIndex_Ptr & rhs )
    {
      if ( lhs == rhs )
        return false; // incl. NULL == NULL
      if ( ! lhs )
        return true;  // NULL < nonNULL
      if ( ! rhs )
        return false; // nonNULL > NULL
      // both nonNULL ==> compare medianr and path
      if ( lhs->location.medianr() == rhs->location.medianr() )
        return lhs->location.filename() < rhs->location.filename();
      //else
        return lhs->location.medianr() < rhs->location.medianr();
    }
  };

  /**
   * Class to encapsulate the \ref OnMediaLocation object
   * and the \ref FileChecker together
   */
  struct FetcherJob
  {
    enum Flag
    {
        None      = 0x0000,
        Directory = 0x0001,
        Recursive = 0x0002,
        RecursiveDirectory = Directory | Recursive,
        // check checksums even if there is no such
        // checksum (warns of no checksum)
        AlwaysVerifyChecksum = 0x0004,
    };
    ZYPP_DECLARE_FLAGS(Flags, Flag);


    FetcherJob( const OnMediaLocation &loc, const Pathname dfile = Pathname())
      : location(loc)
      , deltafile(dfile)
      , flags(None)
    {
      //MIL << location << endl;
    }

    ~FetcherJob()
    {
      //MIL << location << " | * " << checkers.size() << endl;
    }

    OnMediaLocation location;
    Pathname deltafile;
    //CompositeFileChecker checkers;
    list<FileChecker> checkers;
    Flags flags;
  };

  ZYPP_DECLARE_OPERATORS_FOR_FLAGS(FetcherJob::Flags);
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

    ~Impl() {}

    void setOptions( Fetcher::Options options );
    Fetcher::Options options() const;

    void addIndex( const OnMediaLocation &resource );

    void enqueueDir( const OnMediaLocation &resource, bool recursive, const FileChecker &checker = FileChecker() );
    void enqueueDigestedDir( const OnMediaLocation &resource, bool recursive, const FileChecker &checker = FileChecker() );

    void enqueue( const OnMediaLocation &resource, const FileChecker &checker = FileChecker()  );
    void enqueueDigested( const OnMediaLocation &resource, const FileChecker &checker = FileChecker(), const Pathname &deltafile = Pathname() );
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
      void downloadAndReadIndexList( MediaSetAccess &media, const Pathname &dest_dir);

      /**
       * download the indexes and reads them
       */
      void downloadIndex( MediaSetAccess &media, const OnMediaLocation &resource, const Pathname &dest_dir);

      /**
       * reads a downloaded index file and updates internal
       * attributes table
       *
       * The index lists files relative to a directory, which is
       * normally the same as the index file is located.
       */
      void readIndex( const Pathname &index, const Pathname &basedir );

      /** specific version of \ref readIndex for CHECKSUMS file */
      void readChecksumsIndex( const Pathname &index, const Pathname &basedir );

      /** specific version of \ref readIndex for content file */
      void readContentFileIndex( const Pathname &index, const Pathname &basedir );

      /** reads the content of a directory but keeps a cache **/
      void getDirectoryContent( MediaSetAccess &media, const OnMediaLocation &resource, filesystem::DirContent &content );

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
                        const Pathname &dest_dir, FetcherJob::Flags flags );

      /**
       * auto discovery and reading of indexes
       */
      void autoaddIndexes( const filesystem::DirContent &content,
                           MediaSetAccess &media,
                           const OnMediaLocation &resource,
                           const Pathname &dest_dir );
      /**
       * Provide the resource to \ref dest_dir
       */
      void provideToDest( MediaSetAccess &media, const OnMediaLocation &resource, const Pathname &dest_dir , const Pathname &deltafile);

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }

    list<FetcherJob_Ptr>   _resources;
    std::set<FetcherIndex_Ptr,SameFetcherIndex> _indexes;
    std::set<Pathname> _caches;
    // checksums read from the indexes
    map<string, CheckSum> _checksums;
    // cache of dir contents
    map<string, filesystem::DirContent> _dircontent;

    Fetcher::Options _options;
  };
  ///////////////////////////////////////////////////////////////////

  void Fetcher::Impl::enqueueDigested( const OnMediaLocation &resource, const FileChecker &checker, const Pathname &deltafile )
  {
    FetcherJob_Ptr job;
    job.reset(new FetcherJob(resource, deltafile));
    job->flags |= FetcherJob:: AlwaysVerifyChecksum;
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
    if ( recursive )
        job->flags |= FetcherJob::Recursive;
    job->flags |= FetcherJob::Directory;

    _resources.push_back(job);
  }

  void Fetcher::Impl::enqueueDigestedDir( const OnMediaLocation &resource,
                                          bool recursive,
                                          const FileChecker &checker )
  {
    FetcherJob_Ptr job;
    job.reset(new FetcherJob(resource));
    if ( checker )
        job->checkers.push_back(checker);
    if ( recursive )
        job->flags |= FetcherJob::Recursive;
    job->flags |= FetcherJob::Directory;
    job->flags |= FetcherJob::AlwaysVerifyChecksum;

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
    _indexes.insert(FetcherIndex_Ptr(new FetcherIndex(resource)));
  }


  void Fetcher::Impl::reset()
  {
    _resources.clear();
    _indexes.clear();
    _checksums.clear();
    _dircontent.clear();
  }

  void Fetcher::Impl::addCachePath( const Pathname &cache_dir )
  {
    PathInfo info(cache_dir);
    if ( info.isExist() )
    {
      if ( info.isDir() )
      {
        DBG << "Adding fetcher cache: '" << cache_dir << "'." << endl;
        _caches.insert(cache_dir);
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

  // tries to provide resource to dest_dir from any of the configured additional
  // cache paths where the file may already be present. returns true if the
  // file was provided from the cache.
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

            if ( filesystem::hardlinkCopy(cached_file, dest_full_path ) != 0 )
            {
              ERR << "Can't hardlink/copy " << cached_file + " to " + dest_dir << endl;
              continue;
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

  void Fetcher::Impl::autoaddIndexes( const filesystem::DirContent &content,
                                      MediaSetAccess &media,
                                      const OnMediaLocation &resource,
                                      const Pathname &dest_dir )
  {
      auto fnc_addIfInContent( [&]( const std::string & index_r ) -> bool
      {
	if ( find( content.begin(), content.end(), filesystem::DirEntry(index_r,filesystem::FT_FILE) ) == content.end() )
	  return false;
	// add the index of this directory
	OnMediaLocation indexloc( resource );
	indexloc.changeFilename( resource.filename() + index_r );
	addIndex( indexloc );
	// we need to read it now
	downloadAndReadIndexList( media, dest_dir );
	return true;
      } );

      if ( _options & AutoAddChecksumsIndexes )
      {
	fnc_addIfInContent( "CHECKSUMS" ) || fnc_addIfInContent( "SHA1SUMS" );
      }
      if ( _options & AutoAddContentFileIndexes )
      {
	fnc_addIfInContent( "content" );
      }
  }

  void Fetcher::Impl::getDirectoryContent( MediaSetAccess &media,
                                           const OnMediaLocation &resource,
                                           filesystem::DirContent &content )
  {
      if ( _dircontent.find(resource.filename().asString())
           != _dircontent.end() )
      {
          filesystem::DirContent filled(_dircontent[resource.filename().asString()]);

          std::copy(filled.begin(), filled.end(), std::back_inserter(content));
      }
      else
      {
          filesystem::DirContent tofill;
          media.dirInfo( tofill,
                         resource.filename(),
                         false /* dots */,
                         resource.medianr());
          std::copy(tofill.begin(), tofill.end(), std::back_inserter(content));
          _dircontent[resource.filename().asString()] = tofill;
      }
  }

  void Fetcher::Impl::addDirJobs( MediaSetAccess &media,
                                  const OnMediaLocation &resource,
                                  const Pathname &dest_dir, FetcherJob::Flags flags  )
  {
      // first get the content of the directory so we can add
      // individual transfer jobs
      MIL << "Adding directory " << resource.filename() << endl;
      filesystem::DirContent content;
      try {
	getDirectoryContent(media, resource, content);
      }
      catch ( media::MediaFileNotFoundException & exception )
      {
	ZYPP_CAUGHT( exception );
	WAR << "Skiping subtree hidden at " << resource.filename() << endl;
	return;
      }

      // this method test for the option flags so indexes are added
      // only if the options are enabled
      autoaddIndexes(content, media, resource, dest_dir);

      for ( filesystem::DirContent::const_iterator it = content.begin();
            it != content.end();
            ++it )
      {
          // skip CHECKSUMS* as they were already retrieved
          if ( str::hasPrefix(it->name, "CHECKSUMS") || str::hasPrefix(it->name, "SHA1SUMS") )
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

              if ( flags & FetcherJob::AlwaysVerifyChecksum )
                  enqueueDigested(OnMediaLocation(filename, resource.medianr()).setChecksum(chksm));
              else
                  enqueue(OnMediaLocation(filename, resource.medianr()).setChecksum(chksm));
              break;
          }
          case filesystem::FT_DIR: // newer directory.yast contain at least directory info
              if ( flags & FetcherJob::Recursive )
                  addDirJobs(media, filename, dest_dir, flags);
              break;
          default:
              // don't provide devices, sockets, etc.
              break;
          }
      }
  }

  void Fetcher::Impl::provideToDest( MediaSetAccess &media, const OnMediaLocation &resource, const Pathname &dest_dir, const Pathname &deltafile )
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
        Pathname tmp_file = media.provideFile(resource, resource.optional() ? MediaSetAccess::PROVIDE_NON_INTERACTIVE : MediaSetAccess::PROVIDE_DEFAULT, deltafile );

        Pathname dest_full_path = dest_dir + resource.filename();

        if ( assert_dir( dest_full_path.dirname() ) != 0 )
              ZYPP_THROW( Exception("Can't create " + dest_full_path.dirname().asString()));
        if ( filesystem::hardlinkCopy( tmp_file, dest_full_path ) != 0 )
        {
          if ( ! PathInfo(tmp_file).isExist() )
              ERR << tmp_file << " does not exist" << endl;
          if ( ! PathInfo(dest_full_path.dirname()).isExist() )
              ERR << dest_full_path.dirname() << " does not exist" << endl;

          media.releaseFile(resource); //not needed anymore, only eat space
          ZYPP_THROW( Exception("Can't hardlink/copy " + tmp_file.asString() + " to " + dest_dir.asString()));
        }

        media.releaseFile(resource); //not needed anymore, only eat space
      }
      catch (Exception & excpt_r)
      {
        if ( resource.optional() )
        {
	    ZYPP_CAUGHT(excpt_r);
            WAR << "optional resource " << resource << " could not be transfered" << endl;
            return;
        }
        else
        {
	    excpt_r.remember("Can't provide " + resource.filename().asString() );
            ZYPP_RETHROW(excpt_r);
        }
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
    if ( index.basename() == "CHECKSUMS" || index.basename() == "SHA1SUMS" )
      readChecksumsIndex(index, basedir);
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

  // reads a CHECKSUMS (old SHA1SUMS) file index
  void Fetcher::Impl::readChecksumsIndex( const Pathname &index, const Pathname &basedir )
  {
      std::ifstream in( index.c_str() );
      if ( ! in.fail() )
      {
	  std::string buffer;
          while ( getline( in, buffer ) )
          {

	      if ( buffer[0] == '#' )
		continue;	// simple comment

	      CheckSum checksum( str::stripFirstWord( buffer, /*ltrim before strip*/true ) );
	      if ( checksum.empty() )
		continue;	// empty line | unknown cheksum format

	      if ( buffer.empty() )
	      {
		WAR << "Missing filename in CHECKSUMS file: " << index.asString() << " (" << checksum << ")" << endl;
		continue;
	      }

	      _checksums[(basedir/buffer).asString()] = checksum;
          }
      }
      else
          ZYPP_THROW(Exception("Can't open CHECKSUMS file: " + index.asString()));
  }

  void Fetcher::Impl::downloadIndex( MediaSetAccess &media, const OnMediaLocation &resource, const Pathname &dest_dir)
  {
    MIL << "downloading index " << resource << endl;
    // create a new fetcher with a different state to transfer the
    // file containing checksums and its signature
    Fetcher fetcher;
    // signature checker for index. We havent got the signature from
    // the nextwork yet.
    SignatureFileChecker sigchecker;

    // build the name of the index and the signature
    OnMediaLocation idxloc(resource);
    OnMediaLocation sigloc(resource);
    OnMediaLocation keyloc(resource);

    // we should not fail the download if those don't exists
    // the checking will warn later
    sigloc.setOptional(true);
    keyloc.setOptional(true);

    // calculate signature and key name
    sigloc.changeFilename( sigloc.filename().extend(".asc") );
    keyloc.changeFilename( keyloc.filename().extend(".key") );

    //assert_dir(dest_dir + idxloc.filename().dirname());

    // transfer the signature
    fetcher.enqueue(sigloc);
    fetcher.start( dest_dir, media );
    // if we get the signature, update the checker
    if ( PathInfo(dest_dir + sigloc.filename()).isExist() )
        sigchecker = SignatureFileChecker(dest_dir + sigloc.filename());

    fetcher.reset();

    // now the key
    fetcher.enqueue(keyloc);
    fetcher.start( dest_dir, media );
    fetcher.reset();

    // try to import the key
    if ( PathInfo(dest_dir + keyloc.filename()).isExist() )
        getZYpp()->keyRing()->importKey(PublicKey(dest_dir + keyloc.filename()), false);
    else
        WAR << "No public key specified by user for index '" << keyloc.filename() << "'"<< endl;

    // now the index itself
    fetcher.enqueue( idxloc, FileChecker(sigchecker) );
    fetcher.start( dest_dir, media );
    fetcher.reset();
 }

  // this method takes all the user pointed indexes, gets them and also tries to
  // download their signature, and verify them. After that, its parses each one
  // to fill the checksum cache.
  void Fetcher::Impl::downloadAndReadIndexList( MediaSetAccess &media, const Pathname &dest_dir)
  {
      // if there is no indexes, then just return to avoid
      // the directory listing
      if ( _indexes.empty() )
      {
          MIL << "No indexes to read." << endl;
          return;
      }

      for_( it_idx, _indexes.begin(), _indexes.end() )
      {
        if ( (*it_idx)->read )
        {
          DBG << "Already read index " << PathInfo(dest_dir + (*it_idx)->location.filename()) << endl;
        }
        else
        {
          // base::LogControl::TmpLineWriter shutUp;
          downloadIndex( media, (*it_idx)->location, dest_dir );
          // now we have the indexes in dest_dir
          readIndex( dest_dir + (*it_idx)->location.filename(), (*it_idx)->location.filename().dirname() );
          // Take care we don't process it again
          MIL << "Remember read index " << PathInfo(dest_dir + (*it_idx)->location.filename()) << endl;
          (*it_idx)->read = true;
        }
      }
      MIL << "done reading indexes" << endl;
  }

  // start processing all fetcher jobs.
  // it processes any user pointed index first
  void Fetcher::Impl::start( const Pathname &dest_dir,
                             MediaSetAccess &media,
                             const ProgressData::ReceiverFnc & progress_receiver )
  {
    ProgressData progress(_resources.size());
    progress.sendTo(progress_receiver);

    downloadAndReadIndexList(media, dest_dir);

    for ( list<FetcherJob_Ptr>::const_iterator it_res = _resources.begin(); it_res != _resources.end(); ++it_res )
    {

      if ( (*it_res)->flags & FetcherJob::Directory )
      {
          const OnMediaLocation location((*it_res)->location);
          addDirJobs(media, location, dest_dir, (*it_res)->flags);
          continue;
      }

      // may be this code can be factored out
      // together with the autodiscovery of indexes
      // of addDirJobs
      if ( ( _options & AutoAddChecksumsIndexes ) ||
           ( _options & AutoAddContentFileIndexes ) )
      {
          // if auto indexing is enabled, then we need to read the
          // index for each file. We look only in the directory
          // where the file is. this is expensive of course.
          filesystem::DirContent content;
          getDirectoryContent(media, (*it_res)->location.filename().dirname(), content);
          // this method test for the option flags so indexes are added
          // only if the options are enabled
          MIL << "Autodiscovering signed indexes on '"
              << (*it_res)->location.filename().dirname() << "' for '"
              << (*it_res)->location.filename() << "'" << endl;

          autoaddIndexes(content, media, (*it_res)->location.filename().dirname(), dest_dir);

          // also look in the root of the media
          content.clear();
          getDirectoryContent(media, Pathname("/"), content);
          // this method test for the option flags so indexes are added
          // only if the options are enabled
          MIL << "Autodiscovering signed indexes on '"
              << "/" << "' for '"
              << (*it_res)->location.filename() << "'" << endl;

          autoaddIndexes(content, media, Pathname("/"), dest_dir);
      }

      provideToDest(media, (*it_res)->location, dest_dir, (*it_res)->deltafile);

      // if the file was not transfered, and no exception, just
      // return, as it was an optional file
      if ( ! PathInfo(dest_dir + (*it_res)->location.filename()).isExist() )
          return;

      // if the checksum is empty, but the checksum is in one of the
      // indexes checksum, then add a checker
      if ( (*it_res)->location.checksum().empty() )
      {
          if ( _checksums.find((*it_res)->location.filename().asString())
               != _checksums.end() )
          {
              CheckSum chksm = _checksums[(*it_res)->location.filename().asString()];
              ChecksumFileChecker digest_check(chksm);
              (*it_res)->checkers.push_back(digest_check);
          }
          else
          {
              // if the index checksum is empty too, we only add the checker
              // if the  AlwaysVerifyChecksum option is set on
              if ( (*it_res)->flags & FetcherJob::AlwaysVerifyChecksum )
              {
                  // add the checker with the empty checksum
                  ChecksumFileChecker digest_check((*it_res)->location.checksum());
                  (*it_res)->checkers.push_back(digest_check);
              }
          }
      }
      else
      {
          // checksum is not empty, so add a checksum checker
          ChecksumFileChecker digest_check((*it_res)->location.checksum());
          (*it_res)->checkers.push_back(digest_check);
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

  Fetcher::Fetcher()
  : _pimpl( new Impl() )
  {}

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

  void Fetcher::enqueueDigested( const OnMediaLocation &resource, const FileChecker &checker, const Pathname &deltafile )
  {
    _pimpl->enqueueDigested(resource, checker, deltafile);
  }

  void Fetcher::enqueueDir( const OnMediaLocation &resource,
                            bool recursive,
                            const FileChecker &checker )
  {
      _pimpl->enqueueDir(resource, recursive, checker);
  }

  void Fetcher::enqueueDigestedDir( const OnMediaLocation &resource,
                                    bool recursive,
                                    const FileChecker &checker )
  {
      _pimpl->enqueueDigestedDir(resource, recursive, checker);
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

  std::ostream & operator<<( std::ostream & str, const Fetcher & obj )
  {
    return str << *obj._pimpl;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

