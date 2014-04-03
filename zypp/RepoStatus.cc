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
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/RepoStatus.h"
#include "zypp/PathInfo.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : RepoStatus::Impl
  //
  /** RepoStatus implementation. */
  struct RepoStatus::Impl
  {
  public:
    string _checksum;
    Date _timestamp;

    /** Recursive computation of max dir timestamp. */
    static void recursive_timestamp( const Pathname & dir_r, time_t & max_r )
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
	  recursive_timestamp( pi.path(), max_r );
	}
      }
    }

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates RepoStatus::Impl Stream output */
  inline std::ostream & operator<<( std::ostream & str, const RepoStatus::Impl & obj )
  { return str << obj._checksum << " " << (time_t)obj._timestamp; }

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
	_pimpl->_timestamp = Date( info.mtime() );
	_pimpl->_checksum = filesystem::sha1sum( path_r );
      }
      else if ( info.isDir() )
      {
	time_t t = info.mtime();
	Impl::recursive_timestamp( path_r, t );
	_pimpl->_timestamp = Date(t);
	_pimpl->_checksum = CheckSum::sha1FromString( str::numstring( t ) ).checksum();
      }

      // NOTE: changing magic will once invalidate all solv file caches
      // Helpfull if solv file content must be refreshed (e.g. due to different
      // repo2* arguments) even if raw metadata are unchanged.
      static const std::string magic( "42" );
      _pimpl->_checksum += magic;
    }
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
      // line := "[checksum] time_t"
      std::string line( str::getline( file ) );
      ret._pimpl->_timestamp = Date( str::strtonum<time_t>( str::stripLastWord( line ) ) );
      ret._pimpl->_checksum = line;
    }
    return ret;
  }

  void RepoStatus::saveToCookieFile( const Pathname & path_r ) const
  {
    std::ofstream file(path_r.c_str());
    if (!file) {
      ZYPP_THROW (Exception( "Can't open " + path_r.asString() ) );
    }
    file << _pimpl->_checksum << " " << (time_t)_pimpl->_timestamp << endl;
    file.close();
  }

  bool RepoStatus::empty() const
  { return _pimpl->_checksum.empty(); }

  Date RepoStatus::timestamp() const
  { return _pimpl->_timestamp; }

  std::ostream & operator<<( std::ostream & str, const RepoStatus & obj )
  { return str << *obj._pimpl; }

  RepoStatus operator&&( const RepoStatus & lhs, const RepoStatus & rhs )
  {
    RepoStatus result;

    if ( lhs.empty() )
      result = rhs;
    else if ( rhs.empty() )
      result = lhs;
    else
    {
      // order strings to assert && is kommutativ
      std::string lchk( lhs._pimpl->_checksum );
      std::string rchk( rhs._pimpl->_checksum );
      stringstream ss( lchk < rchk ? lchk+rchk : rchk+lchk );

      result._pimpl->_checksum = CheckSum::sha1(ss).checksum();
      result._pimpl->_timestamp = std::max( lhs._pimpl->_timestamp, rhs._pimpl->_timestamp );
    }
    return result;
  }

  bool operator==( const RepoStatus & lhs, const RepoStatus & rhs )
  { return lhs._pimpl->_checksum == rhs._pimpl->_checksum; }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
