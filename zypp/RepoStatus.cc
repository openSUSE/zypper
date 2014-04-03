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
    string checksum;
    Date timestamp;

    /** Recursive computation of max dir timestamp. */
    static void recursive_timestamp( const Pathname & dir, time_t & max )
    {
      std::list<std::string> dircontent;
      if ( filesystem::readdir( dircontent, dir, false/*no dots*/ ) != 0 )
	return; // readdir logged the error

      for_( it, dircontent.begin(), dircontent.end() )
      {
	PathInfo pi( dir + *it, PathInfo::LSTAT );
	if ( pi.isDir() )
	{
	  recursive_timestamp( pi.path(), max );
	  if ( pi.mtime() > max )
	    max = pi.mtime();
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
  {
    return str << obj.checksum << " " << (time_t)obj.timestamp;
  }

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
	_pimpl->timestamp = Date( info.mtime() );
	_pimpl->checksum = filesystem::sha1sum( path_r );
      }
      else if ( info.isDir() )
      {
	time_t t = info.mtime();
	Impl::recursive_timestamp( path_r, t );
	_pimpl->timestamp = Date(t);
	_pimpl->checksum = CheckSum::sha1FromString( str::numstring( t ) ).checksum();
      }
    }
  }

  RepoStatus::~RepoStatus()
  {}

  RepoStatus RepoStatus::fromCookieFile( const Pathname & cookiefile_r )
  {
    RepoStatus ret;
    std::ifstream file( cookiefile_r.c_str() );
    if ( !file )
    {
      WAR << "No cookie file " << cookiefile_r << endl;
    }
    else
    {
      // line := "[checksum] time_t"
      std::string line( str::getline( file ) );
      ret.setTimestamp( Date( str::strtonum<time_t>( str::stripLastWord( line ) ) ) );
      ret.setChecksum( line );
    }
    return ret;
  }

  void RepoStatus::saveToCookieFile( const Pathname &cookiefile ) const
  {
    std::ofstream file(cookiefile.c_str());
    if (!file) {
      ZYPP_THROW (Exception( "Can't open " + cookiefile.asString() ) );
    }
    file << checksum() << " " << (time_t)timestamp() << endl;
    file.close();
  }

  bool RepoStatus::empty() const
  {
    return _pimpl->checksum.empty();
  }

  RepoStatus & RepoStatus::setChecksum( const string &checksum )
  {
    _pimpl->checksum = checksum;
    return *this;
  }

  RepoStatus & RepoStatus::setTimestamp( const Date &timestamp )
  {
    _pimpl->timestamp = timestamp;
    return *this;
  }

  string RepoStatus::checksum() const
  { return _pimpl->checksum; }

  Date RepoStatus::timestamp() const
  { return _pimpl->timestamp; }

  RepoStatus operator&&( const RepoStatus & lhs, const RepoStatus & rhs )
  {
    if ( lhs.empty() )
      return rhs;
    if ( rhs.empty() )
      return lhs;

    // order strings to assert && is kommutativ
    std::string lchk( lhs.checksum() );
    std::string rchk( rhs.checksum() );
    stringstream ss( lchk < rchk ? lchk+rchk : rchk+lchk );

    RepoStatus result;
    result.setChecksum( CheckSum::sha1(ss).checksum() );
    result.setTimestamp( lhs.timestamp() < rhs.timestamp() ? rhs.timestamp() : lhs.timestamp() );
    return result;
  }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const RepoStatus & obj )
  {
    return str << *obj._pimpl;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
