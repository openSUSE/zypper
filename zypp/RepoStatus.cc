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
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates RepoStatus::Impl Stream output */
  inline std::ostream & operator<<( std::ostream & str, const RepoStatus::Impl & obj )
  {
    return str << obj.checksum << " " << (time_t) obj.timestamp;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : RepoStatus
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : RepoStatus::RepoStatus
  //	METHOD TYPE : Ctor
  //
  RepoStatus::RepoStatus()
    : _pimpl( new Impl() )
  {}

  RepoStatus::RepoStatus( const Pathname &path )
    : _pimpl( new Impl() )
  {
      PathInfo info(path);
      if ( info.isExist() )
      {
        _pimpl->timestamp = Date(info.mtime());
        if ( info.isFile() )
          _pimpl->checksum = filesystem::sha1sum(path);
        else // non files
          _pimpl->checksum = str::numstring(info.mtime());
      }
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : RepoStatus::~RepoStatus
  //	METHOD TYPE : Dtor
  //
  RepoStatus::~RepoStatus()
  {}

  RepoStatus RepoStatus::fromCookieFile( const Pathname &cookiefile )
  {
    std::ifstream file(cookiefile.c_str());
    if (!file) {
      WAR << "No cookie file " << cookiefile << endl;
      return RepoStatus();
    }

    RepoStatus status;
    std::string buffer;
    file >> buffer;
    status.setChecksum(buffer);
    file >> buffer;
    status.setTimestamp(Date(str::strtonum<time_t>(buffer)));
    return status;
  }

  void RepoStatus::saveToCookieFile( const Pathname &cookiefile ) const
  {
    std::ofstream file(cookiefile.c_str());
    if (!file) {
      ZYPP_THROW (Exception( "Can't open " + cookiefile.asString() ) );
    }
    file << this->checksum() << " " << (int) this->timestamp() << endl << endl;
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

    std::string lchk( lhs.checksum() );
    std::string rchk( rhs.checksum() );
    // order strings to assert && is kommutativ
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
