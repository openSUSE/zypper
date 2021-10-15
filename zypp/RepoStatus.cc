/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/RepoStatus.cc
 *
*/
#include <iostream>
#include <sstream>
#include <fstream>
#include <optional>
#include <set>
#include <zypp/base/Logger.h>
#include <zypp/base/String.h>
#include <zypp/RepoStatus.h>
#include <zypp/RepoInfo.h>
#include <zypp/PathInfo.h>

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace
  {
    /** Recursive computation of max dir timestamp. */
    void recursiveTimestamp( const Pathname & dir_r, time_t & max_r )
    {
      std::list<std::string> dircontent;
      if ( filesystem::readdir( dircontent, dir_r, false/*no dots*/ ) != 0 )
        return; // readdir logged the error

      for_( it, dircontent.begin(), dircontent.end() )
      {
        PathInfo pi( dir_r + *it, PathInfo::LSTAT );
        if ( pi.isDir() )
        {
          if ( pi.mtime() > max_r )
            max_r = pi.mtime();
          recursiveTimestamp( pi.path(), max_r );
        }
      }
    }
  } // namespace
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : RepoStatus::Impl
  //
  /** RepoStatus implementation. */
  struct RepoStatus::Impl
  {
    using Checksums = std::set<std::string>;

  public:
    /** Assign data called from RepoStatus ctor (adds magic).
     *
     * \Note Changing magic will at once invalidate all solv file caches.
     * Helpfull if solv file content must be refreshed (e.g. due to different
     * repo2solv arguments) even if raw metadata are unchanged. Only values
     * set from a RepoStatus ctor need magic to be added!
     */
    void assignFromCtor( std::string && checksum_r, Date && timestamp_r )
    {
      if ( !checksum_r.empty() ) {
        static const std::string magic( "43" );
        checksum_r += magic;
        _checksums.insert( std::move(checksum_r) );
      }
      _timestamp = std::move(timestamp_r);
    }

    /** Inject raw data (no magic added). */
    void inject( std::string && checksum_r, Date && timestamp_r )
    {
      if ( !checksum_r.empty() ) {
        _checksums.insert( std::move(checksum_r) );
        _cachedchecksum.reset();
      }

      if ( timestamp_r > _timestamp )
        _timestamp = timestamp_r;
    }

    /** Inject the raw data from rhs */
    void injectFrom( const Impl & rhs )
    {
      if ( &rhs == this )	// no self insert
        return;

      if ( !rhs._checksums.empty() ) {
        _checksums.insert( rhs._checksums.begin(), rhs._checksums.end() );
        _cachedchecksum.reset();
      }

      if ( rhs._timestamp > _timestamp )
        _timestamp = rhs._timestamp;
    }

    bool empty() const
    { return _checksums.empty(); }

    std::string checksum() const
    {
      std::string ret;
      if ( _checksums.empty() )
        return ret;

      if ( _checksums.size() == 1 )
        ret = *_checksums.begin();
      else {
        if ( !_cachedchecksum ) {
          std::stringstream ss;
          for ( std::string_view c : _checksums )
            ss << c;
          _cachedchecksum = CheckSum::sha1(ss).checksum();
        }
        ret = *_cachedchecksum;
      }
      return ret;
    }

    Date timestamp() const
    { return _timestamp; }

    /** Dump to log file (not to/from CookieFile). */
    std::ostream & dumpOn( std::ostream & str ) const
    { return str << ( empty() ? "NO_REPOSTATUS" : checksum() ) << " " << time_t(_timestamp); }

  private:
    Checksums _checksums;
    Date _timestamp;

    mutable std::optional<std::string> _cachedchecksum;

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : RepoStatus
  //
  ///////////////////////////////////////////////////////////////////

  RepoStatus::RepoStatus()
    : _pimpl( new Impl() )
  {}

  RepoStatus::RepoStatus( const Pathname & path_r )
    : _pimpl( new Impl() )
  {
    PathInfo info( path_r );
    if ( info.isExist() )
    {
      if ( info.isFile() )
      {
        _pimpl->assignFromCtor( filesystem::sha1sum( path_r ), Date( info.mtime() ) );
      }
      else if ( info.isDir() )
      {
        time_t t = info.mtime();
        recursiveTimestamp( path_r, t );
        _pimpl->assignFromCtor( CheckSum::sha1FromString( str::numstring( t ) ).checksum(), Date( t ) );
      }
    }
  }

  RepoStatus::RepoStatus( const RepoInfo & info_r )
  : _pimpl( new Impl() )
  {
    _pimpl->assignFromCtor( CheckSum::sha1FromString( info_r.url().asString() ).checksum(), Date() );
  }

  RepoStatus::RepoStatus( std::string checksum_r, Date timestamp_r )
  : _pimpl( new Impl() )
  {
    _pimpl->assignFromCtor( std::move(checksum_r), std::move(timestamp_r) );
  }

  RepoStatus::~RepoStatus()
  {}

  RepoStatus RepoStatus::fromCookieFile( const Pathname & path_r )
  {
    RepoStatus ret;
    std::ifstream file( path_r.c_str() );
    if ( !file )
    {
      WAR << "No cookie file " << path_r << endl;
    }
    else
    {
      // line := "[checksum] time_t"  !!! strip time from line
      std::string line { str::getline( file ) };
      Date        stmp { str::strtonum<time_t>( str::stripLastWord( line ) ) };
      ret._pimpl->inject( std::move(line), std::move(stmp) );	// raw inject to avoid magic being added
    }
    return ret;
  }

  void RepoStatus::saveToCookieFile( const Pathname & path_r ) const
  {
    std::ofstream file(path_r.c_str());
    if (!file) {
      ZYPP_THROW (Exception( "Can't open " + path_r.asString() ) );
    }
    file << _pimpl->checksum() << " " << time_t(_pimpl->timestamp()) << endl;
    file.close();
  }

  bool RepoStatus::empty() const
  { return _pimpl->empty(); }

  Date RepoStatus::timestamp() const
  { return _pimpl->timestamp(); }

  std::ostream & operator<<( std::ostream & str, const RepoStatus & obj )
  { return obj._pimpl->dumpOn( str ); }

  RepoStatus operator&&( const RepoStatus & lhs, const RepoStatus & rhs )
  {
    RepoStatus result { lhs };
    result._pimpl->injectFrom( *rhs._pimpl );
    return result;
  }

  bool operator==( const RepoStatus & lhs, const RepoStatus & rhs )
  { return lhs._pimpl->checksum() == rhs._pimpl->checksum(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
